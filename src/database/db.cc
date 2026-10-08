#include "db.hh"
#include "db_driver.hh"
#include "../game/config.hh"
#include <memory>
#include <iostream>

// Driver factories
std::unique_ptr<IDatabaseDriver> CreateSQLiteDriver(const char *dbPath);
std::unique_ptr<IDatabaseDriver> CreateMySQLDriver(const char *host, int port, const char *user, const char *pass, const char *db, bool autoReconnect);

static std::unique_ptr<IDatabaseDriver> g_DBDriver = nullptr;

bool DatabaseInit(const char *DatabaseFile){
	if(g_DBDriver) return true;

	// Check configured database engine
	bool useMySQL = (DB_Type[0] != 0 && (_stricmp(DB_Type, "mysql") == 0 || _stricmp(DB_Type, "mariadb") == 0));

	if(useMySQL){
		std::cout << "\n:: Initializing MySQL/MariaDB driver (" << MySQL_Host << ":" << MySQL_Port << "/" << MySQL_Database << ")... " << std::flush;
		g_DBDriver = CreateMySQLDriver(MySQL_Host, MySQL_Port, MySQL_User, MySQL_Password, MySQL_Database, MySQL_Reconnect);
	} else {
		const char *filePath = (DatabaseFile && DatabaseFile[0]) ? DatabaseFile : (DB_File[0] ? DB_File : "tibia.db");
		std::cout << "\n:: Initializing SQLite 3 driver (" << filePath << ")... " << std::flush;
		g_DBDriver = CreateSQLiteDriver(filePath);
	}

	if(!g_DBDriver || !g_DBDriver->Init()){
		g_DBDriver.reset();
		return false;
	}

	return true;
}

void DatabaseExit(void){
	if(g_DBDriver){
		g_DBDriver->Exit();
		g_DBDriver.reset();
	}
}

bool DatabaseExecuteSchema(void){
	if(!g_DBDriver) return false;
	return g_DBDriver->ExecuteSchema();
}

const char *DatabaseGetDriverName(void){
	if(!g_DBDriver) return "None";
	return g_DBDriver->GetName();
}

int DB_LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
		int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LoginAccount(AccountID, Password, IPAddress, MaxCharacters, NumCharacters, Characters, PremiumDays);
}

int DB_LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
		int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
		int *PremiumNewbieBuffer){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LoadWorldConfig(WorldType, RebootTime, IPAddress, Port, MaxPlayers, PremiumPlayerBuffer, MaxNewbies, PremiumNewbieBuffer);
}

int DB_LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
		const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
		bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
		char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
		char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LoginGame(AccountID, PlayerName, Password, IPAddress, PrivateWorld, PremiumAccountRequired,
			GamemasterRequired, CharacterID, Sex, Guild, Rank, Title, NumberOfBuddies, BuddyIDs,
			BuddyNames, Rights, PremiumAccountActivated);
}

int DB_LogoutGame(uint32 CharacterID, int Level, const char *Profession,
		const char *Residence, time_t LastLoginTime, int TutorActivities){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LogoutGame(CharacterID, Level, Profession, Residence, LastLoginTime, TutorActivities);
}

int DB_LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
		const char *Remark, bool Unjustified, time_t Time){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LogCharacterDeath(CharacterID, Level, Offender, Remark, Unjustified, Time);
}

int DB_InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	if(!g_DBDriver) return 1;
	return g_DBDriver->InsertHouseOwner(HouseID, OwnerID, PaidUntil);
}

int DB_UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	if(!g_DBDriver) return 1;
	return g_DBDriver->UpdateHouseOwner(HouseID, OwnerID, PaidUntil);
}

int DB_DeleteHouseOwner(uint16 HouseID){
	if(!g_DBDriver) return 1;
	return g_DBDriver->DeleteHouseOwner(HouseID);
}

int DB_GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
		uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils){
	if(!g_DBDriver) return 1;
	return g_DBDriver->GetHouseOwners(NumberOfOwners, HouseIDs, OwnerIDs, OwnerNames, PaidUntils);
}

int DB_ClearIsOnline(int *NumberOfAffectedPlayers){
	if(!g_DBDriver) return 1;
	return g_DBDriver->ClearIsOnline(NumberOfAffectedPlayers);
}

int DB_CreatePlayerlist(int NumberOfPlayers, const char **Names,
		int *Levels, const char (*Professions)[30], bool *NewRecord){
	if(!g_DBDriver) return 1;
	return g_DBDriver->CreatePlayerlist(NumberOfPlayers, Names, Levels, Professions, NewRecord);
}

int DB_LogKilledCreatures(int NumberOfRaces, const char **Names,
		int *KilledPlayers, int *KilledCreatures){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LogKilledCreatures(NumberOfRaces, Names, KilledPlayers, KilledCreatures);
}

int DB_AddBuddy(uint32 AccountID, uint32 Buddy){
	if(!g_DBDriver) return 1;
	return g_DBDriver->AddBuddy(AccountID, Buddy);
}

int DB_RemoveBuddy(uint32 AccountID, uint32 Buddy){
	if(!g_DBDriver) return 1;
	return g_DBDriver->RemoveBuddy(AccountID, Buddy);
}

int DB_DecrementIsOnline(uint32 CharacterID){
	if(!g_DBDriver) return 1;
	return g_DBDriver->DecrementIsOnline(CharacterID);
}

int DB_LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
		char (*PlayerNames)[30], uint32 *CharacterIDs){
	if(!g_DBDriver) return 1;
	return g_DBDriver->LoadPlayers(MinimumCharacterID, NumberOfPlayers, PlayerNames, CharacterIDs);
}
