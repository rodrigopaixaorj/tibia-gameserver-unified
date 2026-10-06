#ifndef TIBIA_READER_HH_
#define TIBIA_READER_HH_ 1

#include "common.hh"

struct TPlayerData;

typedef void TRefreshSectorFunction(int SectorX, int SectorY, int SectorZ,
									const uint8 *Data, int Size);
typedef void TSendMailsFunction(TPlayerData *PlayerData);

enum TReaderThreadOrderType: int {
	READER_ORDER_TERMINATE		= 0,
	READER_ORDER_LOADSECTOR		= 1,
	READER_ORDER_LOADCHARACTER	= 2,
};

enum TReaderThreadReplyType: int {
	READER_REPLY_SECTORDATA		= 0,
	READER_REPLY_CHARACTERDATA	= 1,
};

struct TReaderThreadOrder {
	TReaderThreadOrderType OrderType;
	int SectorX;
	int SectorY;
	int SectorZ;
	uint32 CharacterID;
};

struct TReaderThreadReply {
	TReaderThreadReplyType ReplyType;
	int SectorX;
	int SectorY;
	int SectorZ;
	uint8 *Data;
	int Size;
};

void InitReaderBuffers(void);
void InsertOrder(TReaderThreadOrderType OrderType,
		int SectorX, int SectorY, int SectorZ, uint32 CharacterID);
void GetOrder(TReaderThreadOrder *Order);
void TerminateReaderOrder(void);
void LoadSectorOrder(int SectorX, int SectorY, int SectorZ);
void LoadCharacterOrder(uint32 CharacterID);
void ProcessLoadSectorOrder(int SectorX, int SectorY, int SectorZ);
void ProcessLoadCharacterOrder(uint32 CharacterID);
int ReaderThreadLoop(void *Unused);

void InsertReply(TReaderThreadReplyType ReplyType,
		int SectorX, int SectorY, int SectorZ, uint8 *Data, int Size);
bool GetReply(TReaderThreadReply *Reply);
void SectorReply(int SectorX, int SectorY, int SectorZ, uint8 *Data, int Size);
void CharacterReply(uint32 CharacterID);
void ProcessSectorReply(TRefreshSectorFunction *RefreshSector,
		int SectorX, int SectorY, int SectorZ, uint8 *Data, int Size);
void ProcessCharacterReply(TSendMailsFunction *SendMails, uint32 CharacterID);
void ProcessReaderThreadReplies(TRefreshSectorFunction *RefreshSector, TSendMailsFunction *SendMails);

void InitReader(void);
void ExitReader(void);

#endif //TIBIA_READER_HH
