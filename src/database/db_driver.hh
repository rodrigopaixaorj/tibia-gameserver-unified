#ifndef TIBIA_DB_DRIVER_HH_
#define TIBIA_DB_DRIVER_HH_ 1

#include "db.hh"

// Abstract Database Driver Interface
class IDatabaseDriver {
public:
	virtual ~IDatabaseDriver() = default;

	// Driver lifecycle
	virtual bool Init() = 0;
	virtual void Exit() = 0;
	virtual bool ExecuteSchema() = 0;
	virtual const char *GetName() const = 0;

	// Login Service Queries
	virtual int LoginAccount(uint32 AccountID, const char *Password, const char *IPAddress,
			int MaxCharacters, int *NumCharacters, TCharacterLoginData *Characters, int *PremiumDays) = 0;

	// Game Service Queries
	virtual int LoadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
			int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
			int *PremiumNewbieBuffer) = 0;

	virtual int LoginGame(uint32 AccountID, char *PlayerName, const char *Password,
			const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
			bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
			char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
			char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated) = 0;

	virtual int LogoutGame(uint32 CharacterID, int Level, const char *Profession,
			const char *Residence, time_t LastLoginTime, int TutorActivities) = 0;

	virtual int LogCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
			const char *Remark, bool Unjustified, time_t Time) = 0;

	virtual int InsertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) = 0;
	virtual int UpdateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil) = 0;
	virtual int DeleteHouseOwner(uint16 HouseID) = 0;
	virtual int GetHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
			uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils) = 0;

	virtual int ClearIsOnline(int *NumberOfAffectedPlayers) = 0;
	virtual int CreatePlayerlist(int NumberOfPlayers, const char **Names,
			int *Levels, const char (*Professions)[30], bool *NewRecord) = 0;
	virtual int LogKilledCreatures(int NumberOfRaces, const char **Names,
			int *KilledPlayers, int *KilledCreatures) = 0;

	virtual int AddBuddy(uint32 AccountID, uint32 Buddy) = 0;
	virtual int RemoveBuddy(uint32 AccountID, uint32 Buddy) = 0;
	virtual int DecrementIsOnline(uint32 CharacterID) = 0;
	virtual int LoadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
			char (*PlayerNames)[30], uint32 *CharacterIDs) = 0;
};

#endif // TIBIA_DB_DRIVER_HH_
