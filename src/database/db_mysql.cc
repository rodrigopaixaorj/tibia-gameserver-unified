#include "db_driver.hh"
#include "db_common.hh"

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#if defined(__has_include)
#  if __has_include(<mysql/mysql.h>)
#    include <mysql/mysql.h>
#  else
#    include <mysql.h>
#  endif
#else
#  include <mysql.h>
#endif

#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>

class MySQLDriver : public IDatabaseDriver {
private:
	MYSQL *m_MySQL = nullptr;
	std::mutex m_Mutex;
	std::string m_Host;
	int m_Port = 3306;
	std::string m_User;
	std::string m_Password;
	std::string m_Database;
	bool m_AutoReconnect = true;

	bool ExecSimpleSQL(const char *SQL){
		if(!m_MySQL) return false;
		if(mysql_query(m_MySQL, SQL) != 0){
			LOG_ERR("MySQL query error: %s", mysql_error(m_MySQL));
			return false;
		}
		// Consume any remaining result sets if any (multi-query support)
		do {
			MYSQL_RES *res = mysql_store_result(m_MySQL);
			if(res) mysql_free_result(res);
		} while(mysql_next_result(m_MySQL) == 0);
		return true;
	}

public:
	MySQLDriver(const char *host, int port, const char *user, const char *pass, const char *db, bool autoReconnect)
		: m_Host(host ? host : "127.0.0.1"),
		  m_Port(port > 0 ? port : 3306),
		  m_User(user ? user : "root"),
		  m_Password(pass ? pass : ""),
		  m_Database(db ? db : "tibia"),
		  m_AutoReconnect(autoReconnect) {}

	virtual ~MySQLDriver(){
		Exit();
	}

	const char *GetName() const override {
		return "MySQL/MariaDB";
	}

	bool Init() override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_MySQL) return true;

		m_MySQL = mysql_init(nullptr);
		if(!m_MySQL){
			LOG_ERR("Failed to allocate MySQL connection handle.");
			return false;
		}

		my_bool reconnect = m_AutoReconnect ? 1 : 0;
		mysql_options(m_MySQL, MYSQL_OPT_RECONNECT, &reconnect);

		unsigned int timeout = 10;
		mysql_options(m_MySQL, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

		if(!mysql_real_connect(m_MySQL, m_Host.c_str(), m_User.c_str(), m_Password.c_str(),
				m_Database.c_str(), m_Port, nullptr, CLIENT_MULTI_STATEMENTS)){
			LOG_ERR("Failed to connect to MySQL database at %s:%d: %s", m_Host.c_str(), m_Port, mysql_error(m_MySQL));
			mysql_close(m_MySQL);
			m_MySQL = nullptr;
			return false;
		}

		mysql_set_character_set(m_MySQL, "utf8mb4");

		// Check if schema exists, otherwise attempt auto-initialization from schema_mysql.sql
		if(mysql_query(m_MySQL, "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = DATABASE() AND table_name = 'Accounts'") == 0){
			MYSQL_RES *res = mysql_store_result(m_MySQL);
			if(res){
				MYSQL_ROW row = mysql_fetch_row(res);
				int tableCount = (row && row[0]) ? atoi(row[0]) : 0;
				mysql_free_result(res);

				if(tableCount == 0){
					ExecuteSchema();
				}
			}
		}

		// Clear residual online status
		ExecSimpleSQL("UPDATE Characters SET IsOnline = 0;");
		return true;
	}

	void Exit() override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_MySQL){
			mysql_close(m_MySQL);
			m_MySQL = nullptr;
		}
	}

	bool ExecuteSchema() override {
		const char *schemaCandidates[] = {
			"tibia_mysql_full.sql",
			"../tibia_mysql_full.sql",
			"../../tibia_mysql_full.sql",
			"schema_mysql.sql",
			"../schema_mysql.sql",
			"../../schema_mysql.sql"
		};
		const char *schemaPath = nullptr;
		for(const char *cand : schemaCandidates){
			if(FileExists(cand)){
				schemaPath = cand;
				break;
			}
		}

		if(!schemaPath){
			LOG_WARN("Schema file not found for automatic MySQL initialization.");
			return false;
		}

		FILE *f = fopen(schemaPath, "rb");
		if(!f) return false;

		fseek(f, 0, SEEK_END);
		long sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *sql = (char*)malloc(sz + 1);
		if(!sql){
			fclose(f);
			return false;
		}

		fread(sql, 1, sz, f);
		sql[sz] = 0;
		fclose(f);

		bool ok = ExecSimpleSQL(sql);
		free(sql);
		return ok;
	}

	int LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
			int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL || !NumCharacters || !Characters || !PremiumDays) return 1;

		*NumCharacters = 0;
		*PremiumDays = 0;

		// 1. Fetch Account
		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ? LIMIT 1";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			LOG_ERR("mysql_stmt_prepare failed: %s", mysql_stmt_error(stmt));
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND bindParam[1];
		memset(bindParam, 0, sizeof(bindParam));
		unsigned int accId = AccountID;
		bindParam[0].buffer_type = MYSQL_TYPE_LONG;
		bindParam[0].buffer = &accId;
		bindParam[0].is_unsigned = 1;

		if(mysql_stmt_bind_param(stmt, bindParam) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND bindResult[3];
		memset(bindResult, 0, sizeof(bindResult));
		uint8 authBuf[256];
		unsigned long authLen = 0;
		my_bool isAuthNull = 0;
		long long premiumEnd = 0;
		int deleted = 0;

		bindResult[0].buffer_type = MYSQL_TYPE_BLOB;
		bindResult[0].buffer = authBuf;
		bindResult[0].buffer_length = sizeof(authBuf);
		bindResult[0].length = &authLen;
		bindResult[0].is_null = &isAuthNull;

		bindResult[1].buffer_type = MYSQL_TYPE_LONGLONG;
		bindResult[1].buffer = &premiumEnd;

		bindResult[2].buffer_type = MYSQL_TYPE_LONG;
		bindResult[2].buffer = &deleted;

		if(mysql_stmt_bind_result(stmt, bindResult) != 0 || mysql_stmt_store_result(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		if(mysql_stmt_fetch(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1; // Account not found
		}

		mysql_stmt_close(stmt);

		if(deleted != 0){
			return 1;
		}

		// Verify Password
		if(!DB_VerifyPassword(authBuf, (int)authLen, Password)){
			return 2; // Invalid password
		}

		int64 now = (int64)time(nullptr);
		if(premiumEnd > now){
			*PremiumDays = (int)((premiumEnd - now) / 86400);
			if(*PremiumDays <= 0) *PremiumDays = 1;
		} else {
			*PremiumDays = 0;
		}

		// 2. Fetch Characters
		stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 0;

		const char *charSql = "SELECT C.Name, W.Name, W.Host, W.Port "
							  "FROM Characters AS C "
							  "JOIN Worlds AS W ON C.WorldID = W.WorldID "
							  "WHERE C.AccountID = ? AND C.Deleted = 0 "
							  "ORDER BY C.Level DESC, C.LastLoginTime DESC LIMIT ?";

		if(mysql_stmt_prepare(stmt, charSql, (unsigned long)strlen(charSql)) == 0){
			MYSQL_BIND charParams[2];
			memset(charParams, 0, sizeof(charParams));

			charParams[0].buffer_type = MYSQL_TYPE_LONG;
			charParams[0].buffer = &accId;
			charParams[0].is_unsigned = 1;

			int maxChars = MaxCharacters;
			charParams[1].buffer_type = MYSQL_TYPE_LONG;
			charParams[1].buffer = &maxChars;

			if(mysql_stmt_bind_param(stmt, charParams) == 0 && mysql_stmt_execute(stmt) == 0){
				MYSQL_BIND charResults[4];
				memset(charResults, 0, sizeof(charResults));

				char charName[64] = {0};
				char worldName[64] = {0};
				char hostStr[64] = {0};
				int portVal = 7172;
				unsigned long nameLen = 0, worldLen = 0, hostLen = 0;

				charResults[0].buffer_type = MYSQL_TYPE_STRING;
				charResults[0].buffer = charName;
				charResults[0].buffer_length = sizeof(charName) - 1;
				charResults[0].length = &nameLen;

				charResults[1].buffer_type = MYSQL_TYPE_STRING;
				charResults[1].buffer = worldName;
				charResults[1].buffer_length = sizeof(worldName) - 1;
				charResults[1].length = &worldLen;

				charResults[2].buffer_type = MYSQL_TYPE_STRING;
				charResults[2].buffer = hostStr;
				charResults[2].buffer_length = sizeof(hostStr) - 1;
				charResults[2].length = &hostLen;

				charResults[3].buffer_type = MYSQL_TYPE_LONG;
				charResults[3].buffer = &portVal;

				if(mysql_stmt_bind_result(stmt, charResults) == 0 && mysql_stmt_store_result(stmt) == 0){
					int count = 0;
					while(mysql_stmt_fetch(stmt) == 0 && count < MaxCharacters){
						charName[nameLen < sizeof(charName) ? nameLen : sizeof(charName) - 1] = 0;
						worldName[worldLen < sizeof(worldName) ? worldLen : sizeof(worldName) - 1] = 0;
						hostStr[hostLen < sizeof(hostStr) ? hostLen : sizeof(hostStr) - 1] = 0;

						StringCopy(Characters[count].Name, sizeof(Characters[count].Name), charName);
						StringCopy(Characters[count].WorldName, sizeof(Characters[count].WorldName), worldName[0] ? worldName : "Tibia");
						Characters[count].WorldAddress = DB_ParseIPv4(hostStr);
						Characters[count].WorldPort = portVal > 0 ? portVal : 7172;
						count++;
					}
					*NumCharacters = count;
				}
			}
			mysql_stmt_close(stmt);
		}

		return 0; // Success
	}

	int LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
			int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
			int *PremiumNewbieBuffer) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "SELECT Type, RebootTime, Host, Port, MaxPlayers, PremiumPlayerBuffer, MaxNewbies, PremiumNewbieBuffer FROM Worlds LIMIT 1";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		if(mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		int wType = 0, rTime = 0, port = 7172, maxP = 1000, pBuffer = 100, maxN = 300, pnBuffer = 100;
		char hostStr[64] = {0};
		unsigned long hostLen = 0;

		MYSQL_BIND bindRes[8];
		memset(bindRes, 0, sizeof(bindRes));

		bindRes[0].buffer_type = MYSQL_TYPE_LONG;
		bindRes[0].buffer = &wType;

		bindRes[1].buffer_type = MYSQL_TYPE_LONG;
		bindRes[1].buffer = &rTime;

		bindRes[2].buffer_type = MYSQL_TYPE_STRING;
		bindRes[2].buffer = hostStr;
		bindRes[2].buffer_length = sizeof(hostStr) - 1;
		bindRes[2].length = &hostLen;

		bindRes[3].buffer_type = MYSQL_TYPE_LONG;
		bindRes[3].buffer = &port;

		bindRes[4].buffer_type = MYSQL_TYPE_LONG;
		bindRes[4].buffer = &maxP;

		bindRes[5].buffer_type = MYSQL_TYPE_LONG;
		bindRes[5].buffer = &pBuffer;

		bindRes[6].buffer_type = MYSQL_TYPE_LONG;
		bindRes[6].buffer = &maxN;

		bindRes[7].buffer_type = MYSQL_TYPE_LONG;
		bindRes[7].buffer = &pnBuffer;

		if(mysql_stmt_bind_result(stmt, bindRes) == 0 && mysql_stmt_store_result(stmt) == 0){
			if(mysql_stmt_fetch(stmt) == 0){
				hostStr[hostLen < sizeof(hostStr) ? hostLen : sizeof(hostStr) - 1] = 0;
				if(WorldType) *WorldType = wType;
				if(RebootTime) *RebootTime = rTime;
				if(IPAddress) *IPAddress = (int)DB_ParseIPv4(hostStr);
				if(Port) *Port = port;
				if(MaxPlayers) *MaxPlayers = maxP;
				if(PremiumPlayerBuffer) *PremiumPlayerBuffer = pBuffer;
				if(MaxNewbies) *MaxNewbies = maxN;
				if(PremiumNewbieBuffer) *PremiumNewbieBuffer = pnBuffer;

				mysql_stmt_close(stmt);
				return 0;
			}
		}

		mysql_stmt_close(stmt);
		return 1;
	}

	int LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
			const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
			bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
			char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
			char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		// 1. Validate Account
		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ? LIMIT 1";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		unsigned int accId = AccountID;
		MYSQL_BIND bindParam[1];
		memset(bindParam, 0, sizeof(bindParam));
		bindParam[0].buffer_type = MYSQL_TYPE_LONG;
		bindParam[0].buffer = &accId;
		bindParam[0].is_unsigned = 1;

		if(mysql_stmt_bind_param(stmt, bindParam) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		uint8 authBuf[256];
		unsigned long authLen = 0;
		long long premiumEnd = 0;
		int deleted = 0;

		MYSQL_BIND bindResult[3];
		memset(bindResult, 0, sizeof(bindResult));
		bindResult[0].buffer_type = MYSQL_TYPE_BLOB;
		bindResult[0].buffer = authBuf;
		bindResult[0].buffer_length = sizeof(authBuf);
		bindResult[0].length = &authLen;

		bindResult[1].buffer_type = MYSQL_TYPE_LONGLONG;
		bindResult[1].buffer = &premiumEnd;

		bindResult[2].buffer_type = MYSQL_TYPE_LONG;
		bindResult[2].buffer = &deleted;

		if(mysql_stmt_bind_result(stmt, bindResult) != 0 || mysql_stmt_store_result(stmt) != 0 || mysql_stmt_fetch(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1; // Account not found
		}
		mysql_stmt_close(stmt);

		if(deleted != 0 || !DB_VerifyPassword(authBuf, (int)authLen, Password)){
			return 2; // Invalid password
		}

		bool isPremium = (premiumEnd > (int64)time(nullptr));
		if(PremiumAccountActivated){
			*PremiumAccountActivated = isPremium;
		}

		// 2. Fetch Character
		stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 3;

		const char *charSql = "SELECT CharacterID, Sex, Profession, Residence FROM Characters WHERE AccountID = ? AND Name = ? AND Deleted = 0 LIMIT 1";
		if(mysql_stmt_prepare(stmt, charSql, (unsigned long)strlen(charSql)) != 0){
			mysql_stmt_close(stmt);
			return 3;
		}

		MYSQL_BIND cParams[2];
		memset(cParams, 0, sizeof(cParams));
		cParams[0].buffer_type = MYSQL_TYPE_LONG;
		cParams[0].buffer = &accId;
		cParams[0].is_unsigned = 1;

		unsigned long pNameLen = (unsigned long)strlen(PlayerName);
		cParams[1].buffer_type = MYSQL_TYPE_STRING;
		cParams[1].buffer = PlayerName;
		cParams[1].buffer_length = pNameLen;
		cParams[1].length = &pNameLen;

		if(mysql_stmt_bind_param(stmt, cParams) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 3;
		}

		unsigned int cid = 0;
		int sexVal = 0;
		char profStr[64] = {0}, resStr[64] = {0};
		unsigned long profLen = 0, resLen = 0;

		MYSQL_BIND cResults[4];
		memset(cResults, 0, sizeof(cResults));
		cResults[0].buffer_type = MYSQL_TYPE_LONG;
		cResults[0].buffer = &cid;
		cResults[0].is_unsigned = 1;

		cResults[1].buffer_type = MYSQL_TYPE_LONG;
		cResults[1].buffer = &sexVal;

		cResults[2].buffer_type = MYSQL_TYPE_STRING;
		cResults[2].buffer = profStr;
		cResults[2].buffer_length = sizeof(profStr) - 1;
		cResults[2].length = &profLen;

		cResults[3].buffer_type = MYSQL_TYPE_STRING;
		cResults[3].buffer = resStr;
		cResults[3].buffer_length = sizeof(resStr) - 1;
		cResults[3].length = &resLen;

		if(mysql_stmt_bind_result(stmt, cResults) != 0 || mysql_stmt_store_result(stmt) != 0 || mysql_stmt_fetch(stmt) != 0){
			mysql_stmt_close(stmt);
			return 3; // Character not found
		}
		mysql_stmt_close(stmt);

		if(CharacterID) *CharacterID = (uint32)cid;
		if(Sex) *Sex = sexVal;

		// 3. Fetch Rights
		if(Rights){
			memset(Rights, 0, 12);
			if(isPremium){
				DB_SetRightBit(Rights, 0); // PREMIUM_ACCOUNT
			}

			stmt = mysql_stmt_init(m_MySQL);
			if(stmt){
				const char *rSql = "SELECT Name FROM CharacterRights WHERE CharacterID = ?";
				if(mysql_stmt_prepare(stmt, rSql, (unsigned long)strlen(rSql)) == 0){
					MYSQL_BIND rParams[1];
					memset(rParams, 0, sizeof(rParams));
					rParams[0].buffer_type = MYSQL_TYPE_LONG;
					rParams[0].buffer = &cid;
					rParams[0].is_unsigned = 1;

					if(mysql_stmt_bind_param(stmt, rParams) == 0 && mysql_stmt_execute(stmt) == 0){
						char rName[64] = {0};
						unsigned long rNameLen = 0;
						MYSQL_BIND rResults[1];
						memset(rResults, 0, sizeof(rResults));
						rResults[0].buffer_type = MYSQL_TYPE_STRING;
						rResults[0].buffer = rName;
						rResults[0].buffer_length = sizeof(rName) - 1;
						rResults[0].length = &rNameLen;

						if(mysql_stmt_bind_result(stmt, rResults) == 0 && mysql_stmt_store_result(stmt) == 0){
							while(mysql_stmt_fetch(stmt) == 0){
								rName[rNameLen < sizeof(rName) ? rNameLen : sizeof(rName) - 1] = 0;
								int r = DB_GetRightByName(rName);
								if(r >= 0){
									DB_SetRightBit(Rights, r);
								}
							}
						}
					}
				}
				mysql_stmt_close(stmt);
			}

			// Apply Gamemaster rights
			DB_ApplyGamemasterRights(AccountID, PlayerName, Rights);
		}

		// 4. Fetch Buddies
		if(NumberOfBuddies){
			*NumberOfBuddies = 0;
			if(BuddyIDs && BuddyNames){
				stmt = mysql_stmt_init(m_MySQL);
				if(stmt){
					const char *bSql = "SELECT B.BuddyID, C.Name FROM Buddies B "
									  "JOIN Characters C ON B.BuddyID = C.CharacterID "
									  "WHERE B.AccountID = ? LIMIT 100";
					if(mysql_stmt_prepare(stmt, bSql, (unsigned long)strlen(bSql)) == 0){
						MYSQL_BIND bParams[1];
						memset(bParams, 0, sizeof(bParams));
						bParams[0].buffer_type = MYSQL_TYPE_LONG;
						bParams[0].buffer = &accId;
						bParams[0].is_unsigned = 1;

						if(mysql_stmt_bind_param(stmt, bParams) == 0 && mysql_stmt_execute(stmt) == 0){
							unsigned int bId = 0;
							char bName[64] = {0};
							unsigned long bNameLen = 0;

							MYSQL_BIND bResults[2];
							memset(bResults, 0, sizeof(bResults));
							bResults[0].buffer_type = MYSQL_TYPE_LONG;
							bResults[0].buffer = &bId;
							bResults[0].is_unsigned = 1;

							bResults[1].buffer_type = MYSQL_TYPE_STRING;
							bResults[1].buffer = bName;
							bResults[1].buffer_length = sizeof(bName) - 1;
							bResults[1].length = &bNameLen;

							if(mysql_stmt_bind_result(stmt, bResults) == 0 && mysql_stmt_store_result(stmt) == 0){
								int bCount = 0;
								while(mysql_stmt_fetch(stmt) == 0 && bCount < 100){
									bName[bNameLen < sizeof(bName) ? bNameLen : sizeof(bName) - 1] = 0;
									BuddyIDs[bCount] = (uint32)bId;
									strncpy(BuddyNames[bCount], bName, 29);
									BuddyNames[bCount][29] = 0;
									bCount++;
								}
								*NumberOfBuddies = bCount;
							}
						}
					}
					mysql_stmt_close(stmt);
				}
			}
		}

		if(Guild) Guild[0] = 0;
		if(Rank) Rank[0] = 0;
		if(Title) Title[0] = 0;

		// Mark Online
		stmt = mysql_stmt_init(m_MySQL);
		if(stmt){
			const char *updSql = "UPDATE Characters SET IsOnline = 1 WHERE CharacterID = ?";
			if(mysql_stmt_prepare(stmt, updSql, (unsigned long)strlen(updSql)) == 0){
				MYSQL_BIND uParams[1];
				memset(uParams, 0, sizeof(uParams));
				uParams[0].buffer_type = MYSQL_TYPE_LONG;
				uParams[0].buffer = &cid;
				uParams[0].is_unsigned = 1;

				if(mysql_stmt_bind_param(stmt, uParams) == 0){
					mysql_stmt_execute(stmt);
				}
			}
			mysql_stmt_close(stmt);
		}

		return 0; // Success
	}

	int LogoutGame(uint32 CharacterID, int Level, const char *Profession,
			const char *Residence, time_t LastLoginTime, int TutorActivities) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "UPDATE Characters SET Level = ?, Profession = ?, Residence = ?, LastLoginTime = ?, TutorActivities = ?, IsOnline = 0 WHERE CharacterID = ?";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[6];
		memset(params, 0, sizeof(params));

		int lvl = Level;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &lvl;

		const char *prof = Profession ? Profession : "";
		unsigned long profLen = (unsigned long)strlen(prof);
		params[1].buffer_type = MYSQL_TYPE_STRING;
		params[1].buffer = (void*)prof;
		params[1].buffer_length = profLen;
		params[1].length = &profLen;

		const char *res = Residence ? Residence : "";
		unsigned long resLen = (unsigned long)strlen(res);
		params[2].buffer_type = MYSQL_TYPE_STRING;
		params[2].buffer = (void*)res;
		params[2].buffer_length = resLen;
		params[2].length = &resLen;

		long long loginTime = (long long)LastLoginTime;
		params[3].buffer_type = MYSQL_TYPE_LONGLONG;
		params[3].buffer = &loginTime;

		int tutor = TutorActivities;
		params[4].buffer_type = MYSQL_TYPE_LONG;
		params[4].buffer = &tutor;

		unsigned int cid = CharacterID;
		params[5].buffer_type = MYSQL_TYPE_LONG;
		params[5].buffer = &cid;
		params[5].is_unsigned = 1;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
			const char *Remark, bool Unjustified, time_t Time) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "INSERT INTO CharacterDeaths (CharacterID, Level, OffenderID, Remark, Unjustified, Timestamp) VALUES (?, ?, ?, ?, ?, ?)";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[6];
		memset(params, 0, sizeof(params));

		unsigned int cid = CharacterID;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &cid;
		params[0].is_unsigned = 1;

		int lvl = Level;
		params[1].buffer_type = MYSQL_TYPE_LONG;
		params[1].buffer = &lvl;

		unsigned int off = Offender;
		params[2].buffer_type = MYSQL_TYPE_LONG;
		params[2].buffer = &off;
		params[2].is_unsigned = 1;

		const char *rem = Remark ? Remark : "";
		unsigned long remLen = (unsigned long)strlen(rem);
		params[3].buffer_type = MYSQL_TYPE_STRING;
		params[3].buffer = (void*)rem;
		params[3].buffer_length = remLen;
		params[3].length = &remLen;

		int unj = Unjustified ? 1 : 0;
		params[4].buffer_type = MYSQL_TYPE_LONG;
		params[4].buffer = &unj;

		long long deathTime = (long long)Time;
		params[5].buffer_type = MYSQL_TYPE_LONGLONG;
		params[5].buffer = &deathTime;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "REPLACE INTO HouseOwners (WorldID, HouseID, OwnerID, PaidUntil) VALUES (1, ?, ?, ?)";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[3];
		memset(params, 0, sizeof(params));

		unsigned short hid = HouseID;
		params[0].buffer_type = MYSQL_TYPE_SHORT;
		params[0].buffer = &hid;
		params[0].is_unsigned = 1;

		unsigned int oid = OwnerID;
		params[1].buffer_type = MYSQL_TYPE_LONG;
		params[1].buffer = &oid;
		params[1].is_unsigned = 1;

		int paid = PaidUntil;
		params[2].buffer_type = MYSQL_TYPE_LONG;
		params[2].buffer = &paid;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) override {
		return InsertHouseOwner(HouseID, OwnerID, PaidUntil);
	}

	int DeleteHouseOwner(uint16 HouseID) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "DELETE FROM HouseOwners WHERE HouseID = ?";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		unsigned short hid = HouseID;
		params[0].buffer_type = MYSQL_TYPE_SHORT;
		params[0].buffer = &hid;
		params[0].is_unsigned = 1;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
			uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL || !NumberOfOwners) return 1;

		int maxOwners = *NumberOfOwners;
		*NumberOfOwners = 0;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "SELECT HO.HouseID, HO.OwnerID, C.Name, HO.PaidUntil "
						  "FROM HouseOwners HO "
						  "JOIN Characters C ON HO.OwnerID = C.CharacterID "
						  "ORDER BY HO.HouseID ASC LIMIT ?";

		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &maxOwners;

		if(mysql_stmt_bind_param(stmt, params) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		unsigned short hid = 0;
		unsigned int oid = 0;
		char name[64] = {0};
		unsigned long nameLen = 0;
		int paid = 0;

		MYSQL_BIND results[4];
		memset(results, 0, sizeof(results));

		results[0].buffer_type = MYSQL_TYPE_SHORT;
		results[0].buffer = &hid;
		results[0].is_unsigned = 1;

		results[1].buffer_type = MYSQL_TYPE_LONG;
		results[1].buffer = &oid;
		results[1].is_unsigned = 1;

		results[2].buffer_type = MYSQL_TYPE_STRING;
		results[2].buffer = name;
		results[2].buffer_length = sizeof(name) - 1;
		results[2].length = &nameLen;

		results[3].buffer_type = MYSQL_TYPE_LONG;
		results[3].buffer = &paid;

		if(mysql_stmt_bind_result(stmt, results) == 0 && mysql_stmt_store_result(stmt) == 0){
			int count = 0;
			while(mysql_stmt_fetch(stmt) == 0 && count < maxOwners){
				name[nameLen < sizeof(name) ? nameLen : sizeof(name) - 1] = 0;
				if(HouseIDs) HouseIDs[count] = hid;
				if(OwnerIDs) OwnerIDs[count] = oid;
				if(OwnerNames){
					strncpy(OwnerNames[count], name, 29);
					OwnerNames[count][29] = 0;
				}
				if(PaidUntils) PaidUntils[count] = paid;
				count++;
			}
			*NumberOfOwners = count;
			mysql_stmt_close(stmt);
			return 0;
		}

		mysql_stmt_close(stmt);
		return 1;
	}

	int ClearIsOnline(int *NumberOfAffectedPlayers) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;
		ExecSimpleSQL("UPDATE Characters SET IsOnline = 0;");
		if(NumberOfAffectedPlayers) *NumberOfAffectedPlayers = (int)mysql_affected_rows(m_MySQL);
		return 0;
	}

	int CreatePlayerlist(int NumberOfPlayers, const char **Names,
			int *Levels, const char (*Professions)[30], bool *NewRecord) override {
		if(NewRecord) *NewRecord = false;
		return 0;
	}

	int LogKilledCreatures(int NumberOfRaces, const char **Names,
			int *KilledPlayers, int *KilledCreatures) override {
		return 0;
	}

	int AddBuddy(uint32 AccountID, uint32 Buddy) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "INSERT IGNORE INTO Buddies (WorldID, AccountID, BuddyID) VALUES (1, ?, ?)";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[2];
		memset(params, 0, sizeof(params));

		unsigned int acc = AccountID;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &acc;
		params[0].is_unsigned = 1;

		unsigned int bud = Buddy;
		params[1].buffer_type = MYSQL_TYPE_LONG;
		params[1].buffer = &bud;
		params[1].is_unsigned = 1;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int RemoveBuddy(uint32 AccountID, uint32 Buddy) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "DELETE FROM Buddies WHERE AccountID = ? AND BuddyID = ?";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[2];
		memset(params, 0, sizeof(params));

		unsigned int acc = AccountID;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &acc;
		params[0].is_unsigned = 1;

		unsigned int bud = Buddy;
		params[1].buffer_type = MYSQL_TYPE_LONG;
		params[1].buffer = &bud;
		params[1].is_unsigned = 1;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int DecrementIsOnline(uint32 CharacterID) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return 1;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "UPDATE Characters SET IsOnline = 0 WHERE CharacterID = ?";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));

		unsigned int cid = CharacterID;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &cid;
		params[0].is_unsigned = 1;

		int resCode = 1;
		if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
			resCode = 0;
		}

		mysql_stmt_close(stmt);
		return resCode;
	}

	int LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
			char (*PlayerNames)[30], uint32 *CharacterIDs) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL || !NumberOfPlayers) return 1;

		*NumberOfPlayers = 0;

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt) return 1;

		const char *sql = "SELECT CharacterID, Name FROM Characters WHERE CharacterID > ? ORDER BY CharacterID ASC LIMIT 10000";
		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		int minCid = MinimumCharacterID;
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &minCid;

		if(mysql_stmt_bind_param(stmt, params) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return 1;
		}

		unsigned int cid = 0;
		char name[64] = {0};
		unsigned long nameLen = 0;

		MYSQL_BIND results[2];
		memset(results, 0, sizeof(results));

		results[0].buffer_type = MYSQL_TYPE_LONG;
		results[0].buffer = &cid;
		results[0].is_unsigned = 1;

		results[1].buffer_type = MYSQL_TYPE_STRING;
		results[1].buffer = name;
		results[1].buffer_length = sizeof(name) - 1;
		results[1].length = &nameLen;

		if(mysql_stmt_bind_result(stmt, results) == 0 && mysql_stmt_store_result(stmt) == 0){
			int count = 0;
			while(mysql_stmt_fetch(stmt) == 0 && count < 10000){
				name[nameLen < sizeof(name) ? nameLen : sizeof(name) - 1] = 0;
				if(CharacterIDs) CharacterIDs[count] = (uint32)cid;
				if(PlayerNames){
					strncpy(PlayerNames[count], name, 29);
					PlayerNames[count][29] = 0;
				}
				count++;
			}
			*NumberOfPlayers = count;
			mysql_stmt_close(stmt);
			return 0;
		}

		mysql_stmt_close(stmt);
		return 1;
	}
};

std::unique_ptr<IDatabaseDriver> CreateMySQLDriver(const char *host, int port, const char *user, const char *pass, const char *db, bool autoReconnect){
	return std::make_unique<MySQLDriver>(host, port, user, pass, db, autoReconnect);
}
