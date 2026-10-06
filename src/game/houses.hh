#ifndef TIBIA_HOUSES_HH_
#define TIBIA_HOUSES_HH_ 1

#include "common.hh"
#include "containers.hh"
#include "map.hh"

#define MAX_HOUSE_GUEST_NAME 60

struct TPlayer;
struct TPlayerData;

struct THelpDepot {
	uint32 CharacterID;
	Object Box;
	int DepotNr;
};

struct THouseArea {
	uint16 ID;
	int SQMPrice;
	int DepotNr;
};

struct THouseGuest {
	char Name[MAX_HOUSE_GUEST_NAME];
};

struct THouse {
	THouse(void);

	// TODO(fusion): Same as `TChannel` in `operate.hh`.
	THouse(const THouse &Other);
	void operator=(const THouse &Other);

	// DATA
	// =================
	uint16 ID;
	char Name[50];
	char Description[500];
	int Size;
	int Rent;
	int DepotNr;
	bool NoAuction;
	bool GuildHouse;
	int ExitX;
	int ExitY;
	int ExitZ;
	int CenterX;
	int CenterY;
	int CenterZ;
	uint32 OwnerID;
	char OwnerName[30];
	int LastTransition;
	int PaidUntil;
	vector<THouseGuest> Subowner;
	int Subowners;
	vector<THouseGuest> Guest;
	int Guests;
	int Help;
};

THouseArea *GetHouseArea(uint16 ID);
int CheckAccessRight(const char *Rule, TPlayer *Player);
THouse *GetHouse(uint16 ID);
bool IsOwner(uint16 HouseID, TPlayer *Player);
bool IsSubowner(uint16 HouseID, TPlayer *Player, int TimeStamp);
bool IsGuest(uint16 HouseID, TPlayer *Player, int TimeStamp);
bool IsInvited(uint16 HouseID, TPlayer *Player, int TimeStamp);
const char *GetHouseName(uint16 HouseID);
const char *GetHouseOwner(uint16 HouseID);
void ShowSubownerList(uint16 HouseID, TPlayer *Player, char *Buffer);
void ShowGuestList(uint16 HouseID, TPlayer *Player, char *Buffer);
void ChangeSubowners(uint16 HouseID, TPlayer *Player, const char *Buffer);
void ChangeGuests(uint16 HouseID, TPlayer *Player, const char *Buffer);
void GetExitPosition(uint16 HouseID, int *x, int *y, int *z);
void KickGuest(uint16 HouseID, TPlayer *Guest);
void KickGuest(uint16 HouseID, TPlayer *Host, TPlayer *Guest);
void KickGuests(uint16 HouseID);
bool MayOpenDoor(Object Door, TPlayer *Player);
void ShowNameDoor(Object Door, TPlayer *Player, char *Buffer);
void ChangeNameDoor(Object Door, TPlayer *Player, const char *Buffer);
void CleanField(int x, int y, int z, Object Depot);
void CleanHouse(THouse *House, TPlayerData *PlayerData);
void ClearHouse(THouse *House);
bool FinishAuctions(void);
bool TransferHouses(void);
bool EvictFreeAccounts(void);
bool EvictDeletedCharacters(void);
bool EvictExGuildLeaders(void);
void CollectRent(void);
void ProcessRent(void);
bool StartAuctions(void);
bool UpdateHouseOwners(void);
void PrepareHouseCleanup(void);
void FinishHouseCleanup(void);
void CleanHouseField(int x, int y, int z);
void LoadHouseAreas(void);
void LoadHouses(void);
void LoadOwners(void);
void SaveOwners(void);
void ProcessHouses(void);
void InitHouses(void);
void ExitHouses(void);

#endif //TIBIA_HOUSES_HH_
