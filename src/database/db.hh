#ifndef TIBIA_DB_HH_
#define TIBIA_DB_HH_ 1

#include "../compat/compat.hh"
#include <string>
#include <vector>

struct TCharacterLoginData {
	char   Name[30];
	char   WorldName[30];
	uint32 WorldAddress;
	int    WorldPort;
};

// Database Management Interface
bool DatabaseInit(const char *DatabaseFile);
void DatabaseExit(void);
bool DatabaseExecuteSchema(void);

// Login Service Queries
int DB_LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
		int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays);

// Game Service Queries
int DB_LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
		int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
		int *PremiumNewbieBuffer);

int DB_LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
		const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
		bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
		char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
		char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated);

int DB_LogoutGame(uint32 CharacterID, int Level, const char *Profession,
		const char *Residence, time_t LastLoginTime, int TutorActivities);

int DB_LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
		const char *Remark, bool Unjustified, time_t Time);

int DB_InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil);
int DB_UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil);
int DB_DeleteHouseOwner(uint16 HouseID);
int DB_GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
		uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils);

int DB_ClearIsOnline(int *NumberOfAffectedPlayers);
int DB_CreatePlayerlist(int NumberOfPlayers, const char **Names,
		int *Levels, const char (*Professions)[30], bool *NewRecord);
int DB_LogKilledCreatures(int NumberOfRaces, const char **Names,
		int *KilledPlayers, int *KilledCreatures);

int DB_AddBuddy(uint32 AccountID, uint32 Buddy);
int DB_RemoveBuddy(uint32 AccountID, uint32 Buddy);
int DB_DecrementIsOnline(uint32 CharacterID);
int DB_LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
		char (*PlayerNames)[30], uint32 *CharacterIDs);

#endif // TIBIA_DB_HH_
