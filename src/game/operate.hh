#ifndef TIBIA_OPERATE_HH_
#define TIBIA_OPERATE_HH_ 1

#include "common.hh"
#include "containers.hh"
#include "cr.hh"
#include "map.hh"

enum : int {
	CREATURE_HEALTH_CHANGED		= 1,
	CREATURE_LIGHT_CHANGED		= 2,
	CREATURE_OUTFIT_CHANGED		= 3,
	CREATURE_SPEED_CHANGED		= 4,
	CREATURE_SKULL_CHANGED		= 5,
	CREATURE_PARTY_CHANGED		= 6,
};

enum : int {
	OBJECT_DELETED				= 0,
	OBJECT_CREATED				= 1,
	OBJECT_CHANGED				= 2,
	OBJECT_MOVED				= 3,
};

enum : int {
	CHANNEL_GUILD				= 0,
	CHANNEL_GAMEMASTER			= 1,
	CHANNEL_TUTOR				= 2,
	CHANNEL_RULEVIOLATIONS		= 3,
	CHANNEL_GAMECHAT			= 4,
	CHANNEL_TRADE				= 5,
	CHANNEL_RLCHAT				= 6,
	CHANNEL_HELP				= 7,

	PUBLIC_CHANNELS				= 8,
	FIRST_PRIVATE_CHANNEL		= PUBLIC_CHANNELS,
	MAX_CHANNELS				= 0xFFFF,
};

struct TChannel {
	TChannel(void);

	// TODO(fusion): `TChannel` will primarily live inside the `Channel` vector,
	// which needs to resize its internal array as needed. To resize, it needs
	// to copy/swap elements, which would be impossible since `TChannel` itself
	// owns a few vectors which were made NON-COPYABLE to prevent memory bugs.
	//	To fix this problem and allow `TChannel` to be copyable, we need to
	// implement the copy constructor and assignment manually. They're annoying
	// and expensive but should allow everything to compile again.
	//	This problem arises from the fact that our unconventional `vector` type
	// has a weird interface but still manages its underlying memory. If we are
	// to resolve this completely, we'd need to re-implement `vector` properly
	// and preferably with move semantics (if we want to follow the C++ route,
	// which we may not).
	TChannel(const TChannel &Other);
	void operator=(const TChannel &Other);

	// DATA
	// =================
	uint32 Moderator;
	char ModeratorName[30];
	vector<uint32> Subscriber;
	int Subscribers;
	vector<uint32> InvitedPlayer;
	int InvitedPlayers;
};

struct TParty {
	TParty(void);

	// TODO(fusion): Same as `TChannel`.
	TParty(const TParty &Other);
	void operator=(const TParty &Other);

	// DATA
	// =================
	uint32 Leader;
	vector<uint32> Member;
	int Members;
	vector<uint32> InvitedPlayer;
	int InvitedPlayers;
};

struct TStatement {
	uint32 StatementID;
	int TimeStamp;
	uint32 CharacterID;
	int Mode;
	int Channel;
	uint32 Text;
	bool Reported;
};

struct TListener {
	uint32 StatementID;
	uint32 CharacterID;
};

struct TReportedStatement {
	uint32 StatementID;
	int TimeStamp;
	uint32 CharacterID;
	int Mode;
	int Channel;
	char Text[256];
};

void AnnounceMovingCreature(uint32 CreatureID, Object Con);
void AnnounceChangedCreature(uint32 CreatureID, int Type);
void AnnounceChangedField(Object Obj, int Type);
void AnnounceChangedContainer(Object Obj, int Type);
void AnnounceChangedInventory(Object Obj, int Type);
void AnnounceChangedObject(Object Obj, int Type);
void AnnounceGraphicalEffect(int x, int y, int z, int Type);
void AnnounceTextualEffect(int x, int y, int z, int Color, const char *Text);
void AnnounceMissile(int OrigX, int OrigY, int OrigZ,
		int DestX, int DestY, int DestZ, int Type);
void CheckTopMoveObject(uint32 CreatureID, Object Obj, Object Ignore);
void CheckTopUseObject(uint32 CreatureID, Object Obj);
void CheckTopMultiuseObject(uint32 CreatureID, Object Obj);
void CheckMoveObject(uint32 CreatureID, Object Obj, bool Take);
void CheckMapDestination(uint32 CreatureID, Object Obj, Object MapCon);
void CheckMapPlace(uint32 CreatureID, ObjectType Type, Object MapCon);
void CheckContainerDestination(Object Obj, Object Con);
void CheckContainerPlace(ObjectType Type, Object Con, Object OldObj);
void CheckDepotSpace(uint32 CreatureID, Object Source, Object Destination, int Count);
void CheckInventoryDestination(Object Obj, Object Con, bool Split);
void CheckInventoryPlace(ObjectType Type, Object Con, Object OldObj);
void CheckWeight(uint32 CreatureID, Object Obj, int Count);
void CheckWeight(uint32 CreatureID, ObjectType Type, uint32 Value, int OldWeight);
void NotifyCreature(uint32 CreatureID, Object Obj, bool Inventory);
void NotifyCreature(uint32 CreatureID, ObjectType Type, bool Inventory);
void NotifyAllCreatures(Object Obj, int Type, Object OldCon);
void NotifyTrades(Object Obj);
void NotifyDepot(uint32 CreatureID, Object Obj, int Count);
void CloseContainer(Object Con, bool Force);
Object Create(Object Con, ObjectType Type, uint32 Value);
Object Copy(Object Con, Object Source);
void Move(uint32 CreatureID, Object Obj, Object Con, int Count, bool NoMerge, Object Ignore);
void Merge(uint32 CreatureID, Object Obj, Object Dest, int Count, Object Ignore);
void Change(Object Obj, ObjectType NewType, uint32 Value);
void Change(Object Obj, INSTANCEATTRIBUTE Attribute, uint32 Value);
void Delete(Object Obj, int Count);
void Empty(Object Con, int Remainder);
void GraphicalEffect(int x, int y, int z, int Type);
void GraphicalEffect(Object Obj, int Type);
void TextualEffect(Object Obj, int Color, const char *Format, ...) ATTR_PRINTF(3, 4);
void Missile(Object Start, Object Dest, int Type);
void Look(uint32 CreatureID, Object Obj);
void Talk(uint32 CreatureID, int Mode, const char *Addressee, const char *Text, bool CheckSpamming);
void Use(uint32 CreatureID, Object Obj1, Object Obj2, uint8 Info);
void Turn(uint32 CreatureID, Object Obj);
void CreatePool(Object Con, ObjectType Type, uint32 Value);
void EditText(uint32 CreatureID, Object Obj, const char *Text);
Object CreateAtCreature(uint32 CreatureID, ObjectType Type, uint32 Value);
void DeleteAtCreature(uint32 CreatureID, ObjectType Type, int Amount, uint32 Value);

void ProcessCronSystem(void);
bool SectorRefreshable(int SectorX, int SectorY, int SectorZ);
void RefreshSector(int SectorX, int SectorY, int SectorZ, const uint8 *Data, int Count);
void RefreshMap(void);
void RefreshCylinders(void);
void ApplyPatch(int SectorX, int SectorY, int SectorZ,
		bool FullSector, TReadScriptFile *Script, bool SaveHouses);
void ApplyPatches(void);

uint32 LogCommunication(uint32 CreatureID, int Mode, int Channel, const char *Text);
uint32 LogListener(uint32 StatementID, TPlayer *Player);
void ProcessCommunicationControl(void);
int GetCommunicationContext(uint32 CharacterID, uint32 StatementID,
		int *NumberOfStatements, vector<TReportedStatement> **ReportedStatements);

int GetNumberOfChannels(void);
bool ChannelActive(int ChannelID);
bool ChannelAvailable(int ChannelID, uint32 CharacterID);
const char *GetChannelName(int ChannelID, uint32 CharacterID);
bool ChannelSubscribed(int ChannelID, uint32 CharacterID);
uint32 GetFirstSubscriber(int ChannelID);
uint32 GetNextSubscriber(void);
bool MayOpenChannel(uint32 CharacterID);
void OpenChannel(uint32 CharacterID);
void CloseChannel(int ChannelID);
void InviteToChannel(uint32 CharacterID, const char *Name);
void ExcludeFromChannel(uint32 CharacterID, const char *Name);
bool JoinChannel(int ChannelID, uint32 CharacterID);
void LeaveChannel(int ChannelID, uint32 CharacterID, bool Close);
void LeaveAllChannels(uint32 CharacterID);

TParty *GetParty(uint32 LeaderID);
bool IsInvitedToParty(uint32 GuestID, uint32 HostID);
void DisbandParty(uint32 LeaderID);
void InviteToParty(uint32 HostID, uint32 GuestID);
void RevokeInvitation(uint32 HostID, uint32 GuestID);
void JoinParty(uint32 GuestID, uint32 HostID);
void PassLeadership(uint32 OldLeaderID, uint32 NewLeaderID);
void LeaveParty(uint32 MemberID, bool Forced);

#endif //TIBIA_OPERATE_HH_
