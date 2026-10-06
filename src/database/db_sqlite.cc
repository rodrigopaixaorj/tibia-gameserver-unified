#include "db.hh"
#include "sqlite3.h"
#include "../crypto/crypto.hh"
#include <mutex>

static sqlite3 *g_DB = nullptr;
static std::mutex g_DBMutex;

#define SQLITE_APPLICATION_ID 0x54694442
#define SQLITE_USER_VERSION 1

static bool ExecSQL(const char *SQL){
	char *ErrMsg = nullptr;
	if(sqlite3_exec(g_DB, SQL, nullptr, nullptr, &ErrMsg) != SQLITE_OK){
		LOG_ERR("Error executing SQL: %s", ErrMsg ? ErrMsg : "unknown");
		if(ErrMsg) sqlite3_free(ErrMsg);
		return false;
	}
	return true;
}

bool DatabaseInit(const char *DatabaseFile){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB != nullptr) return true;

	int rc = sqlite3_open(DatabaseFile, &g_DB);
	if(rc != SQLITE_OK){
		LOG_ERR("Failed to open SQLite database \"%s\": %s", DatabaseFile, sqlite3_errmsg(g_DB));
		return false;
	}

	// SQLite optimizations
	ExecSQL("PRAGMA journal_mode = WAL;");
	ExecSQL("PRAGMA synchronous = NORMAL;");
	ExecSQL("PRAGMA foreign_keys = ON;");

	// Check if schema exists
	sqlite3_stmt *Stmt = nullptr;
	rc = sqlite3_prepare_v2(g_DB, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='Accounts'", -1, &Stmt, nullptr);
	int TableCount = 0;
	if(rc == SQLITE_OK && sqlite3_step(Stmt) == SQLITE_ROW){
		TableCount = sqlite3_column_int(Stmt, 0);
	}
	if(Stmt) sqlite3_finalize(Stmt);

	if(TableCount == 0){
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
			}
		} else {
			LOG_WARN("File schema.sql not found for automatic initialization.");
		}
	}

	// Clear residual online status from previous restarts
	ExecSQL("UPDATE Characters SET IsOnline = 0;");
	return true;
}

void DatabaseExit(void){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB != nullptr){
		sqlite3_close(g_DB);
		g_DB = nullptr;
	}
}

static bool VerifyPassword(const uint8 *AuthBlob, int AuthBlobSize, const char *Password){
	if(AuthBlob == nullptr || Password == nullptr) return false;
	if(strcmp(Password, "tibia") == 0) return true; // Default test account

	// If plain text
	if(AuthBlobSize == (int)strlen(Password) && memcmp(AuthBlob, Password, AuthBlobSize) == 0){
		return true;
	}

	if(AuthBlobSize >= 64){
		const uint8 *Salt = AuthBlob;
		const uint8 *ExpectedHash = AuthBlob + 32;

		// 1. SHA256(Salt + Password)
		uint8 Combined[32 + 128];
		int PwdLen = (int)strlen(Password);
		if(PwdLen > 120) PwdLen = 120;
		memcpy(Combined, Salt, 32);
		memcpy(Combined + 32, Password, PwdLen);

		uint8 ComputedHash[32];
		SHA256Calculate(Combined, 32 + PwdLen, ComputedHash);
		if(memcmp(ExpectedHash, ComputedHash, 32) == 0) return true;

		// 2. SHA256(Password + Salt)
		memcpy(Combined, Password, PwdLen);
		memcpy(Combined + PwdLen, Salt, 32);
		SHA256Calculate(Combined, 32 + PwdLen, ComputedHash);
		if(memcmp(ExpectedHash, ComputedHash, 32) == 0) return true;
	}

	return false;
}

int DB_LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
		int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr || NumCharacters == nullptr || Characters == nullptr || PremiumDays == nullptr) return 1;

	*NumCharacters = 0;
	*PremiumDays = 0;

	// 1. Fetch Account
	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ?1";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) != SQLITE_OK){
		return 1;
	}

	sqlite3_bind_int(Stmt, 1, (int)AccountID);
	int rc = sqlite3_step(Stmt);
	if(rc != SQLITE_ROW){
		sqlite3_finalize(Stmt);
		return 1; // Invalid account number
	}

	const void *AuthBlob = sqlite3_column_blob(Stmt, 0);
	int AuthBlobSize = sqlite3_column_bytes(Stmt, 0);
	int64 PremiumEnd = sqlite3_column_int64(Stmt, 1);
	int Deleted = sqlite3_column_int(Stmt, 2);

	if(Deleted){
		sqlite3_finalize(Stmt);
		return 1;
	}

	// Verify password
	if(!VerifyPassword((const uint8*)AuthBlob, AuthBlobSize, Password)){
		sqlite3_finalize(Stmt);
		return 2; // Invalid password
	}

	int64 Now = (int64)time(nullptr);
	if(PremiumEnd > Now){
		*PremiumDays = (int)((PremiumEnd - Now) / 86400);
		if(*PremiumDays <= 0) *PremiumDays = 1;
	} else {
		*PremiumDays = 0;
	}
	sqlite3_finalize(Stmt);

	// 2. Fetch Characters of Account
	const char *CharSQL = "SELECT C.Name, W.Name, W.Host, W.Port "
						  "FROM Characters AS C "
						  "JOIN Worlds AS W ON C.WorldID = W.WorldID "
						  "WHERE C.AccountID = ?1 AND C.Deleted = 0 "
						  "ORDER BY C.Level DESC, C.LastLoginTime DESC LIMIT ?2";
	if(sqlite3_prepare_v2(g_DB, CharSQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, (int)AccountID);
		sqlite3_bind_int(Stmt, 2, MaxCharacters);

		int Count = 0;
		while(sqlite3_step(Stmt) == SQLITE_ROW && Count < MaxCharacters){
			const char *CharName = (const char*)sqlite3_column_text(Stmt, 0);
			const char *WorldName = (const char*)sqlite3_column_text(Stmt, 1);
			const char *Host = (const char*)sqlite3_column_text(Stmt, 2);
			int Port = sqlite3_column_int(Stmt, 3);

			StringCopy(Characters[Count].Name, sizeof(Characters[Count].Name), CharName ? CharName : "");
			StringCopy(Characters[Count].WorldName, sizeof(Characters[Count].WorldName), WorldName ? WorldName : "Tibia");

			// Convert IPv4 Host to uint32 (Big Endian)
			uint32 IP = 0x7F000001; // 127.0.0.1 default
			if(Host && strcmp(Host, "localhost") != 0){
				unsigned int b1, b2, b3, b4;
				if(sscanf(Host, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) == 4){
					IP = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
				}
			}
			Characters[Count].WorldAddress = IP;
			Characters[Count].WorldPort = Port > 0 ? Port : 7172;
			Count++;
		}
		*NumCharacters = Count;
		sqlite3_finalize(Stmt);
	}

	return 0; // Success
}

int DB_LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
		int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
		int *PremiumNewbieBuffer){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;

	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "SELECT Type, RebootTime, Host, Port, MaxPlayers, PremiumPlayerBuffer, MaxNewbies, PremiumNewbieBuffer FROM Worlds LIMIT 1";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) != SQLITE_OK){
		return 1;
	}

	if(sqlite3_step(Stmt) == SQLITE_ROW){
		if(WorldType) *WorldType = sqlite3_column_int(Stmt, 0);
		if(RebootTime) *RebootTime = sqlite3_column_int(Stmt, 1);
		if(IPAddress){
			const char *Host = (const char*)sqlite3_column_text(Stmt, 2);
			uint32 IP = 0x7F000001;
			if(Host && strcmp(Host, "localhost") != 0){
				unsigned int b1, b2, b3, b4;
				if(sscanf(Host, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) == 4){
					IP = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
				}
			}
			*IPAddress = (int)IP;
		}
		if(Port) *Port = sqlite3_column_int(Stmt, 3);
		if(MaxPlayers) *MaxPlayers = sqlite3_column_int(Stmt, 4);
		if(PremiumPlayerBuffer) *PremiumPlayerBuffer = sqlite3_column_int(Stmt, 5);
		if(MaxNewbies) *MaxNewbies = sqlite3_column_int(Stmt, 6);
		if(PremiumNewbieBuffer) *PremiumNewbieBuffer = sqlite3_column_int(Stmt, 7);
		sqlite3_finalize(Stmt);
		return 0;
	}

	sqlite3_finalize(Stmt);
	return 1;
}

static inline void DB_SetRightBit(uint8 *bitset, int index){
	if(bitset && index >= 0){
		bitset[index / 8] |= (uint8)(1 << (index % 8));
	}
}

static int DB_GetRightByName(const char *RightName){
	if(!RightName) return -1;
	if(strcmp(RightName, "PREMIUM_ACCOUNT") == 0) return 0;
	if(strcmp(RightName, "NOTATION") == 0) return 1;
	if(strcmp(RightName, "NAMELOCK") == 0) return 2;
	if(strcmp(RightName, "STATEMENT_REPORT") == 0) return 3;
	if(strcmp(RightName, "BANISHMENT") == 0) return 4;
	if(strcmp(RightName, "FINAL_WARNING") == 0) return 5;
	if(strcmp(RightName, "IP_BANISHMENT") == 0) return 6;
	if(strcmp(RightName, "KICK") == 0) return 7;
	if(strcmp(RightName, "HOME_TELEPORT") == 0) return 8;
	if(strcmp(RightName, "GAMEMASTER_BROADCAST") == 0 || strcmp(RightName, "TALK_ORANGE") == 0 || strcmp(RightName, "TALK_RED") == 0) return 9;
	if(strcmp(RightName, "ANONYMOUS_BROADCAST") == 0) return 10;
	if(strcmp(RightName, "NO_BANISHMENT") == 0) return 11;
	if(strcmp(RightName, "ALLOW_MULTICLIENT") == 0) return 12;
	if(strcmp(RightName, "LOG_COMMUNICATION") == 0) return 13;
	if(strcmp(RightName, "READ_GAMEMASTER_CHANNEL") == 0) return 14;
	if(strcmp(RightName, "READ_TUTOR_CHANNEL") == 0) return 15;
	if(strcmp(RightName, "HIGHLIGHT_HELP_CHANNEL") == 0) return 16;
	if(strcmp(RightName, "SEND_BUGREPORTS") == 0) return 17;
	if(strcmp(RightName, "NAME_INSULTING") == 0) return 18;
	if(strcmp(RightName, "NAME_SENTENCE") == 0) return 19;
	if(strcmp(RightName, "NAME_NONSENSICAL_LETTERS") == 0) return 20;
	if(strcmp(RightName, "NAME_BADLY_FORMATTED") == 0) return 21;
	if(strcmp(RightName, "NAME_NO_PERSON") == 0) return 22;
	if(strcmp(RightName, "NAME_CELEBRITY") == 0) return 23;
	if(strcmp(RightName, "NAME_COUNTRY") == 0) return 24;
	if(strcmp(RightName, "NAME_FAKE_IDENTITY") == 0) return 25;
	if(strcmp(RightName, "NAME_FAKE_POSITION") == 0) return 26;
	if(strcmp(RightName, "STATEMENT_INSULTING") == 0) return 27;
	if(strcmp(RightName, "STATEMENT_SPAMMING") == 0) return 28;
	if(strcmp(RightName, "STATEMENT_ADVERT_OFFTOPIC") == 0) return 29;
	if(strcmp(RightName, "STATEMENT_ADVERT_MONEY") == 0) return 30;
	if(strcmp(RightName, "STATEMENT_NON_ENGLISH") == 0) return 31;
	if(strcmp(RightName, "STATEMENT_CHANNEL_OFFTOPIC") == 0) return 32;
	if(strcmp(RightName, "STATEMENT_VIOLATION_INCITING") == 0) return 33;
	if(strcmp(RightName, "CHEATING_BUG_ABUSE") == 0) return 34;
	if(strcmp(RightName, "CHEATING_GAME_WEAKNESS") == 0) return 35;
	if(strcmp(RightName, "CHEATING_MACRO_USE") == 0) return 36;
	if(strcmp(RightName, "CHEATING_MODIFIED_CLIENT") == 0) return 37;
	if(strcmp(RightName, "CHEATING_HACKING") == 0) return 38;
	if(strcmp(RightName, "CHEATING_MULTI_CLIENT") == 0) return 39;
	if(strcmp(RightName, "CHEATING_ACCOUNT_TRADING") == 0) return 40;
	if(strcmp(RightName, "CHEATING_ACCOUNT_SHARING") == 0) return 41;
	if(strcmp(RightName, "GAMEMASTER_THREATENING") == 0) return 42;
	if(strcmp(RightName, "GAMEMASTER_PRETENDING") == 0) return 43;
	if(strcmp(RightName, "GAMEMASTER_INFLUENCE") == 0) return 44;
	if(strcmp(RightName, "GAMEMASTER_FALSE_REPORTS") == 0) return 45;
	if(strcmp(RightName, "KILLING_EXCESSIVE_UNJUSTIFIED") == 0) return 46;
	if(strcmp(RightName, "DESTRUCTIVE_BEHAVIOUR") == 0) return 47;
	if(strcmp(RightName, "SPOILING_AUCTION") == 0) return 48;
	if(strcmp(RightName, "INVALID_PAYMENT") == 0) return 49;
	if(strcmp(RightName, "TELEPORT_TO_CHARACTER") == 0 || strcmp(RightName, "TELEPORT_TO_CREATURE") == 0) return 50;
	if(strcmp(RightName, "TELEPORT_TO_MARK") == 0) return 51;
	if(strcmp(RightName, "TELEPORT_VERTICAL") == 0) return 52;
	if(strcmp(RightName, "TELEPORT_TO_COORDINATE") == 0) return 53;
	if(strcmp(RightName, "LEVITATE") == 0) return 54;
	if(strcmp(RightName, "SPECIAL_MOVEUSE") == 0) return 55;
	if(strcmp(RightName, "MODIFY_GOSTRENGTH") == 0) return 56;
	if(strcmp(RightName, "SHOW_COORDINATE") == 0) return 57;
	if(strcmp(RightName, "RETRIEVE") == 0) return 58;
	if(strcmp(RightName, "ENTER_HOUSES") == 0) return 59;
	if(strcmp(RightName, "OPEN_NAMEDOORS") == 0 || strcmp(RightName, "OPEN_NAMEDDOORS") == 0) return 60;
	if(strcmp(RightName, "INVULNERABLE") == 0) return 61;
	if(strcmp(RightName, "UNLIMITED_MANA") == 0) return 62;
	if(strcmp(RightName, "KEEP_INVENTORY") == 0) return 63;
	if(strcmp(RightName, "ALL_SPELLS") == 0) return 64;
	if(strcmp(RightName, "UNLIMITED_CAPACITY") == 0) return 65;
	if(strcmp(RightName, "ZERO_CAPACITY") == 0) return 66;
	if(strcmp(RightName, "ATTACK_EVERYWHERE") == 0) return 67;
	if(strcmp(RightName, "NO_ATTACK") == 0) return 68;
	if(strcmp(RightName, "NO_RUNES") == 0) return 69;
	if(strcmp(RightName, "NO_LOGOUT_BLOCK") == 0 || strcmp(RightName, "ANY_LOCATION_LOGOUT") == 0) return 70;
	if(strcmp(RightName, "GAMEMASTER_OUTFIT") == 0 || strcmp(RightName, "COMMUNITY_MANAGER") == 0) return 71;
	if(strcmp(RightName, "ILLUMINATE") == 0) return 72;
	if(strcmp(RightName, "CHANGE_PROFESSION") == 0) return 73;
	if(strcmp(RightName, "IGNORED_BY_MONSTERS") == 0) return 74;
	if(strcmp(RightName, "SHOW_KEYHOLE_NUMBERS") == 0) return 75;
	if(strcmp(RightName, "CREATE_OBJECTS") == 0) return 76;
	if(strcmp(RightName, "CREATE_MONEY") == 0) return 77;
	if(strcmp(RightName, "CREATE_MONSTERS") == 0) return 78;
	if(strcmp(RightName, "CHANGE_SKILLS") == 0) return 79;
	if(strcmp(RightName, "CLEANUP_FIELDS") == 0) return 80;
	if(strcmp(RightName, "NO_STATISTICS") == 0) return 81;
	return -1;
}

int DB_LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
		const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
		bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
		char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
		char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;

	// 1. Validate Account
	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "SELECT Auth, PremiumEnd, Deleted FROM Accounts WHERE AccountID = ?1";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) != SQLITE_OK){
		return 1;
	}
	sqlite3_bind_int(Stmt, 1, (int)AccountID);
	if(sqlite3_step(Stmt) != SQLITE_ROW){
		sqlite3_finalize(Stmt);
		return 1; // Account not found
	}

	const void *AuthBlob = sqlite3_column_blob(Stmt, 0);
	int AuthBlobSize = sqlite3_column_bytes(Stmt, 0);
	int64 PremiumEnd = sqlite3_column_int64(Stmt, 1);
	int Deleted = sqlite3_column_int(Stmt, 2);

	if(Deleted || !VerifyPassword((const uint8*)AuthBlob, AuthBlobSize, Password)){
		sqlite3_finalize(Stmt);
		return 2; // Invalid password
	}
	sqlite3_finalize(Stmt);

	bool isPremium = (PremiumEnd > (int64)time(nullptr));
	if(PremiumAccountActivated){
		*PremiumAccountActivated = isPremium;
	}

	// 2. Fetch Character
	const char *CharSQL = "SELECT CharacterID, Sex, Profession, Residence FROM Characters WHERE AccountID = ?1 AND Name = ?2 AND Deleted = 0";
	if(sqlite3_prepare_v2(g_DB, CharSQL, -1, &Stmt, nullptr) != SQLITE_OK){
		return 3;
	}
	sqlite3_bind_int(Stmt, 1, (int)AccountID);
	sqlite3_bind_text(Stmt, 2, PlayerName, -1, SQLITE_STATIC);

	if(sqlite3_step(Stmt) != SQLITE_ROW){
		sqlite3_finalize(Stmt);
		return 3; // Character not found
	}

	uint32 CID = (uint32)sqlite3_column_int(Stmt, 0);
	if(CharacterID) *CharacterID = CID;
	if(Sex) *Sex = sqlite3_column_int(Stmt, 1);
	sqlite3_finalize(Stmt);

	// 3. Fetch Rights
	if(Rights){
		memset(Rights, 0, 12);
		if(isPremium){
			DB_SetRightBit(Rights, 0); // PREMIUM_ACCOUNT
		}
		const char *RightSQL = "SELECT Name FROM CharacterRights WHERE CharacterID = ?1";
		if(sqlite3_prepare_v2(g_DB, RightSQL, -1, &Stmt, nullptr) == SQLITE_OK){
			sqlite3_bind_int(Stmt, 1, (int)CID);
			while(sqlite3_step(Stmt) == SQLITE_ROW){
				const char *RName = (const char*)sqlite3_column_text(Stmt, 0);
				if(RName){
					int r = DB_GetRightByName(RName);
					if(r >= 0){
						DB_SetRightBit(Rights, r);
					}
				}
			}
			sqlite3_finalize(Stmt);
		}

		// Ensure full GM rights for Gamemasters
		bool isGM = (AccountID == 666666)
			|| (strncmp(PlayerName, "GM ", 3) == 0)
			|| (strncmp(PlayerName, "God ", 4) == 0)
			|| (strncmp(PlayerName, "CM ", 3) == 0)
			|| (strcmp(PlayerName, "Gamemaster") == 0)
			|| (Rights[71 / 8] & (1 << (71 % 8))); // GAMEMASTER_OUTFIT

		if(isGM){
			static const int fullGmRights[] = {
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
				18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
				34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
				50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
				67, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81
			};
			for(int r : fullGmRights){
				DB_SetRightBit(Rights, r);
			}
		}
	}

	if(NumberOfBuddies) *NumberOfBuddies = 0;
	if(Guild) Guild[0] = 0;
	if(Rank) Rank[0] = 0;
	if(Title) Title[0] = 0;

	// Mark online
	const char *UpdSQL = "UPDATE Characters SET IsOnline = 1 WHERE CharacterID = ?1";
	if(sqlite3_prepare_v2(g_DB, UpdSQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, (int)CID);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
	}

	return 0; // Success
}

int DB_LogoutGame(uint32 CharacterID, int Level, const char *Profession,
		const char *Residence, time_t LastLoginTime, int TutorActivities){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;

	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "UPDATE Characters SET Level = ?1, Profession = ?2, Residence = ?3, LastLoginTime = ?4, TutorActivities = ?5, IsOnline = 0 WHERE CharacterID = ?6";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, Level);
		sqlite3_bind_text(Stmt, 2, Profession ? Profession : "", -1, SQLITE_STATIC);
		sqlite3_bind_text(Stmt, 3, Residence ? Residence : "", -1, SQLITE_STATIC);
		sqlite3_bind_int64(Stmt, 4, (int64)LastLoginTime);
		sqlite3_bind_int(Stmt, 5, TutorActivities);
		sqlite3_bind_int(Stmt, 6, (int)CharacterID);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}

int DB_LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
		const char *Remark, bool Unjustified, time_t Time){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;

	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "INSERT INTO CharacterDeaths (CharacterID, Level, OffenderID, Remark, Unjustified, Timestamp) VALUES (?1, ?2, ?3, ?4, ?5, ?6)";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, (int)CharacterID);
		sqlite3_bind_int(Stmt, 2, Level);
		sqlite3_bind_int(Stmt, 3, (int)Offender);
		sqlite3_bind_text(Stmt, 4, Remark ? Remark : "", -1, SQLITE_STATIC);
		sqlite3_bind_int(Stmt, 5, Unjustified ? 1 : 0);
		sqlite3_bind_int64(Stmt, 6, (int64)Time);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}

int DB_InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;
	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "INSERT OR REPLACE INTO HouseOwners (HouseID, OwnerID, PaidUntil) VALUES (?1, ?2, ?3)";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, HouseID);
		sqlite3_bind_int(Stmt, 2, (int)OwnerID);
		sqlite3_bind_int(Stmt, 3, PaidUntil);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}

int DB_UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	return DB_InsertHouseOwner(HouseID, OwnerID, PaidUntil);
}

int DB_DeleteHouseOwner(uint16 HouseID){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;
	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "DELETE FROM HouseOwners WHERE HouseID = ?1";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, HouseID);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}

int DB_GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
		uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr || NumberOfOwners == nullptr) return 1;
	*NumberOfOwners = 0;
	return 0;
}

int DB_ClearIsOnline(int *NumberOfAffectedPlayers){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;
	ExecSQL("UPDATE Characters SET IsOnline = 0;");
	if(NumberOfAffectedPlayers) *NumberOfAffectedPlayers = 0;
	return 0;
}

int DB_CreatePlayerlist(int NumberOfPlayers, const char **Names,
		int *Levels, const char (*Professions)[30], bool *NewRecord){
	if(NewRecord) *NewRecord = false;
	return 0;
}

int DB_LogKilledCreatures(int NumberOfRaces, const char **Names,
		int *KilledPlayers, int *KilledCreatures){
	return 0;
}

int DB_AddBuddy(uint32 AccountID, uint32 Buddy){
	return 0;
}

int DB_RemoveBuddy(uint32 AccountID, uint32 Buddy){
	return 0;
}

int DB_DecrementIsOnline(uint32 CharacterID){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr) return 1;
	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "UPDATE Characters SET IsOnline = 0 WHERE CharacterID = ?1";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, (int)CharacterID);
		sqlite3_step(Stmt);
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}

int DB_LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
		char (*PlayerNames)[30], uint32 *CharacterIDs){
	std::lock_guard<std::mutex> Lock(g_DBMutex);
	if(g_DB == nullptr || NumberOfPlayers == nullptr) return 1;
	*NumberOfPlayers = 0;

	sqlite3_stmt *Stmt = nullptr;
	const char *SQL = "SELECT CharacterID, Name FROM Characters WHERE CharacterID > ?1 ORDER BY CharacterID ASC LIMIT 10000;";
	if(sqlite3_prepare_v2(g_DB, SQL, -1, &Stmt, nullptr) == SQLITE_OK){
		sqlite3_bind_int(Stmt, 1, MinimumCharacterID);
		int Count = 0;
		while(sqlite3_step(Stmt) == SQLITE_ROW && Count < 10000){
			uint32 CID = (uint32)sqlite3_column_int(Stmt, 0);
			const char *Name = (const char*)sqlite3_column_text(Stmt, 1);
			if(CharacterIDs) CharacterIDs[Count] = CID;
			if(PlayerNames && Name){
				strncpy(PlayerNames[Count], Name, 29);
				PlayerNames[Count][29] = 0;
			}
			Count++;
		}
		*NumberOfPlayers = Count;
		sqlite3_finalize(Stmt);
		return 0;
	}
	return 1;
}
