#include "db_driver.hh"
#include "db_common.hh"
#include "sqlite3.h"
#include <mutex>
#include <string>
#include <vector>
#include <ctime>

#define SQLITE_APPLICATION_ID 0x54694442
#define SQLITE_USER_VERSION 1

class SQLiteDriver : public IDatabaseDriver {
private:
	sqlite3 *m_DB = nullptr;
	std::mutex m_Mutex;
	std::string m_DatabaseFile;

	bool ExecSQL(const char *SQL){
		char *ErrMsg = nullptr;
		if(sqlite3_exec(m_DB, SQL, nullptr, nullptr, &ErrMsg) != SQLITE_OK){
			LOG_ERR("Error executing SQL: %s", ErrMsg ? ErrMsg : "unknown");
			if(ErrMsg) sqlite3_free(ErrMsg);
			return false;
		}
		return true;
	}

public:
	SQLiteDriver(const char *dbFile)
		: m_DatabaseFile(dbFile && dbFile[0] ? dbFile : "tibia.db") {}

	virtual ~SQLiteDriver(){
		Exit();
	}

	const char *GetName() const override {
		return "SQLite 3";
	}

	bool Init() override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB != nullptr) return true;

		int rc = sqlite3_open(m_DatabaseFile.c_str(), &m_DB);
		if(rc != SQLITE_OK){
			LOG_ERR("Failed to open SQLite database \"%s\": %s", m_DatabaseFile.c_str(), sqlite3_errmsg(m_DB));
			return false;
		}

		// SQLite optimizations
		ExecSQL("PRAGMA journal_mode = WAL;");
		ExecSQL("PRAGMA synchronous = NORMAL;");
		ExecSQL("PRAGMA foreign_keys = ON;");

		// Check if schema exists
		sqlite3_stmt *stmt = nullptr;
		rc = sqlite3_prepare_v2(m_DB, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='Accounts'", -1, &stmt, nullptr);
		int tableCount = 0;
		if(rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW){
			tableCount = sqlite3_column_int(stmt, 0);
		}
		if(stmt) sqlite3_finalize(stmt);

		if(tableCount == 0){
			ExecuteSchema();
		}

		// Clear residual online status from previous restarts
		ExecSQL("UPDATE Characters SET IsOnline = 0;");
		return true;
	}

	void Exit() override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB != nullptr){
			sqlite3_close(m_DB);
			m_DB = nullptr;
		}
	}

	bool ExecuteSchema() override {
		const char *schemaCandidates[] = {
			"schema.sql",
			"../schema.sql",
			"../../schema.sql",
			"../tibia-server/schema.sql",
			"tibia-server/schema.sql"
		};
		const char *schemaPath = nullptr;
		for(const char *cand : schemaCandidates){
			if(FileExists(cand)){
				schemaPath = cand;
				break;
			}
		}

		if(schemaPath){
			FILE *f = fopen(schemaPath, "rb");
			if(f){
				fseek(f, 0, SEEK_END);
				long sz = ftell(f);
				fseek(f, 0, SEEK_SET);
				char *sql = (char*)malloc(sz + 1);
				if(sql){
					fread(sql, 1, sz, f);
					sql[sz] = 0;
					ExecSQL(sql);
					free(sql);
				}
				fclose(f);
				return true;
			}
		} else {
			LOG_WARN("File schema.sql not found for automatic SQLite initialization.");
		}
		return false;
	}

	int LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
			int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr || NumCharacters == nullptr || Characters == nullptr || PremiumDays == nullptr) return 1;

		*NumCharacters = 0;
		*PremiumDays = 0;

		// 1. Fetch Account
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ?1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) != SQLITE_OK){
			return 1;
		}

		sqlite3_bind_int(stmt, 1, (int)AccountID);
		int rc = sqlite3_step(stmt);
		if(rc != SQLITE_ROW){
			sqlite3_finalize(stmt);
			return 1; // Invalid account number
		}

		const void *authBlob = sqlite3_column_blob(stmt, 0);
		int authBlobSize = sqlite3_column_bytes(stmt, 0);
		int64 premiumEnd = sqlite3_column_int64(stmt, 1);
		int deleted = sqlite3_column_int(stmt, 2);

		if(deleted){
			sqlite3_finalize(stmt);
			return 1;
		}

		// Verify password
		if(!DB_VerifyPassword((const uint8*)authBlob, authBlobSize, Password)){
			sqlite3_finalize(stmt);
			return 2; // Invalid password
		}

		int64 now = (int64)time(nullptr);
		if(premiumEnd > now){
			*PremiumDays = (int)((premiumEnd - now) / 86400);
			if(*PremiumDays <= 0) *PremiumDays = 1;
		} else {
			*PremiumDays = 0;
		}
		sqlite3_finalize(stmt);

		// 2. Fetch Characters of Account
		const char *charSql = "SELECT C.Name, W.Name, W.Host, W.Port "
							  "FROM Characters AS C "
							  "JOIN Worlds AS W ON C.WorldID = W.WorldID "
							  "WHERE C.AccountID = ?1 AND C.Deleted = 0 "
							  "ORDER BY C.Level DESC, C.LastLoginTime DESC LIMIT ?2";
		if(sqlite3_prepare_v2(m_DB, charSql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)AccountID);
			sqlite3_bind_int(stmt, 2, MaxCharacters);

			int count = 0;
			while(sqlite3_step(stmt) == SQLITE_ROW && count < MaxCharacters){
				const char *charName = (const char*)sqlite3_column_text(stmt, 0);
				const char *worldName = (const char*)sqlite3_column_text(stmt, 1);
				const char *host = (const char*)sqlite3_column_text(stmt, 2);
				int port = sqlite3_column_int(stmt, 3);

				StringCopy(Characters[count].Name, sizeof(Characters[count].Name), charName ? charName : "");
				StringCopy(Characters[count].WorldName, sizeof(Characters[count].WorldName), worldName ? worldName : "Tibia");
				Characters[count].WorldAddress = DB_ParseIPv4(host);
				Characters[count].WorldPort = port > 0 ? port : 7172;
				count++;
			}
			*NumCharacters = count;
			sqlite3_finalize(stmt);
		}

		return 0; // Success
	}

	int LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
			int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
			int *PremiumNewbieBuffer) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;

		sqlite3_stmt *stmt = nullptr;
		const char *sql = "SELECT Type, RebootTime, Host, Port, MaxPlayers, PremiumPlayerBuffer, MaxNewbies, PremiumNewbieBuffer FROM Worlds LIMIT 1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) != SQLITE_OK){
			return 1;
		}

		if(sqlite3_step(stmt) == SQLITE_ROW){
			if(WorldType) *WorldType = sqlite3_column_int(stmt, 0);
			if(RebootTime) *RebootTime = sqlite3_column_int(stmt, 1);
			if(IPAddress){
				const char *host = (const char*)sqlite3_column_text(stmt, 2);
				*IPAddress = (int)DB_ParseIPv4(host);
			}
			if(Port) *Port = sqlite3_column_int(stmt, 3);
			if(MaxPlayers) *MaxPlayers = sqlite3_column_int(stmt, 4);
			if(PremiumPlayerBuffer) *PremiumPlayerBuffer = sqlite3_column_int(stmt, 5);
			if(MaxNewbies) *MaxNewbies = sqlite3_column_int(stmt, 6);
			if(PremiumNewbieBuffer) *PremiumNewbieBuffer = sqlite3_column_int(stmt, 7);
			sqlite3_finalize(stmt);
			return 0;
		}

		sqlite3_finalize(stmt);
		return 1;
	}

	int LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
			const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
			bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
			char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
			char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;

		// 1. Validate Account
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ?1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) != SQLITE_OK){
			return 1;
		}
		sqlite3_bind_int(stmt, 1, (int)AccountID);
		if(sqlite3_step(stmt) != SQLITE_ROW){
			sqlite3_finalize(stmt);
			return 1; // Account not found
		}

		const void *authBlob = sqlite3_column_blob(stmt, 0);
		int authBlobSize = sqlite3_column_bytes(stmt, 0);
		int64 premiumEnd = sqlite3_column_int64(stmt, 1);
		int deleted = sqlite3_column_int(stmt, 2);

		if(deleted || !DB_VerifyPassword((const uint8*)authBlob, authBlobSize, Password)){
			sqlite3_finalize(stmt);
			return 2; // Invalid password
		}
		sqlite3_finalize(stmt);

		bool isPremium = (premiumEnd > (int64)time(nullptr));
		if(PremiumAccountActivated){
			*PremiumAccountActivated = isPremium;
		}

		// 2. Fetch Character
		const char *charSql = "SELECT CharacterID, Sex, Profession, Residence FROM Characters WHERE AccountID = ?1 AND Name = ?2 AND Deleted = 0";
		if(sqlite3_prepare_v2(m_DB, charSql, -1, &stmt, nullptr) != SQLITE_OK){
			return 3;
		}
		sqlite3_bind_int(stmt, 1, (int)AccountID);
		sqlite3_bind_text(stmt, 2, PlayerName, -1, SQLITE_STATIC);

		if(sqlite3_step(stmt) != SQLITE_ROW){
			sqlite3_finalize(stmt);
			return 3; // Character not found
		}

		uint32 cid = (uint32)sqlite3_column_int(stmt, 0);
		if(CharacterID) *CharacterID = cid;
		if(Sex) *Sex = sqlite3_column_int(stmt, 1);
		sqlite3_finalize(stmt);

		// 3. Fetch Rights
		if(Rights){
			memset(Rights, 0, 12);
			if(isPremium){
				DB_SetRightBit(Rights, 0); // PREMIUM_ACCOUNT
			}
			const char *rightSql = "SELECT Name FROM CharacterRights WHERE CharacterID = ?1";
			if(sqlite3_prepare_v2(m_DB, rightSql, -1, &stmt, nullptr) == SQLITE_OK){
				sqlite3_bind_int(stmt, 1, (int)cid);
				while(sqlite3_step(stmt) == SQLITE_ROW){
					const char *rName = (const char*)sqlite3_column_text(stmt, 0);
					if(rName){
						int r = DB_GetRightByName(rName);
						if(r >= 0){
							DB_SetRightBit(Rights, r);
						}
					}
				}
				sqlite3_finalize(stmt);
			}

			// Apply full GM rights if Gamemaster
			DB_ApplyGamemasterRights(AccountID, PlayerName, Rights);
		}

		// 4. Fetch Buddies
		if(NumberOfBuddies){
			*NumberOfBuddies = 0;
			if(BuddyIDs && BuddyNames){
				const char *buddySql = "SELECT B.BuddyID, C.Name FROM Buddies B "
									  "JOIN Characters C ON B.BuddyID = C.CharacterID "
									  "WHERE B.AccountID = ?1 LIMIT 100";
				if(sqlite3_prepare_v2(m_DB, buddySql, -1, &stmt, nullptr) == SQLITE_OK){
					sqlite3_bind_int(stmt, 1, (int)AccountID);
					int bCount = 0;
					while(sqlite3_step(stmt) == SQLITE_ROW && bCount < 100){
						BuddyIDs[bCount] = (uint32)sqlite3_column_int(stmt, 0);
						const char *bName = (const char*)sqlite3_column_text(stmt, 1);
						strncpy(BuddyNames[bCount], bName ? bName : "", 29);
						BuddyNames[bCount][29] = 0;
						bCount++;
					}
					*NumberOfBuddies = bCount;
					sqlite3_finalize(stmt);
				}
			}
		}

		if(Guild) Guild[0] = 0;
		if(Rank) Rank[0] = 0;
		if(Title) Title[0] = 0;

		// Mark online
		const char *updSql = "UPDATE Characters SET IsOnline = 1 WHERE CharacterID = ?1";
		if(sqlite3_prepare_v2(m_DB, updSql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)cid);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}

		return 0; // Success
	}

	int LogoutGame(uint32 CharacterID, int Level, const char *Profession,
			const char *Residence, time_t LastLoginTime, int TutorActivities) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;

		sqlite3_stmt *stmt = nullptr;
		const char *sql = "UPDATE Characters SET Level = ?1, Profession = ?2, Residence = ?3, LastLoginTime = ?4, TutorActivities = ?5, IsOnline = 0 WHERE CharacterID = ?6";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, Level);
			sqlite3_bind_text(stmt, 2, Profession ? Profession : "", -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 3, Residence ? Residence : "", -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 4, (int64)LastLoginTime);
			sqlite3_bind_int(stmt, 5, TutorActivities);
			sqlite3_bind_int(stmt, 6, (int)CharacterID);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
			const char *Remark, bool Unjustified, time_t Time) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;

		sqlite3_stmt *stmt = nullptr;
		const char *sql = "INSERT INTO CharacterDeaths (CharacterID, Level, OffenderID, Remark, Unjustified, Timestamp) VALUES (?1, ?2, ?3, ?4, ?5, ?6)";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)CharacterID);
			sqlite3_bind_int(stmt, 2, Level);
			sqlite3_bind_int(stmt, 3, (int)Offender);
			sqlite3_bind_text(stmt, 4, Remark ? Remark : "", -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 5, Unjustified ? 1 : 0);
			sqlite3_bind_int64(stmt, 6, (int64)Time);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "INSERT OR REPLACE INTO HouseOwners (WorldID, HouseID, OwnerID, PaidUntil) VALUES (1, ?1, ?2, ?3)";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, HouseID);
			sqlite3_bind_int(stmt, 2, (int)OwnerID);
			sqlite3_bind_int(stmt, 3, PaidUntil);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) override {
		return InsertHouseOwner(HouseID, OwnerID, PaidUntil);
	}

	int DeleteHouseOwner(uint16 HouseID) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "DELETE FROM HouseOwners WHERE HouseID = ?1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, HouseID);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
			uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr || NumberOfOwners == nullptr) return 1;
		int maxOwners = *NumberOfOwners;
		*NumberOfOwners = 0;

		sqlite3_stmt *stmt = nullptr;
		const char *sql = "SELECT HO.HouseID, HO.OwnerID, C.Name, HO.PaidUntil "
						  "FROM HouseOwners HO "
						  "JOIN Characters C ON HO.OwnerID = C.CharacterID "
						  "ORDER BY HO.HouseID ASC LIMIT ?1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, maxOwners);
			int count = 0;
			while(sqlite3_step(stmt) == SQLITE_ROW && count < maxOwners){
				if(HouseIDs) HouseIDs[count] = (uint16)sqlite3_column_int(stmt, 0);
				if(OwnerIDs) OwnerIDs[count] = (uint32)sqlite3_column_int(stmt, 1);
				const char *name = (const char*)sqlite3_column_text(stmt, 2);
				if(OwnerNames){
					strncpy(OwnerNames[count], name ? name : "", 29);
					OwnerNames[count][29] = 0;
				}
				if(PaidUntils) PaidUntils[count] = sqlite3_column_int(stmt, 3);
				count++;
			}
			*NumberOfOwners = count;
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int ClearIsOnline(int *NumberOfAffectedPlayers) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;
		ExecSQL("UPDATE Characters SET IsOnline = 0;");
		if(NumberOfAffectedPlayers) *NumberOfAffectedPlayers = 0;
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
		if(m_DB == nullptr) return 1;
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "INSERT OR IGNORE INTO Buddies (WorldID, AccountID, BuddyID) VALUES (1, ?1, ?2)";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)AccountID);
			sqlite3_bind_int(stmt, 2, (int)Buddy);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int RemoveBuddy(uint32 AccountID, uint32 Buddy) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "DELETE FROM Buddies WHERE AccountID = ?1 AND BuddyID = ?2";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)AccountID);
			sqlite3_bind_int(stmt, 2, (int)Buddy);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int DecrementIsOnline(uint32 CharacterID) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr) return 1;
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "UPDATE Characters SET IsOnline = 0 WHERE CharacterID = ?1";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, (int)CharacterID);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}

	int LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
			char (*PlayerNames)[30], uint32 *CharacterIDs) override {
		std::lock_guard<std::mutex> lock(m_Mutex);
		if(m_DB == nullptr || NumberOfPlayers == nullptr) return 1;
		*NumberOfPlayers = 0;

		sqlite3_stmt *stmt = nullptr;
		const char *sql = "SELECT CharacterID, Name FROM Characters WHERE CharacterID > ?1 ORDER BY CharacterID ASC LIMIT 10000;";
		if(sqlite3_prepare_v2(m_DB, sql, -1, &stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(stmt, 1, MinimumCharacterID);
			int count = 0;
			while(sqlite3_step(stmt) == SQLITE_ROW && count < 10000){
				uint32 cid = (uint32)sqlite3_column_int(stmt, 0);
				const char *name = (const char*)sqlite3_column_text(stmt, 1);
				if(CharacterIDs) CharacterIDs[count] = cid;
				if(PlayerNames && name){
					strncpy(PlayerNames[count], name, 29);
					PlayerNames[count][29] = 0;
				}
				count++;
			}
			*NumberOfPlayers = count;
			sqlite3_finalize(stmt);
			return 0;
		}
		return 1;
	}
};

std::unique_ptr<IDatabaseDriver> CreateSQLiteDriver(const char *dbPath){
	return std::make_unique<SQLiteDriver>(dbPath);
}
