#ifndef TIBIA_MAP_HH_
#define TIBIA_MAP_HH_ 1

#include "common.hh"
#include "objects.hh"
#include "script.hh"

// NOTE(fusion): This is used by hash table entries and sectors to tell whether
// they're currently loaded or swapped out to disk.
enum : uint8 {
	STATUS_FREE = 0,
	STATUS_LOADED = 1,
	STATUS_SWAPPED = 2,

	// TODO(fusion): It seems this is only used with the `NONE` entry in the
	// hash table. I haven't seen it used **yet** but it may have a purpose
	// aside from preventing swap outs.
	STATUS_PERMANENT = 255,
};

// NOTE(fusion): This is used to determine precedence order of different objects
// in a map container.
enum : int {
	PRIORITY_BANK = 0,
	PRIORITY_CLIP = 1,
	PRIORITY_BOTTOM = 2,
	PRIORITY_TOP = 3,
	PRIORITY_CREATURE = 4,
	PRIORITY_LOW = 5,
};

struct Object {
	constexpr Object(void) : ObjectID(0) {}
	constexpr explicit Object(uint32 ObjectID): ObjectID(ObjectID) {}

	bool exists(void);
	ObjectType getObjectType(void);
	void setObjectType(ObjectType Type);
	Object getNextObject(void);
	void setNextObject(Object NextObject);
	Object getContainer(void);
	void setContainer(Object Con);
	uint32 getCreatureID(void);
	uint32 getAttribute(INSTANCEATTRIBUTE Attribute);
	void setAttribute(INSTANCEATTRIBUTE Attribute, uint32 Value);

	constexpr bool operator==(const Object &Other) const {
		return this->ObjectID == Other.ObjectID;
	}

	constexpr bool operator!=(const Object &Other) const {
		return this->ObjectID != Other.ObjectID;
	}

	// DATA
	// =================
	uint32 ObjectID;
};

constexpr Object NONE;

struct TObject {
	uint32 ObjectID;
	Object NextObject;
	Object Container;
	ObjectType Type;
	uint32 Attributes[4];
};

struct TObjectBlock {
	TObject Object[32768];
};

struct TSector {
	Object MapCon[32][32];
	uint32 TimeStamp;
	uint8 Status;
	uint8 MapFlags;
};

struct TDepotInfo {
	char Town[20];
	int Size;
};

struct TMark {
	char Name[20];
	int x;
	int y;
	int z;
};

struct TCronEntry {
	Object Obj;
	uint32 RoundNr;
	int Previous;
	int Next;
};

// NOTE(fusion): Map config values.
extern int SectorXMin;
extern int SectorXMax;
extern int SectorYMin;
extern int SectorYMax;
extern int SectorZMin;
extern int SectorZMax;
extern int RefreshedCylinders;
extern int NewbieStartPositionX;
extern int NewbieStartPositionY;
extern int NewbieStartPositionZ;
extern int VeteranStartPositionX;
extern int VeteranStartPositionY;
extern int VeteranStartPositionZ;

// NOTE(fusion): Cron management functions. Most for internal use.
Object CronCheck(void);
void CronExpire(Object Obj, int Delay);
void CronChange(Object Obj, int NewDelay);
uint32 CronInfo(Object Obj, bool Delete);
uint32 CronStop(Object Obj);

// NOTE(fusion): Map management functions. Most for internal use.
void SwapObject(TWriteBinaryFile *File, Object Obj, uintptr FileNumber);
void SwapSector(void);
void UnswapSector(uintptr FileNumber);
void DeleteSwappedSectors(void);
void LoadObjects(TReadScriptFile *Script, TWriteStream *Stream, bool Skip);
void LoadObjects(TReadStream *Stream, Object Con);
void InitSector(int SectorX, int SectorY, int SectorZ);
void LoadSector(const char *FileName, int SectorX, int SectorY, int SectorZ);
void LoadMap(void);
void SaveObjects(Object Obj, TWriteStream *Stream, bool Stop);
void SaveObjects(TReadStream *Stream, TWriteScriptFile *Script);
void SaveSector(char *FileName, int SectorX, int SectorY, int SectorZ);
void SaveMap(void);
void RefreshSector(int SectorX, int SectorY, int SectorZ, TReadStream *Stream);
void PatchSector(int SectorX, int SectorY, int SectorZ, bool FullSector,
		TReadScriptFile *Script, bool SaveHouses);
void InitMap(void);
void ExitMap(bool Save);

// NOTE(fusion): Object related functions.
TObject *AccessObject(Object Obj);
Object CreateObject(void);
void DeleteObject(Object Obj);
void ChangeObject(Object Obj, ObjectType NewType);
void ChangeObject(Object Obj, INSTANCEATTRIBUTE Attribute, uint32 Value);
int GetObjectPriority(Object Obj);
void PlaceObject(Object Obj, Object Con, bool Append);
void CutObject(Object Obj);
void MoveObject(Object Obj, Object Con);
Object AppendObject(Object Con, ObjectType Type);
Object SetObject(Object Con, ObjectType Type, uint32 CreatureID);
Object CopyObject(Object Con, Object Source);
Object SplitObject(Object Obj, int Count);
void MergeObjects(Object Obj, Object Dest);
Object GetFirstContainerObject(Object Con);
Object GetContainerObject(Object Con, int Index);
Object GetMapContainer(int x, int y, int z);
Object GetMapContainer(Object Obj);
Object GetFirstObject(int x, int y, int z);
Object GetFirstSpecObject(int x, int y, int z, ObjectType Type);
uint8 GetMapContainerFlags(Object Obj);
void GetObjectCoordinates(Object Obj, int *x, int *y, int *z);
bool CoordinateFlag(int x, int y, int z, FLAG Flag);
bool IsOnMap(int x, int y, int z);
bool IsPremiumArea(int x, int y, int z);
bool IsNoLogoutField(int x, int y, int z);
bool IsProtectionZone(int x, int y, int z);
bool IsHouse(int x, int y, int z);
uint16 GetHouseID(int x, int y, int z);
void SetHouseID(int x, int y, int z, uint16 ID);
int GetDepotNumber(const char *Town);
const char *GetDepotName(int DepotNumber);
int GetDepotSize(int DepotNumber, bool PremiumAccount);
bool GetMarkPosition(const char *Name, int *x, int *y, int *z);
void GetStartPosition(int *x, int *y, int *z, bool Newbie);

#endif //TIBIA_MAP_HH_
