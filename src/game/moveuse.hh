#ifndef TIBIA_MOVEUSE_HH_
#define TIBIA_MOVEUSE_HH_ 1

#include "common.hh"
#include "containers.hh"
#include "map.hh"
#include "script.hh"

#define MOVEUSE_MAX_PARAMETERS 5

struct TPlayerData;

enum MoveUseActionType: int {
	MOVEUSE_ACTION_CREATEONMAP				= 0,
	MOVEUSE_ACTION_CREATE					= 1,
	MOVEUSE_ACTION_MONSTERONMAP				= 2,
	MOVEUSE_ACTION_MONSTER					= 3,
	MOVEUSE_ACTION_EFFECTONMAP				= 4,
	MOVEUSE_ACTION_EFFECT					= 5,
	MOVEUSE_ACTION_TEXTONMAP				= 6,
	MOVEUSE_ACTION_TEXT						= 7,
	MOVEUSE_ACTION_CHANGEONMAP				= 8,
	MOVEUSE_ACTION_CHANGE					= 9,
	MOVEUSE_ACTION_CHANGEREL				= 10,
	MOVEUSE_ACTION_SETATTRIBUTE				= 11,
	MOVEUSE_ACTION_CHANGEATTRIBUTE			= 12,
	MOVEUSE_ACTION_SETQUESTVALUE			= 13,
	MOVEUSE_ACTION_DAMAGE					= 14,
	MOVEUSE_ACTION_SETSTART					= 15,
	MOVEUSE_ACTION_WRITENAME				= 16,
	MOVEUSE_ACTION_WRITETEXT				= 17,
	MOVEUSE_ACTION_LOGOUT					= 18,
	MOVEUSE_ACTION_MOVEALLONMAP				= 19,
	MOVEUSE_ACTION_MOVEALL					= 20,
	MOVEUSE_ACTION_MOVEALLREL				= 21,
	MOVEUSE_ACTION_MOVETOPONMAP				= 22,
	MOVEUSE_ACTION_MOVETOP					= 23,
	MOVEUSE_ACTION_MOVETOPREL				= 24,
	MOVEUSE_ACTION_MOVE						= 25,
	MOVEUSE_ACTION_MOVEREL					= 26,
	MOVEUSE_ACTION_RETRIEVE					= 27,
	MOVEUSE_ACTION_DELETEALLONMAP			= 28,
	MOVEUSE_ACTION_DELETETOPONMAP			= 29,
	MOVEUSE_ACTION_DELETEONMAP				= 30,
	MOVEUSE_ACTION_DELETE					= 31,
	MOVEUSE_ACTION_DELETEININVENTORY		= 32,
	MOVEUSE_ACTION_DESCRIPTION				= 33,
	MOVEUSE_ACTION_LOADDEPOT				= 34,
	MOVEUSE_ACTION_SAVEDEPOT				= 35,
	MOVEUSE_ACTION_SENDMAIL					= 36,
	MOVEUSE_ACTION_NOP						= 37,
};

enum MoveUseConditionType: int {
	MOVEUSE_CONDITION_ISPOSITION			= 0,
	MOVEUSE_CONDITION_ISTYPE				= 1,
	MOVEUSE_CONDITION_ISCREATURE			= 2,
	MOVEUSE_CONDITION_ISPLAYER				= 3,
	MOVEUSE_CONDITION_HASFLAG				= 4,
	MOVEUSE_CONDITION_HASTYPEATTRIBUTE		= 5,
	MOVEUSE_CONDITION_HASINSTANCEATTRIBUTE	= 6,
	MOVEUSE_CONDITION_HASTEXT				= 7,
	MOVEUSE_CONDITION_ISPEACEFUL			= 8,
	MOVEUSE_CONDITION_MAYLOGOUT				= 9,
	MOVEUSE_CONDITION_HASPROFESSION			= 10,
	MOVEUSE_CONDITION_HASLEVEL				= 11,
	MOVEUSE_CONDITION_HASRIGHT				= 12,
	MOVEUSE_CONDITION_HASQUESTVALUE			= 13,
	MOVEUSE_CONDITION_TESTSKILL				= 14,
	MOVEUSE_CONDITION_COUNTOBJECTS			= 15,
	MOVEUSE_CONDITION_COUNTOBJECTSONMAP		= 16,
	MOVEUSE_CONDITION_ISOBJECTTHERE			= 17,
	MOVEUSE_CONDITION_ISCREATURETHERE		= 18,
	MOVEUSE_CONDITION_ISPLAYERTHERE			= 19,
	MOVEUSE_CONDITION_ISOBJECTININVENTORY	= 20,
	MOVEUSE_CONDITION_ISPROTECTIONZONE		= 21,
	MOVEUSE_CONDITION_ISHOUSE				= 22,
	MOVEUSE_CONDITION_ISHOUSEOWNER			= 23,
	MOVEUSE_CONDITION_ISDRESSED				= 24,
	MOVEUSE_CONDITION_RANDOM				= 25,
};

enum MoveUseEventType: int {
	MOVEUSE_EVENT_USE						= 0,
	MOVEUSE_EVENT_MULTIUSE					= 1,
	MOVEUSE_EVENT_MOVEMENT					= 2,
	MOVEUSE_EVENT_COLLISION					= 3,
	MOVEUSE_EVENT_SEPARATION				= 4,
};

enum MoveUseModifierType: int {
	MOVEUSE_MODIFIER_NORMAL					= 0,
	MOVEUSE_MODIFIER_INVERT					= 1,
	MOVEUSE_MODIFIER_TRUE					= 2,
};

enum MoveUseParameterType: int {
	MOVEUSE_PARAMETER_OBJECT				= 0,
	MOVEUSE_PARAMETER_TYPE					= 1,
	MOVEUSE_PARAMETER_FLAG					= 2,
	MOVEUSE_PARAMETER_TYPEATTRIBUTE			= 3,
	MOVEUSE_PARAMETER_INSTANCEATTRIBUTE		= 4,
	MOVEUSE_PARAMETER_COORDINATE			= 5,
	MOVEUSE_PARAMETER_VECTOR				= 6,
	MOVEUSE_PARAMETER_RIGHT					= 7,
	MOVEUSE_PARAMETER_SKILL					= 8,
	MOVEUSE_PARAMETER_NUMBER				= 9,
	MOVEUSE_PARAMETER_TEXT					= 10,
	MOVEUSE_PARAMETER_COMPARISON			= 11,
};

struct TMoveUseAction {
	MoveUseActionType Action;
	int Parameters[MOVEUSE_MAX_PARAMETERS];
};

struct TMoveUseRule {
	int FirstCondition;
	int LastCondition;
	int FirstAction;
	int LastAction;
};

struct TMoveUseCondition {
	MoveUseModifierType Modifier;
	MoveUseConditionType Condition;
	int Parameters[MOVEUSE_MAX_PARAMETERS];
};

struct TMoveUseDatabase {
	TMoveUseDatabase(void) : Rules(1, 100, 100), NumberOfRules(0) {}

	vector<TMoveUseRule> Rules;
	int NumberOfRules;
};

struct TDelayedMail {
    uint32 CharacterID;
    int DepotNumber;
    uint8 *Packet;
    int PacketSize;
};

int PackAbsoluteCoordinate(int x, int y, int z);
void UnpackAbsoluteCoordinate(int Packed, int *x, int *y, int *z);
int PackRelativeCoordinate(int x, int y, int z);
void UnpackRelativeCoordinate(int Packed, int *x, int *y, int *z);

Object GetEventObject(int Nr, Object User, Object Obj1, Object Obj2, Object Temp);
bool Compare(int Value1, int Operator, int Value2);
bool CheckCondition(MoveUseEventType EventType, TMoveUseCondition *Condition,
		Object User, Object Obj1, Object Obj2, Object *Temp);
Object CreateObject(Object Con, ObjectType Type, uint32 Value);
void ChangeObject(Object Obj, ObjectType NewType, uint32 Value);
void MoveOneObject(Object Obj, Object Con);
void MoveAllObjects(Object Obj, Object Dest, Object Exclude, bool MoveUnmovable);
void DeleteAllObjects(Object Obj, Object Exclude, bool DeleteUnmovable);
void ClearField(Object Obj, Object Exclude);
void LoadDepotBox(uint32 CreatureID, int Nr, Object Con);
void SaveDepotBox(uint32 CreatureID, int Nr, Object Con);
void SendMail(Object Obj);
void SendMails(TPlayerData *PlayerData);
void TextEffect(const char *Text, int x, int y, int z, int Radius);
void ExecuteAction(MoveUseEventType EventType, TMoveUseAction *Action,
		Object User, Object Obj1, Object Obj2, Object *Temp);
bool HandleEvent(MoveUseEventType EventType, Object User, Object Obj1, Object Obj2);

void UseContainer(uint32 CreatureID, Object Con, int NextContainerNr);
void UseChest(uint32 CreatureID, Object Chest);
void UseLiquidContainer(uint32 CreatureID, Object Obj, Object Dest);
void UseFood(uint32 CreatureID, Object Obj);
void UseTextObject(uint32 CreatureID, Object Obj);
void UseAnnouncer(uint32 CreatureID, Object Obj);
void UseKeyDoor(uint32 CreatureID, Object Key, Object Door);
void UseNameDoor(uint32 CreatureID, Object Door);
void UseLevelDoor(uint32 CreatureID, Object Door);
void UseQuestDoor(uint32 CreatureID, Object Door);
void UseWeapon(uint32 CreatureID, Object Weapon, Object Target);
void UseChangeObject(uint32 CreatureID, Object Obj);
void UseObject(uint32 CreatureID, Object Obj);
void UseObjects(uint32 CreatureID, Object Obj1, Object Obj2);
void MovementEvent(Object Obj, Object Start, Object Dest);
void SeparationEvent(Object Obj, Object Start);
void CollisionEvent(Object Obj, Object Dest);

void LoadParameters(TReadScriptFile *Script, int *Parameters, int NumberOfParameters, ...);
void LoadCondition(TReadScriptFile *Script, TMoveUseCondition *Condition);
void LoadAction(TReadScriptFile *Script, TMoveUseAction *Action);
void LoadDataBase(void);


void InitMoveUse(void);
void ExitMoveUse(void);

#endif //TIBIA_MOVEUSE_HH_
