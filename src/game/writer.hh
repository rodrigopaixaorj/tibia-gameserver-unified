#ifndef TIBIA_WRITER_HH_
#define TIBIA_WRITER_HH_ 1

#include "common.hh"
#include "containers.hh"
#include "operate.hh"

enum TWriterThreadOrderType: int {
	WRITER_ORDER_TERMINATE			= 0,
	WRITER_ORDER_LOGOUT				= 1,
	WRITER_ORDER_PLAYERLIST			= 2,
	WRITER_ORDER_KILLSTATISTICS		= 3,
	WRITER_ORDER_PUNISHMENT			= 4,
	WRITER_ORDER_CHARACTERDEATH		= 5,
	WRITER_ORDER_ADDBUDDY			= 6,
	WRITER_ORDER_REMOVEBUDDY		= 7,
	WRITER_ORDER_DECREMENTISONLINE	= 8,
	WRITER_ORDER_SAVEPLAYERDATA		= 9,
};

enum TWriterThreadReplyType: int {
	WRITER_REPLY_BROADCAST			= 0,
	WRITER_REPLY_DIRECT				= 1,
	WRITER_REPLY_LOGOUT				= 2,
};

struct TProtocolThreadOrder{
	char ProtocolName[20];
	char Text[256];
};

struct TWriterThreadOrder{
	TWriterThreadOrderType OrderType;
	const void *Data;
};

struct TLogoutOrderData{
	uint32 CharacterID;
	int Level;
	int Profession;
	time_t LastLoginTime;
	int TutorActivities;
	char Residence[30];
};

struct TPlayerlistOrderData{
	int NumberOfPlayers;
	const char *PlayerNames;
	int *PlayerLevels;
	int *PlayerProfessions;
};

struct TKillStatisticsOrderData{
	int NumberOfRaces;
	const char *RaceNames;
	int *KilledPlayers;
	int *KilledCreatures;
};

struct TPunishmentOrderData{
	uint32 GamemasterID;
	char GamemasterName[30];
	char CriminalName[30];
	char CriminalIPAddress[16];
	int Reason;
	int Action;
	char Comment[200];
	int NumberOfStatements;
	vector<TReportedStatement> *ReportedStatements;
	uint32 StatementID;
	bool IPBanishment;
};

struct TCharacterDeathOrderData{
	uint32 CharacterID;
	int Level;
	uint32 Offender;
	char Remark[30];
	bool Unjustified;
	time_t Time;
};

struct TBuddyOrderData{
	uint32 AccountID;
	uint32 Buddy;
};

struct TWriterThreadReply{
	TWriterThreadReplyType ReplyType;
	const void *Data;
};

struct TBroadcastReplyData{
	char Message[100];
};

struct TDirectReplyData{
	uint32 CharacterID;
	char Message[100];
};

void InitProtocol(void);
void InsertProtocolOrder(const char *ProtocolName, const char *Text);
void GetProtocolOrder(TProtocolThreadOrder *Order);
void WriteProtocol(const char *ProtocolName, const char *Text);
int ProtocolThreadLoop(void *Unused);
void InitLog(const char *ProtocolName);
void Log(const char *ProtocolName, const char *Text, ...) ATTR_PRINTF(2, 3);

void InitWriterBuffers(void);
int GetOrderBufferSpace(void);
void InsertOrder(TWriterThreadOrderType OrderType, const void *Data);
void GetOrder(TWriterThreadOrder *Order);
void TerminateWriterOrder(void);
void LogoutOrder(TPlayer *Player);
void PlayerlistOrder(int NumberOfPlayers, const char *PlayerNames,
		int *PlayerLevels, int *PlayerProfessions);
void KillStatisticsOrder(int NumberOfRaces, const char *RaceNames,
		int *KilledPlayers, int *KilledCreatures);
void PunishmentOrder(TCreature *Gamemaster, const char *Name, const char *IPAddress,
		int Reason, int Action, const char *Comment, int NumberOfStatements,
		vector<TReportedStatement> *ReportedStatements, uint32 StatementID,
		bool IPBanishment);
void CharacterDeathOrder(TCreature *Creature, int OldLevel,
		uint32 OffenderID, const char *Remark, bool Unjustified);
void AddBuddyOrder(TCreature *Creature, uint32 BuddyID);
void RemoveBuddyOrder(TCreature *Creature, uint32 BuddyID);
void DecrementIsOnlineOrder(uint32 CharacterID);
void SavePlayerDataOrder(void);
void ProcessLogoutOrder(TLogoutOrderData *Data);
void ProcessPlayerlistOrder(TPlayerlistOrderData *Data);
void ProcessKillStatisticsOrder(TKillStatisticsOrderData *Data);
void ProcessPunishmentOrder(TPunishmentOrderData *Data);
void ProcessCharacterDeathOrder(TCharacterDeathOrderData *Data);
void ProcessAddBuddyOrder(TBuddyOrderData *Data);
void ProcessRemoveBuddyOrder(TBuddyOrderData *Data);
void ProcessDecrementIsOnlineOrder(uint32 CharacterID);
int WriterThreadLoop(void *Unused);

void InsertReply(TWriterThreadReplyType ReplyType, const void *Data);
void BroadcastReply(const char *Text, ...) ATTR_PRINTF(1, 2);
void DirectReply(uint32 CharacterID, const char *Text, ...) ATTR_PRINTF(2, 3);
void LogoutReply(const char *PlayerName);
bool GetReply(TWriterThreadReply *Reply);
void ProcessBroadcastReply(TBroadcastReplyData *Data);
void ProcessDirectReply(TDirectReplyData *Data);
void ProcessLogoutReply(const char *Name);
void ProcessWriterThreadReplies(void);

void ClearPlayers(void);
void InitWriter(void);
void AbortWriter(void);
void ExitWriter(void);

#endif //TIBIA_WRITER_HH_
