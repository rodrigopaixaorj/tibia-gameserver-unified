#include "db_driver.hh"
#include "db_common.hh"
#include "../game/cr.hh"

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

		// Fetch Guild, Rank, and Title from GuildMembers
		if(Guild && Rank && Title){
			stmt = mysql_stmt_init(m_MySQL);
			if(stmt){
				const char *gSql = "SELECT G.Name, GR.Name, GM.Title FROM GuildMembers GM "
								   "JOIN Guilds G ON GM.GuildID = G.GuildID "
								   "JOIN GuildRanks GR ON GM.GuildID = GR.GuildID AND GM.Rank = GR.Rank "
								   "WHERE GM.CharacterID = ? LIMIT 1";
				if(mysql_stmt_prepare(stmt, gSql, (unsigned long)strlen(gSql)) == 0){
					MYSQL_BIND gParams[1];
					memset(gParams, 0, sizeof(gParams));
					gParams[0].buffer_type = MYSQL_TYPE_LONG;
					gParams[0].buffer = &cid;
					gParams[0].is_unsigned = 1;

					if(mysql_stmt_bind_param(stmt, gParams) == 0 && mysql_stmt_execute(stmt) == 0){
						char gName[64] = {0}, grName[64] = {0}, gmTitle[64] = {0};
						unsigned long gLen = 0, grLen = 0, gtLen = 0;
						MYSQL_BIND gRes[3];
						memset(gRes, 0, sizeof(gRes));
						gRes[0].buffer_type = MYSQL_TYPE_STRING; gRes[0].buffer = gName; gRes[0].buffer_length = sizeof(gName) - 1; gRes[0].length = &gLen;
						gRes[1].buffer_type = MYSQL_TYPE_STRING; gRes[1].buffer = grName; gRes[1].buffer_length = sizeof(grName) - 1; gRes[1].length = &grLen;
						gRes[2].buffer_type = MYSQL_TYPE_STRING; gRes[2].buffer = gmTitle; gRes[2].buffer_length = sizeof(gmTitle) - 1; gRes[2].length = &gtLen;

						if(mysql_stmt_bind_result(stmt, gRes) == 0 && mysql_stmt_store_result(stmt) == 0){
							if(mysql_stmt_fetch(stmt) == 0){
								strncpy(Guild, gName, 30); Guild[30] = 0;
								strncpy(Rank, grName, 30); Rank[30] = 0;
								strncpy(Title, gmTitle, 30); Title[30] = 0;
							}
						}
					}
				}
				mysql_stmt_close(stmt);
			}
		}

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

	bool SaveCharacterState(const TPlayerData *Slot) override {
		if(!Slot || Slot->CharacterID == 0) return false;
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return false;

		ExecSimpleSQL("START TRANSACTION;");

		// 1. Save PlayerState
		const char *stateSql = "REPLACE INTO PlayerState ("
			"CharacterID, Race, Profession, "
			"OrigLookType, OrigLookHead, OrigLookBody, OrigLookLegs, OrigLookFeet, "
			"CurrLookType, CurrLookHead, CurrLookBody, CurrLookLegs, CurrLookFeet, "
			"StartX, StartY, StartZ, PosX, PosY, PosZ, PlayerkillerEnd, Spells, Quests, Murders) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt || mysql_stmt_prepare(stmt, stateSql, (unsigned long)strlen(stateSql)) != 0){
			if(stmt) mysql_stmt_close(stmt);
			ExecSimpleSQL("ROLLBACK;");
			return false;
		}

		MYSQL_BIND params[23];
		memset(params, 0, sizeof(params));

		unsigned int cid = Slot->CharacterID;
		int race = Slot->Race;
		int prof = Slot->Profession;

		int origType = Slot->OriginalOutfit.OutfitID;
		int origHead = Slot->OriginalOutfit.Colors[0];
		int origBody = Slot->OriginalOutfit.Colors[1];
		int origLegs = Slot->OriginalOutfit.Colors[2];
		int origFeet = Slot->OriginalOutfit.Colors[3];

		int currType = Slot->CurrentOutfit.OutfitID;
		int currHead = Slot->CurrentOutfit.Colors[0];
		int currBody = Slot->CurrentOutfit.Colors[1];
		int currLegs = Slot->CurrentOutfit.Colors[2];
		int currFeet = Slot->CurrentOutfit.Colors[3];

		int sx = Slot->startx, sy = Slot->starty, sz = Slot->startz;
		int px = Slot->posx, py = Slot->posy, pz = Slot->posz;
		int pkEnd = Slot->PlayerkillerEnd;

		params[0].buffer_type = MYSQL_TYPE_LONG; params[0].buffer = &cid; params[0].is_unsigned = 1;
		params[1].buffer_type = MYSQL_TYPE_LONG; params[1].buffer = &race;
		params[2].buffer_type = MYSQL_TYPE_LONG; params[2].buffer = &prof;

		params[3].buffer_type = MYSQL_TYPE_LONG; params[3].buffer = &origType;
		params[4].buffer_type = MYSQL_TYPE_LONG; params[4].buffer = &origHead;
		params[5].buffer_type = MYSQL_TYPE_LONG; params[5].buffer = &origBody;
		params[6].buffer_type = MYSQL_TYPE_LONG; params[6].buffer = &origLegs;
		params[7].buffer_type = MYSQL_TYPE_LONG; params[7].buffer = &origFeet;

		params[8].buffer_type = MYSQL_TYPE_LONG; params[8].buffer = &currType;
		params[9].buffer_type = MYSQL_TYPE_LONG; params[9].buffer = &currHead;
		params[10].buffer_type = MYSQL_TYPE_LONG; params[10].buffer = &currBody;
		params[11].buffer_type = MYSQL_TYPE_LONG; params[11].buffer = &currLegs;
		params[12].buffer_type = MYSQL_TYPE_LONG; params[12].buffer = &currFeet;

		params[13].buffer_type = MYSQL_TYPE_LONG; params[13].buffer = &sx;
		params[14].buffer_type = MYSQL_TYPE_LONG; params[14].buffer = &sy;
		params[15].buffer_type = MYSQL_TYPE_LONG; params[15].buffer = &sz;
		params[16].buffer_type = MYSQL_TYPE_LONG; params[16].buffer = &px;
		params[17].buffer_type = MYSQL_TYPE_LONG; params[17].buffer = &py;
		params[18].buffer_type = MYSQL_TYPE_LONG; params[18].buffer = &pz;
		params[19].buffer_type = MYSQL_TYPE_LONG; params[19].buffer = &pkEnd;

		unsigned long spellsLen = sizeof(Slot->SpellList);
		params[20].buffer_type = MYSQL_TYPE_BLOB;
		params[20].buffer = (void*)Slot->SpellList;
		params[20].buffer_length = spellsLen;
		params[20].length = &spellsLen;

		unsigned long questsLen = sizeof(Slot->QuestValues);
		params[21].buffer_type = MYSQL_TYPE_BLOB;
		params[21].buffer = (void*)Slot->QuestValues;
		params[21].buffer_length = questsLen;
		params[21].length = &questsLen;

		unsigned long murdersLen = sizeof(Slot->MurderTimestamps);
		params[22].buffer_type = MYSQL_TYPE_BLOB;
		params[22].buffer = (void*)Slot->MurderTimestamps;
		params[22].buffer_length = murdersLen;
		params[22].length = &murdersLen;

		if(mysql_stmt_bind_param(stmt, params) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			ExecSimpleSQL("ROLLBACK;");
			return false;
		}
		mysql_stmt_close(stmt);

		// 2. Save PlayerSkills
		const char *skillSql = "REPLACE INTO PlayerSkills ("
			"CharacterID, SkillID, Actual, Maximum, Minimum, DeltaAct, MagicDeltaAct, "
			"Cycle, MaxCycle, Count, MaxCount, AddLevel, Experience, FactorPercent, NextLevel, Delta) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

		stmt = mysql_stmt_init(m_MySQL);
		if(stmt && mysql_stmt_prepare(stmt, skillSql, (unsigned long)strlen(skillSql)) == 0){
			for(int i = 0; i < 25; i++){
				if(Slot->Minimum[i] == INT_MIN) continue;

				MYSQL_BIND sp[16];
				memset(sp, 0, sizeof(sp));
				unsigned char sid = (unsigned char)i;
				int act = Slot->Actual[i], mx = Slot->Maximum[i], mn = Slot->Minimum[i];
				int dact = Slot->DeltaAct[i], mdact = Slot->MagicDeltaAct[i];
				int cyc = Slot->Cycle[i], mcyc = Slot->MaxCycle[i];
				int cnt = Slot->Count[i], mcnt = Slot->MaxCount[i];
				int al = Slot->AddLevel[i], exp = Slot->Experience[i], fp = Slot->FactorPercent[i];
				int nl = Slot->NextLevel[i], delta = Slot->Delta[i];

				sp[0].buffer_type = MYSQL_TYPE_LONG; sp[0].buffer = &cid; sp[0].is_unsigned = 1;
				sp[1].buffer_type = MYSQL_TYPE_TINY; sp[1].buffer = &sid; sp[1].is_unsigned = 1;
				sp[2].buffer_type = MYSQL_TYPE_LONG; sp[2].buffer = &act;
				sp[3].buffer_type = MYSQL_TYPE_LONG; sp[3].buffer = &mx;
				sp[4].buffer_type = MYSQL_TYPE_LONG; sp[4].buffer = &mn;
				sp[5].buffer_type = MYSQL_TYPE_LONG; sp[5].buffer = &dact;
				sp[6].buffer_type = MYSQL_TYPE_LONG; sp[6].buffer = &mdact;
				sp[7].buffer_type = MYSQL_TYPE_LONG; sp[7].buffer = &cyc;
				sp[8].buffer_type = MYSQL_TYPE_LONG; sp[8].buffer = &mcyc;
				sp[9].buffer_type = MYSQL_TYPE_LONG; sp[9].buffer = &cnt;
				sp[10].buffer_type = MYSQL_TYPE_LONG; sp[10].buffer = &mcnt;
				sp[11].buffer_type = MYSQL_TYPE_LONG; sp[11].buffer = &al;
				sp[12].buffer_type = MYSQL_TYPE_LONG; sp[12].buffer = &exp;
				sp[13].buffer_type = MYSQL_TYPE_LONG; sp[13].buffer = &fp;
				sp[14].buffer_type = MYSQL_TYPE_LONG; sp[14].buffer = &nl;
				sp[15].buffer_type = MYSQL_TYPE_LONG; sp[15].buffer = &delta;

				if(mysql_stmt_bind_param(stmt, sp) == 0){
					mysql_stmt_execute(stmt);
				}
			}
			mysql_stmt_close(stmt);
		}

		// 3. Save PlayerInventory
		if(Slot->Inventory && Slot->InventorySize > 0){
			const char *invSql = "REPLACE INTO PlayerInventory (CharacterID, Data) VALUES (?, ?);";
			stmt = mysql_stmt_init(m_MySQL);
			if(stmt && mysql_stmt_prepare(stmt, invSql, (unsigned long)strlen(invSql)) == 0){
				MYSQL_BIND ip[2];
				memset(ip, 0, sizeof(ip));
				unsigned long invLen = (unsigned long)Slot->InventorySize;
				ip[0].buffer_type = MYSQL_TYPE_LONG; ip[0].buffer = &cid; ip[0].is_unsigned = 1;
				ip[1].buffer_type = MYSQL_TYPE_BLOB; ip[1].buffer = Slot->Inventory;
				ip[1].buffer_length = invLen; ip[1].length = &invLen;

				if(mysql_stmt_bind_param(stmt, ip) == 0){
					mysql_stmt_execute(stmt);
				}
				mysql_stmt_close(stmt);
			}
		} else {
			const char *delSql = "DELETE FROM PlayerInventory WHERE CharacterID = ?;";
			stmt = mysql_stmt_init(m_MySQL);
			if(stmt && mysql_stmt_prepare(stmt, delSql, (unsigned long)strlen(delSql)) == 0){
				MYSQL_BIND dp[1];
				memset(dp, 0, sizeof(dp));
				dp[0].buffer_type = MYSQL_TYPE_LONG; dp[0].buffer = &cid; dp[0].is_unsigned = 1;
				if(mysql_stmt_bind_param(stmt, dp) == 0){
					mysql_stmt_execute(stmt);
				}
				mysql_stmt_close(stmt);
			}
		}

		// 4. Save PlayerDepots
		for(int d = 0; d < MAX_DEPOTS; d++){
			if(Slot->Depot[d] && Slot->DepotSize[d] > 0){
				const char *depotSql = "REPLACE INTO PlayerDepots (CharacterID, DepotID, Data) VALUES (?, ?, ?);";
				stmt = mysql_stmt_init(m_MySQL);
				if(stmt && mysql_stmt_prepare(stmt, depotSql, (unsigned long)strlen(depotSql)) == 0){
					MYSQL_BIND dp[3];
					memset(dp, 0, sizeof(dp));
					unsigned char depId = (unsigned char)d;
					unsigned long depLen = (unsigned long)Slot->DepotSize[d];
					dp[0].buffer_type = MYSQL_TYPE_LONG; dp[0].buffer = &cid; dp[0].is_unsigned = 1;
					dp[1].buffer_type = MYSQL_TYPE_TINY; dp[1].buffer = &depId; dp[1].is_unsigned = 1;
					dp[2].buffer_type = MYSQL_TYPE_BLOB; dp[2].buffer = Slot->Depot[d];
					dp[2].buffer_length = depLen; dp[2].length = &depLen;

					if(mysql_stmt_bind_param(stmt, dp) == 0){
						mysql_stmt_execute(stmt);
					}
					mysql_stmt_close(stmt);
				}
			} else {
				const char *delDepSql = "DELETE FROM PlayerDepots WHERE CharacterID = ? AND DepotID = ?;";
				stmt = mysql_stmt_init(m_MySQL);
				if(stmt && mysql_stmt_prepare(stmt, delDepSql, (unsigned long)strlen(delDepSql)) == 0){
					MYSQL_BIND dp[2];
					memset(dp, 0, sizeof(dp));
					unsigned char depId = (unsigned char)d;
					dp[0].buffer_type = MYSQL_TYPE_LONG; dp[0].buffer = &cid; dp[0].is_unsigned = 1;
					dp[1].buffer_type = MYSQL_TYPE_TINY; dp[1].buffer = &depId; dp[1].is_unsigned = 1;
					if(mysql_stmt_bind_param(stmt, dp) == 0){
						mysql_stmt_execute(stmt);
					}
					mysql_stmt_close(stmt);
				}
			}
		}

		ExecSimpleSQL("COMMIT;");
		return true;
	}

	bool LoadCharacterState(TPlayerData *Slot) override {
		if(!Slot || Slot->CharacterID == 0) return false;
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(!m_MySQL) return false;

		// 1. Query PlayerState
		const char *stateSql = "SELECT Race, Profession, "
			"OrigLookType, OrigLookHead, OrigLookBody, OrigLookLegs, OrigLookFeet, "
			"CurrLookType, CurrLookHead, CurrLookBody, CurrLookLegs, CurrLookFeet, "
			"StartX, StartY, StartZ, PosX, PosY, PosZ, PlayerkillerEnd, Spells, Quests, Murders "
			"FROM PlayerState WHERE CharacterID = ?;";

		MYSQL_STMT *stmt = mysql_stmt_init(m_MySQL);
		if(!stmt || mysql_stmt_prepare(stmt, stateSql, (unsigned long)strlen(stateSql)) != 0){
			if(stmt) mysql_stmt_close(stmt);
			return false;
		}

		unsigned int cid = Slot->CharacterID;
		MYSQL_BIND params[1];
		memset(params, 0, sizeof(params));
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &cid;
		params[0].is_unsigned = 1;

		if(mysql_stmt_bind_param(stmt, params) != 0 || mysql_stmt_execute(stmt) != 0){
			mysql_stmt_close(stmt);
			return false;
		}

		int race = 0, prof = 0;
		int origType = 0, origHead = 0, origBody = 0, origLegs = 0, origFeet = 0;
		int currType = 0, currHead = 0, currBody = 0, currLegs = 0, currFeet = 0;
		int sx = 0, sy = 0, sz = 0, px = 0, py = 0, pz = 0, pkEnd = 0;
		uint8 spellsBuf[256] = {0}; unsigned long spellsLen = 0;
		int questsBuf[500] = {0}; unsigned long questsLen = 0;
		int murdersBuf[20] = {0}; unsigned long murdersLen = 0;

		MYSQL_BIND results[22];
		memset(results, 0, sizeof(results));
		results[0].buffer_type = MYSQL_TYPE_LONG; results[0].buffer = &race;
		results[1].buffer_type = MYSQL_TYPE_LONG; results[1].buffer = &prof;
		results[2].buffer_type = MYSQL_TYPE_LONG; results[2].buffer = &origType;
		results[3].buffer_type = MYSQL_TYPE_LONG; results[3].buffer = &origHead;
		results[4].buffer_type = MYSQL_TYPE_LONG; results[4].buffer = &origBody;
		results[5].buffer_type = MYSQL_TYPE_LONG; results[5].buffer = &origLegs;
		results[6].buffer_type = MYSQL_TYPE_LONG; results[6].buffer = &origFeet;
		results[7].buffer_type = MYSQL_TYPE_LONG; results[7].buffer = &currType;
		results[8].buffer_type = MYSQL_TYPE_LONG; results[8].buffer = &currHead;
		results[9].buffer_type = MYSQL_TYPE_LONG; results[9].buffer = &currBody;
		results[10].buffer_type = MYSQL_TYPE_LONG; results[10].buffer = &currLegs;
		results[11].buffer_type = MYSQL_TYPE_LONG; results[11].buffer = &currFeet;
		results[12].buffer_type = MYSQL_TYPE_LONG; results[12].buffer = &sx;
		results[13].buffer_type = MYSQL_TYPE_LONG; results[13].buffer = &sy;
		results[14].buffer_type = MYSQL_TYPE_LONG; results[14].buffer = &sz;
		results[15].buffer_type = MYSQL_TYPE_LONG; results[15].buffer = &px;
		results[16].buffer_type = MYSQL_TYPE_LONG; results[16].buffer = &py;
		results[17].buffer_type = MYSQL_TYPE_LONG; results[17].buffer = &pz;
		results[18].buffer_type = MYSQL_TYPE_LONG; results[18].buffer = &pkEnd;

		results[19].buffer_type = MYSQL_TYPE_BLOB; results[19].buffer = spellsBuf;
		results[19].buffer_length = sizeof(spellsBuf); results[19].length = &spellsLen;

		results[20].buffer_type = MYSQL_TYPE_BLOB; results[20].buffer = questsBuf;
		results[20].buffer_length = sizeof(questsBuf); results[20].length = &questsLen;

		results[21].buffer_type = MYSQL_TYPE_BLOB; results[21].buffer = murdersBuf;
		results[21].buffer_length = sizeof(murdersBuf); results[21].length = &murdersLen;

		if(mysql_stmt_bind_result(stmt, results) != 0 || mysql_stmt_store_result(stmt) != 0){
			mysql_stmt_close(stmt);
			return false;
		}

		if(mysql_stmt_fetch(stmt) != 0){
			mysql_stmt_close(stmt);
			return false; // Character not found in PlayerState table!
		}
		mysql_stmt_close(stmt);

		Slot->Race = race;
		Slot->Profession = prof;
		Slot->OriginalOutfit.OutfitID = origType;
		Slot->OriginalOutfit.Colors[0] = (uint8)origHead;
		Slot->OriginalOutfit.Colors[1] = (uint8)origBody;
		Slot->OriginalOutfit.Colors[2] = (uint8)origLegs;
		Slot->OriginalOutfit.Colors[3] = (uint8)origFeet;
		Slot->CurrentOutfit.OutfitID = currType;
		Slot->CurrentOutfit.Colors[0] = (uint8)currHead;
		Slot->CurrentOutfit.Colors[1] = (uint8)currBody;
		Slot->CurrentOutfit.Colors[2] = (uint8)currLegs;
		Slot->CurrentOutfit.Colors[3] = (uint8)currFeet;
		Slot->startx = sx; Slot->starty = sy; Slot->startz = sz;
		Slot->posx = px; Slot->posy = py; Slot->posz = pz;
		Slot->PlayerkillerEnd = pkEnd;

		if(spellsLen > 0){
			memcpy(Slot->SpellList, spellsBuf, spellsLen < sizeof(Slot->SpellList) ? spellsLen : sizeof(Slot->SpellList));
		}
		if(questsLen > 0){
			memcpy(Slot->QuestValues, questsBuf, questsLen < sizeof(Slot->QuestValues) ? questsLen : sizeof(Slot->QuestValues));
		}
		if(murdersLen > 0){
			memcpy(Slot->MurderTimestamps, murdersBuf, murdersLen < sizeof(Slot->MurderTimestamps) ? murdersLen : sizeof(Slot->MurderTimestamps));
		}

		// 2. Query PlayerSkills
		const char *skillSql = "SELECT SkillID, Actual, Maximum, Minimum, DeltaAct, MagicDeltaAct, "
			"Cycle, MaxCycle, Count, MaxCount, AddLevel, Experience, FactorPercent, NextLevel, Delta "
			"FROM PlayerSkills WHERE CharacterID = ?;";

		stmt = mysql_stmt_init(m_MySQL);
		if(stmt && mysql_stmt_prepare(stmt, skillSql, (unsigned long)strlen(skillSql)) == 0){
			memset(params, 0, sizeof(params));
			params[0].buffer_type = MYSQL_TYPE_LONG; params[0].buffer = &cid; params[0].is_unsigned = 1;

			if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
				unsigned char sid = 0;
				int act = 0, mx = 0, mn = 0, dact = 0, mdact = 0;
				int cyc = 0, mcyc = 0, cnt = 0, mcnt = 0, al = 0, exp = 0, fp = 0, nl = 0, delta = 0;

				MYSQL_BIND sres[15];
				memset(sres, 0, sizeof(sres));
				sres[0].buffer_type = MYSQL_TYPE_TINY; sres[0].buffer = &sid; sres[0].is_unsigned = 1;
				sres[1].buffer_type = MYSQL_TYPE_LONG; sres[1].buffer = &act;
				sres[2].buffer_type = MYSQL_TYPE_LONG; sres[2].buffer = &mx;
				sres[3].buffer_type = MYSQL_TYPE_LONG; sres[3].buffer = &mn;
				sres[4].buffer_type = MYSQL_TYPE_LONG; sres[4].buffer = &dact;
				sres[5].buffer_type = MYSQL_TYPE_LONG; sres[5].buffer = &mdact;
				sres[6].buffer_type = MYSQL_TYPE_LONG; sres[6].buffer = &cyc;
				sres[7].buffer_type = MYSQL_TYPE_LONG; sres[7].buffer = &mcyc;
				sres[8].buffer_type = MYSQL_TYPE_LONG; sres[8].buffer = &cnt;
				sres[9].buffer_type = MYSQL_TYPE_LONG; sres[9].buffer = &mcnt;
				sres[10].buffer_type = MYSQL_TYPE_LONG; sres[10].buffer = &al;
				sres[11].buffer_type = MYSQL_TYPE_LONG; sres[11].buffer = &exp;
				sres[12].buffer_type = MYSQL_TYPE_LONG; sres[12].buffer = &fp;
				sres[13].buffer_type = MYSQL_TYPE_LONG; sres[13].buffer = &nl;
				sres[14].buffer_type = MYSQL_TYPE_LONG; sres[14].buffer = &delta;

				if(mysql_stmt_bind_result(stmt, sres) == 0 && mysql_stmt_store_result(stmt) == 0){
					while(mysql_stmt_fetch(stmt) == 0){
						if(sid < 25){
							Slot->Actual[sid] = act;
							Slot->Maximum[sid] = mx;
							Slot->Minimum[sid] = mn;
							Slot->DeltaAct[sid] = dact;
							Slot->MagicDeltaAct[sid] = mdact;
							Slot->Cycle[sid] = cyc;
							Slot->MaxCycle[sid] = mcyc;
							Slot->Count[sid] = cnt;
							Slot->MaxCount[sid] = mcnt;
							Slot->AddLevel[sid] = al;
							Slot->Experience[sid] = exp;
							Slot->FactorPercent[sid] = fp;
							Slot->NextLevel[sid] = nl;
							Slot->Delta[sid] = delta;
						}
					}
				}
			}
			mysql_stmt_close(stmt);
		}

		// 3. Query PlayerInventory
		const char *invSql = "SELECT Data FROM PlayerInventory WHERE CharacterID = ?;";
		stmt = mysql_stmt_init(m_MySQL);
		if(stmt && mysql_stmt_prepare(stmt, invSql, (unsigned long)strlen(invSql)) == 0){
			memset(params, 0, sizeof(params));
			params[0].buffer_type = MYSQL_TYPE_LONG; params[0].buffer = &cid; params[0].is_unsigned = 1;
			if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
				mysql_stmt_store_result(stmt);
				MYSQL_BIND ires[1];
				memset(ires, 0, sizeof(ires));
				unsigned long blobLen = 0;
				ires[0].buffer_type = MYSQL_TYPE_BLOB;
				ires[0].buffer = nullptr;
				ires[0].buffer_length = 0;
				ires[0].length = &blobLen;
				mysql_stmt_bind_result(stmt, ires);

				int fetchRet = mysql_stmt_fetch(stmt);
				if(fetchRet == 0 || fetchRet == MYSQL_DATA_TRUNCATED){
					if(blobLen > 0){
						delete[] Slot->Inventory;
						Slot->Inventory = new uint8[blobLen];
						Slot->InventorySize = (int)blobLen;

						MYSQL_BIND colBind;
						memset(&colBind, 0, sizeof(colBind));
						colBind.buffer_type = MYSQL_TYPE_BLOB;
						colBind.buffer = Slot->Inventory;
						colBind.buffer_length = blobLen;
						colBind.length = &blobLen;
						mysql_stmt_fetch_column(stmt, &colBind, 0, 0);
					}
				}
			}
			mysql_stmt_close(stmt);
		}

		// 4. Query PlayerDepots
		const char *depSql = "SELECT DepotID, Data FROM PlayerDepots WHERE CharacterID = ?;";
		stmt = mysql_stmt_init(m_MySQL);
		if(stmt && mysql_stmt_prepare(stmt, depSql, (unsigned long)strlen(depSql)) == 0){
			memset(params, 0, sizeof(params));
			params[0].buffer_type = MYSQL_TYPE_LONG; params[0].buffer = &cid; params[0].is_unsigned = 1;
			if(mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0){
				mysql_stmt_store_result(stmt);
				unsigned char did = 0;
				unsigned long blobLen = 0;
				MYSQL_BIND dres[2];
				memset(dres, 0, sizeof(dres));
				dres[0].buffer_type = MYSQL_TYPE_TINY; dres[0].buffer = &did; dres[0].is_unsigned = 1;
				dres[1].buffer_type = MYSQL_TYPE_BLOB;
				dres[1].buffer = nullptr;
				dres[1].buffer_length = 0;
				dres[1].length = &blobLen;
				mysql_stmt_bind_result(stmt, dres);

				int fetchRet;
				while((fetchRet = mysql_stmt_fetch(stmt)) == 0 || fetchRet == MYSQL_DATA_TRUNCATED){
					if(did < MAX_DEPOTS && blobLen > 0){
						delete[] Slot->Depot[did];
						Slot->Depot[did] = new uint8[blobLen];
						Slot->DepotSize[did] = (int)blobLen;

						MYSQL_BIND colBind;
						memset(&colBind, 0, sizeof(colBind));
						colBind.buffer_type = MYSQL_TYPE_BLOB;
						colBind.buffer = Slot->Depot[did];
						colBind.buffer_length = blobLen;
						colBind.length = &blobLen;
						mysql_stmt_fetch_column(stmt, &colBind, 1, 0);
					}
					// Reset dres[1] for next row fetch
					blobLen = 0;
					dres[1].buffer = nullptr;
					dres[1].buffer_length = 0;
				}
			}
			mysql_stmt_close(stmt);
		}

		return true;
	}
};

std::unique_ptr<IDatabaseDriver> CreateMySQLDriver(const char *host, int port, const char *user, const char *pass, const char *db, bool autoReconnect){
	return std::make_unique<MySQLDriver>(host, port, user, pass, db, autoReconnect);
}
