#ifndef TIBIA_CONNECTIONS_HH_
#define TIBIA_CONNECTIONS_HH_ 1

#include "common.hh"
#include "crypto.hh"
#include "enums.hh"
#include "map.hh"

struct TConnection;
struct TPlayer;

enum ClientCommand: int {
	CL_CMD_LOGIN_REQUEST			= 10,
	CL_CMD_LOGIN					= 11,
	CL_CMD_LOGOUT					= 20,
	CL_CMD_PING						= 30,
	CL_CMD_GO_PATH					= 100,
	CL_CMD_GO_NORTH					= 101,
	CL_CMD_GO_EAST					= 102,
	CL_CMD_GO_SOUTH					= 103,
	CL_CMD_GO_WEST					= 104,
	CL_CMD_GO_STOP					= 105,
	CL_CMD_GO_NORTHEAST				= 106,
	CL_CMD_GO_SOUTHEAST				= 107,
	CL_CMD_GO_SOUTHWEST				= 108,
	CL_CMD_GO_NORTHWEST				= 109,
	CL_CMD_ROTATE_NORTH				= 111,
	CL_CMD_ROTATE_EAST				= 112,
	CL_CMD_ROTATE_SOUTH				= 113,
	CL_CMD_ROTATE_WEST				= 114,
	CL_CMD_MOVE_OBJECT				= 120,
	CL_CMD_TRADE_OBJECT				= 125,
	CL_CMD_INSPECT_TRADE			= 126,
	CL_CMD_ACCEPT_TRADE				= 127,
	CL_CMD_REJECT_TRADE				= 128,
	CL_CMD_USE_OBJECT				= 130,
	CL_CMD_USE_TWO_OBJECTS			= 131,
	CL_CMD_USE_ON_CREATURE			= 132,
	CL_CMD_TURN_OBJECT				= 133,
	CL_CMD_CLOSE_CONTAINER			= 135,
	CL_CMD_UP_CONTAINER				= 136,
	CL_CMD_EDIT_TEXT				= 137,
	CL_CMD_EDIT_LIST				= 138,
	CL_CMD_LOOK_AT_POINT			= 140,
	CL_CMD_TALK						= 150,
	CL_CMD_GET_CHANNELS				= 151,
	CL_CMD_JOIN_CHANNEL				= 152,
	CL_CMD_LEAVE_CHANNEL			= 153,
	CL_CMD_PRIVATE_CHANNEL			= 154,
	CL_CMD_PROCESS_REQUEST			= 155,
	CL_CMD_REMOVE_REQUEST			= 156,
	CL_CMD_CANCEL_REQUEST			= 157,
	CL_CMD_SET_TACTICS				= 160,
	CL_CMD_ATTACK					= 161,
	CL_CMD_FOLLOW					= 162,
	CL_CMD_INVITE_TO_PARTY			= 163,
	CL_CMD_JOIN_PARTY				= 164,
	CL_CMD_REVOKE_INVITATION		= 165,
	CL_CMD_PASS_LEADERSHIP			= 166,
	CL_CMD_LEAVE_PARTY				= 167,
	CL_CMD_OPEN_CHANNEL				= 170,
	CL_CMD_INVITE_TO_CHANNEL		= 171,
	CL_CMD_EXCLUDE_FROM_CHANNEL		= 172,
	CL_CMD_CANCEL					= 190,
	CL_CMD_REFRESH_FIELD			= 201,
	CL_CMD_REFRESH_CONTAINER		= 202,
	CL_CMD_GET_OUTFIT				= 210,
	CL_CMD_SET_OUTFIT				= 211,
	CL_CMD_ADD_BUDDY				= 220,
	CL_CMD_REMOVE_BUDDY				= 221,
	CL_CMD_BUG_REPORT				= 230,
	CL_CMD_RULE_VIOLATION			= 231,
	CL_CMD_ERROR_FILE_ENTRY			= 232,
};

enum ServerCommand: int {
	SV_CMD_INIT_GAME				= 10,
	SV_CMD_RIGHTS					= 11,
	SV_CMD_LOGIN_ERROR				= 20,
	SV_CMD_LOGIN_PREMIUM			= 21,
	SV_CMD_LOGIN_WAITINGLIST		= 22,
	SV_CMD_PING						= 30,
	SV_CMD_FULLSCREEN				= 100,
	SV_CMD_ROW_NORTH				= 101,
	SV_CMD_ROW_EAST					= 102,
	SV_CMD_ROW_SOUTH				= 103,
	SV_CMD_ROW_WEST					= 104,
	SV_CMD_FIELD_DATA				= 105,
	SV_CMD_ADD_FIELD				= 106,
	SV_CMD_CHANGE_FIELD				= 107,
	SV_CMD_DELETE_FIELD				= 108,
	SV_CMD_MOVE_CREATURE			= 109,
	SV_CMD_CONTAINER				= 110,
	SV_CMD_CLOSE_CONTAINER			= 111,
	SV_CMD_CREATE_IN_CONTAINER		= 112,
	SV_CMD_CHANGE_IN_CONTAINER		= 113,
	SV_CMD_DELETE_IN_CONTAINER		= 114,
	SV_CMD_SET_INVENTORY			= 120,
	SV_CMD_DELETE_INVENTORY			= 121,
	SV_CMD_TRADE_OFFER_OWN			= 125,
	SV_CMD_TRADE_OFFER_PARTNER		= 126,
	SV_CMD_CLOSE_TRADE				= 127,
	SV_CMD_AMBIENTE					= 130,
	SV_CMD_GRAPHICAL_EFFECT			= 131,
	SV_CMD_TEXTUAL_EFFECT			= 132,
	SV_CMD_MISSILE_EFFECT			= 133,
	SV_CMD_MARK_CREATURE			= 134,
	SV_CMD_CREATURE_HEALTH			= 140,
	SV_CMD_CREATURE_LIGHT			= 141,
	SV_CMD_CREATURE_OUTFIT			= 142,
	SV_CMD_CREATURE_SPEED			= 143,
	SV_CMD_CREATURE_SKULL			= 144,
	SV_CMD_CREATURE_PARTY			= 145,
	SV_CMD_EDIT_TEXT				= 150,
	SV_CMD_EDIT_LIST				= 151,
	SV_CMD_PLAYER_DATA				= 160,
	SV_CMD_PLAYER_SKILLS			= 161,
	SV_CMD_PLAYER_STATE				= 162,
	SV_CMD_CLEAR_TARGET				= 163,
	SV_CMD_TALK						= 170,
	SV_CMD_CHANNELS					= 171,
	SV_CMD_OPEN_CHANNEL				= 172,
	SV_CMD_PRIVATE_CHANNEL			= 173,
	SV_CMD_OPEN_REQUEST_QUEUE		= 174,
	SV_CMD_DELETE_REQUEST			= 175,
	SV_CMD_FINISH_REQUEST			= 176,
	SV_CMD_CLOSE_REQUEST			= 177,
	SV_CMD_OPEN_OWN_CHANNEL			= 178,
	SV_CMD_CLOSE_CHANNEL			= 179,
	SV_CMD_MESSAGE					= 180,
	SV_CMD_SNAPBACK					= 181,
	SV_CMD_FLOOR_UP					= 190,
	SV_CMD_FLOOR_DOWN				= 191,
	SV_CMD_OUTFIT					= 200,
	SV_CMD_BUDDY_DATA				= 210,
	SV_CMD_BUDDY_ONLINE				= 211,
	SV_CMD_BUDDY_OFFLINE			= 212,
};

// TODO(fusion): The maximum number of connections should probably be kept in
// sync with the maximum number of communication threads, or maybe it is the
// same constant.
#define MAX_CONNECTIONS 1100

struct TKnownCreature {
	KNOWNCREATURESTATE State;
	uint32 CreatureID;
	TKnownCreature *Next;
	TConnection *Connection;
};

struct TConnection {
	TConnection(void);
	void Process(void);
	void ResetTimer(int Command);
	void EmergencyPing(void);
	pid_t GetThreadID(void);
	bool SetLoginTimer(int Timeout);
	void StopLoginTimer(void);
	int GetSocket(void);
	const char *GetIPAddress(void);
	void Free(void);
	void Assign(void);
	void Connect(int Socket);
	void Login(void);
	bool JoinGame(TReadBuffer *Buffer);
	void EnterGame(void);
	void Die(void);
	void Logout(int Delay, bool StopFight);
	void Close(bool Delay);
	void Disconnect(void);
	TPlayer *GetPlayer(void);
	const char *GetName(void);
	void GetPosition(int *x, int *y, int *z);
	bool IsVisible(int x, int y, int z);
	KNOWNCREATURESTATE KnownCreature(uint32 ID, bool UpdateFollows);
	uint32 NewKnownCreature(uint32 NewID);
	void ClearKnownCreatureTable(bool Unchain);
	void UnchainKnownCreature(uint32 ID);

	bool InGame(void) const {
		return this->State == CONNECTION_GAME
			|| this->State == CONNECTION_DEAD;
	}

	bool Live(void) const {
		return this->State == CONNECTION_LOGIN
			|| this->State == CONNECTION_GAME
			|| this->State == CONNECTION_DEAD
			|| this->State == CONNECTION_LOGOUT;
	}

	// DATA
	// =================
	uint8 InData[2048];
	int InDataSize;
	bool SigIOPending;
	bool WaitingForACK;
	uint8 OutData[16384];
	int NextToSend;
	int NextToCommit;
	int NextToWrite;
	bool Overflow;
	bool WillingToSend;
	TConnection *NextSendingConnection;
	uint32 RandomSeed;
	CONNECTIONSTATE State;
	pid_t ThreadID;
	timer_t LoginTimer;
	int Socket;
	char IPAddress[16];
	TXTEASymmetricKey SymmetricKey;
	bool ConnectionIsOk;
	bool ClosingIsDelayed;
	uint32 TimeStamp;
	uint32 TimeStampAction;
	int TerminalType;
	int TerminalVersion;
	int TerminalOffsetX;
	int TerminalOffsetY;
	int TerminalWidth;
	int TerminalHeight;
	uint32 CharacterID;
	char Name[31];
	TKnownCreature KnownCreatureTable[150];
};

// connections.cc
TConnection *AssignFreeConnection(void);
TConnection *GetFirstConnection(void);
TConnection *GetNextConnection(void);
void ProcessConnections(void);
void InitConnections(void);
void ExitConnections(void);

// sending.cc
void SendAll(void);
bool BeginSendData(TConnection *Connection);
void FinishSendData(TConnection *Connection);
void SkipFlush(TConnection *Connection);
void SendMapObject(TConnection *Connection, Object Obj);
void SendMapPoint(TConnection *Connection, int x, int y, int z);
void SendResult(TConnection *Connection, RESULT r);
void SendRefresh(TConnection *Connection);
void SendInitGame(TConnection *Connection, uint32 CreatureID);
void SendRights(TConnection *Connection);
void SendPing(TConnection *Connection);
void SendFullScreen(TConnection *Connection);
void SendRow(TConnection *Connection, int Direction);
void SendFloors(TConnection *Connection, bool Up);
void SendFieldData(TConnection *Connection, int x, int y, int z);
void SendAddField(TConnection *Connection, int x, int y, int z, Object Obj);
void SendChangeField(TConnection *Connection, int x, int y, int z, Object Obj);
void SendDeleteField(TConnection *Connection, int x, int y, int z, Object Obj);
void SendMoveCreature(TConnection *Connection,
		uint32 CreatureID, int DestX, int DestY, int DestZ);
void SendContainer(TConnection *Connection, int ContainerNr);
void SendCloseContainer(TConnection *Connection, int ContainerNr);
void SendCreateInContainer(TConnection *Connection, int ContainerNr, Object Obj);
void SendChangeInContainer(TConnection *Connection, int ContainerNr, Object Obj);
void SendDeleteInContainer(TConnection *Connection, int ContainerNr, Object Obj);
void SendBodyInventory(TConnection *Connection, uint32 CreatureID);
void SendSetInventory(TConnection *Connection, int Position, Object Obj);
void SendDeleteInventory(TConnection *Connection, int Position);
void SendTradeOffer(TConnection *Connection, const char *Name, bool OwnOffer, Object Obj);
void SendCloseTrade(TConnection *Connection);
void SendAmbiente(TConnection *Connection);
void SendGraphicalEffect(TConnection *Connection, int x, int y, int z, int Type);
void SendTextualEffect(TConnection *Connection, int x, int y, int z, int Color, const char *Text);
void SendMissileEffect(TConnection *Connection, int OrigX, int OrigY, int OrigZ,
		int DestX, int DestY, int DestZ, int Type);
void SendMarkCreature(TConnection *Connection, uint32 CreatureID, int Color);
void SendCreatureHealth(TConnection *Connection, uint32 CreatureID);
void SendCreatureLight(TConnection *Connection, uint32 CreatureID);
void SendCreatureOutfit(TConnection *Connection, uint32 CreatureID);
void SendCreatureSpeed(TConnection *Connection, uint32 CreatureID);
void SendCreatureSkull(TConnection *Connection, uint32 CreatureID);
void SendCreatureParty(TConnection *Connection, uint32 CreatureID);
void SendEditText(TConnection *Connection, Object Obj);
void SendEditList(TConnection *Connection, uint8 Type, uint32 ID, const char *Text);
void SendPlayerData(TConnection *Connection);
void SendPlayerSkills(TConnection *Connection);
void SendPlayerState(TConnection *Connection, uint8 State);
void SendClearTarget(TConnection *Connection);
void SendTalk(TConnection *Connection, uint32 StatementID,
		const char *Sender, int Mode, const char *Text, int Data);
void SendTalk(TConnection *Connection, uint32 StatementID,
		const char *Sender, int Mode, int Channel, const char *Text);
void SendTalk(TConnection *Connection, uint32 StatementID,
		const char *Sender, int Mode, int x, int y, int z, const char *Text);
void SendChannels(TConnection *Connection);
void SendOpenChannel(TConnection *Connection, int Channel);
void SendPrivateChannel(TConnection *Connection, const char *Name);
void SendOpenRequestQueue(TConnection *Connection);
void SendDeleteRequest(TConnection *Connection, const char *Name);
void SendFinishRequest(TConnection *Connection, const char *Name);
void SendCloseRequest(TConnection *Connection);
void SendOpenOwnChannel(TConnection *Connection, int Channel);
void SendCloseChannel(TConnection *Connection, int Channel);
void SendMessage(TConnection *Connection, int Mode, const char *Text, ...) ATTR_PRINTF(3, 4);
void SendSnapback(TConnection *Connection);
void SendOutfit(TConnection *Connection);
void SendBuddyData(TConnection *Connection, uint32 CharacterID, const char *Name, bool Online);
void SendBuddyStatus(TConnection *Connection, uint32 CharacterID, bool Online);
void BroadcastMessage(int Mode, const char *Text, ...) ATTR_PRINTF(2, 3);
void CreateGamemasterRequest(const char *Name, const char *Text);
void DeleteGamemasterRequest(const char *Name);
void InitSending(void);
void ExitSending(void);

// receiving.cc
void ReceiveData(TConnection *Connection);
void ReceiveData(void);
void InitReceiving(void);
void ExitReceiving(void);

#endif // TIBIA_CONNECTIONS_HH_
