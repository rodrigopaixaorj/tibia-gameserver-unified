#ifndef TIBIA_COMMON_HH_
#define TIBIA_COMMON_HH_ 1

#include "../compat/compat.hh"
#include "../compat/threads.hh"
#include "../crypto/crypto.hh"
#include "enums.hh"

#define MAX_DEPOTS 9
#define MAX_SPELL_SYLLABLES 10

// Forward declarations
struct TConnection;
struct TPlayer;
struct TItem;
struct TCreature;

// Stream definitions
struct TReadStream {
	virtual bool readFlag(void);
	virtual uint8 readByte(void) = 0;
	virtual uint16 readWord(void);
	virtual uint32 readQuad(void);
	virtual void readString(char *Buffer, int MaxLength);
	virtual void readBytes(uint8 *Buffer, int Count);
	virtual bool eof(void) = 0;
	virtual void skip(int Count) = 0;
	virtual ~TReadStream(void) {}
};

struct TReadBuffer: TReadStream {
	TReadBuffer(const uint8 *Data, int Size);

	uint8 readByte(void) override;
	uint16 readWord(void) override;
	uint32 readQuad(void) override;
	void readBytes(uint8 *Buffer, int Count) override;
	bool eof(void) override;
	void skip(int Count) override;

	const uint8 *Data;
	int Size;
	int Position;
};

struct TWriteStream {
	virtual void writeFlag(bool Flag);
	virtual void writeByte(uint8 Byte) = 0;
	virtual void writeWord(uint16 Word);
	virtual void writeQuad(uint32 Quad);
	virtual void writeString(const char *String);
	virtual void writeBytes(const uint8 *Buffer, int Count);
	virtual ~TWriteStream(void) {}
};

struct TWriteBuffer: TWriteStream {
	TWriteBuffer(uint8 *Data, int Size);

	void writeByte(uint8 Byte) override;
	void writeWord(uint16 Word) override;
	void writeQuad(uint32 Quad) override;
	void writeBytes(const uint8 *Buffer, int Count) override;

	uint8 *Data;
	int Size;
	int Position;
};

struct TDynamicWriteBuffer: TWriteBuffer {
	TDynamicWriteBuffer(int InitialSize);
	void resizeBuffer(void);

	void writeByte(uint8 Byte) override;
	void writeWord(uint16 Word) override;
	void writeQuad(uint32 Quad) override;
	void writeBytes(const uint8 *Buffer, int Count) override;
	virtual ~TDynamicWriteBuffer(void);
};

// shm.cc
void StartGame(void);
void CloseGame(void);
void EndGame(void);
bool LoginAllowed(void);
bool GameRunning(void);
bool GameStarting(void);
bool GameEnding(void);
pid_t GetGameProcessID(void);
pid_t GetGameThreadID(void);
int GetPrintlogPosition(void);
char *GetPrintlogLine(int Line);
void IncrementObjectCounter(void);
void DecrementObjectCounter(void);
uint32 GetObjectCounter(void);
void IncrementPlayersOnline(void);
void DecrementPlayersOnline(void);
int GetPlayersOnline(void);
void IncrementNewbiesOnline(void);
void DecrementNewbiesOnline(void);
int GetNewbiesOnline(void);
void SetRoundNr(uint32 RoundNr);
uint32 GetRoundNr(void);
void SetCommand(int Command, char *Text);
int GetCommand(void);
char *GetCommandBuffer(void);
void InitSHM(bool Verbose);
void ExitSHM(void);

// strings.cc
const char *AddStaticString(const char *String);
uint32 AddDynamicString(const char *String);
const char *GetDynamicString(uint32 Number);
void DeleteDynamicString(uint32 Number);
void CleanupDynamicStrings(void);
void InitStrings(void);
void ExitStrings(void);

void AddSlashes(char *Destination, const char *Source);
void Trim(char *Text);
void Trim(char *Destination, const char *Source);

bool IsCountable(const char *s);
const char *Plural(const char *s, int Count);
const char *SearchForWord(const char *Pattern, const char *Text);
const char *SearchForNumber(int Count, const char *Text);
bool MatchString(const char *Pattern, const char *String);
const char *Articles(const char *s, int *Count);
const char *FirstWord(const char *s);
const char *Capitalize(const char *s);

// time.cc
extern uint32 RoundNr;
extern uint32 ServerMilliseconds;
struct tm GetLocalTimeTM(time_t t);
void GetRealTime(int *Hour, int *Minute);
void GetTime(int *Hour, int *Minute);
void GetDate(int *Year, int *Cycle, int *Day);
void GetAmbiente(int *Brightness, int *Color);
void SetRoundForNextMinute(uint32 Round);
uint32 GetRoundForNextMinute(void);
int GetLightHour(void);
int GetLightMinute(void);
void InitTimeSystem(void);
void InitTime(void);
void ExitTime(void);

// utils.cc
typedef void (TErrorFunction)(const char *);
typedef void (TPrintFunction)(int, const char *);

void SetErrorFunction(TErrorFunction *Function);
void SetPrintFunction(TPrintFunction *Function);
void error(const char *Format, ...) ATTR_PRINTF(1, 2);
void print(int Level, const char *Format, ...) ATTR_PRINTF(2, 3);
void Log(const char *File, const char *Format, ...) ATTR_PRINTF(2, 3);
int GetRandom(int Min, int Max);
int random(int Min, int Max);
bool IsOnMap(int x, int y, int z);
bool isSpace(int c);
bool isAlpha(int c);
bool isEngAlpha(int c);
bool isDigit(int c);
int toLower(int c);
int toUpper(int c);
char *strLower(char *s);
char *strUpper(char *s);
int tibia_stricmp(const char *s1, const char *s2, int Max = INT_MAX);
#define stricmp tibia_stricmp
char *findFirst(char *s, char c);
char *findLast(char *s, char c);

bool CheckBitIndex(int BitSetBytes, int Index);
bool CheckBit(uint8 *BitSet, int Index);
void SetBit(uint8 *BitSet, int Index);
void ClearBit(uint8 *BitSet, int Index);

template<typename T>
void RandomShuffle(T *Buffer, int Size){
	if(Buffer == NULL){
		error("RandomShuffle: Buffer ist NULL.\n");
		return;
	}

	int Max = Size - 1;
	for(int Min = 0; Min < Max; Min += 1){
		int Swap = random(Min, Max);
		if(Swap != Min){
			std::swap(Buffer[Min], Buffer[Swap]);
		}
	}
}

const char *GetSignalDescription(int SigNr);
const char *GetErrorDescription(int ErrCode);

// Game loop and control
void AdvanceGame(int Delay);
void ReceiveData(void);
void LogoutAllPlayers(void);

// System wide
void InitAll(void);
void ExitAll(void);

#endif // TIBIA_COMMON_HH_
