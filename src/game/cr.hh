#ifndef TIBIA_CREATURE_HH_
#define TIBIA_CREATURE_HH_ 1

#include "common.hh"
#include "connections.hh"
#include "containers.hh"
#include "enums.hh"
#include "map.hh"

struct TCreature;
struct TMonster;
struct TNPC;
struct TPlayer;

// Creature Data
// =============================================================================
struct TOutfit{
	int OutfitID;
	union{
		int ObjectType;
		uint8 Colors[4];
	};

	constexpr bool operator==(const TOutfit &Other) const {
		// IMPORTANT(fusion): We don't need to have special comparisson cases
		// if all members of the union have the same size. This is not true
		// otherwise when smaller active fields leave trailing bytes of the
		// union filled with whatever data was there before.
		STATIC_ASSERT(sizeof(this->ObjectType) == sizeof(this->Colors));
		return this->OutfitID == Other.OutfitID
			&& this->ObjectType == Other.ObjectType;
	}

	static constexpr TOutfit Invisible(void){
		return TOutfit{};
	}
};

struct TSkillData {
	int Nr;
	int Actual;
	int Minimum;
	int Maximum;
	int NextLevel;
	int FactorPercent;
	int AddLevel;
};

struct TItemData {
	ObjectType Type;
	int Maximum;
	int Probability;
};

struct TSpellData {
	SpellShapeType Shape;
	int ShapeParam1;
	int ShapeParam2;
	int ShapeParam3;
	int ShapeParam4;
	SpellImpactType Impact;
	int ImpactParam1;
	int ImpactParam2;
	int ImpactParam3;
	int ImpactParam4;
	int Delay;
};

struct TRaceData {
	TRaceData(void);

	// DATA
	// =================
	char Name[30];
	char Article[3];
	TOutfit Outfit;
	ObjectType MaleCorpse;
	ObjectType FemaleCorpse;
	BloodType Blood;
	int ExperiencePoints;
	int FleeThreshold;
	int Attack;
	int Defend;
	int Armor;
	int Poison;
	int SummonCost;
	int LoseTarget;
	int Strategy[4];
	bool KickBoxes;
	bool KickCreatures;
	bool SeeInvisible;
	bool Unpushable;
	bool DistanceFighting;
	bool NoSummon;
	bool NoIllusion;
	bool NoConvince;
	bool NoBurning;
	bool NoPoison;
	bool NoEnergy;
	bool NoHit;
	bool NoLifeDrain;
	bool NoParalyze;
	int Skills;
	vector<TSkillData> Skill;
	int Talks;
	vector<uint32> Talk; // POINTER? Probably a reference from `AddDynamicString`?
	int Items;
	vector<TItemData> Item;
	int Spells;
	vector<TSpellData> Spell;
};

struct TPlayerData {
	uint32 CharacterID;
	pid_t Locked;
	int Sticky;
	bool Dirty;
	int Race;
	TOutfit OriginalOutfit;
	TOutfit CurrentOutfit;
	time_t LastLoginTime;
	time_t LastLogoutTime;
	int startx;
	int starty;
	int startz;
	int posx;
	int posy;
	int posz;
	int Profession;
	int PlayerkillerEnd;
	int Actual[25];
	int Maximum[25];
	int Minimum[25];
	int DeltaAct[25];
	int MagicDeltaAct[25];
	int Cycle[25];
	int MaxCycle[25];
	int Count[25];
	int MaxCount[25];
	int AddLevel[25];
	int Experience[25];
	int FactorPercent[25];
	int NextLevel[25];
	int Delta[25];
	uint8 SpellList[256];
	int QuestValues[500];
	int MurderTimestamps[20];
	uint8 *Inventory;
	int InventorySize;
	uint8 *Depot[MAX_DEPOTS];
	int DepotSize[MAX_DEPOTS];
	uint32 AccountID;
	int Sex;
	char Name[30];
	uint8 Rights[12];
	char Guild[31];
	char Rank[31];
	char Title[31];
	int Buddies;
	uint32 Buddy[100];
	char BuddyName[100][30];
	uint32 EarliestYellRound;
	uint32 EarliestTradeChannelRound;
	uint32 EarliestSpellTime;
	uint32 EarliestMultiuseTime;
	uint32 TalkBufferFullTime;
	uint32 MutingEndRound;
	uint32 Addressees[20];
	uint32 AddresseesTimes[20];
	int NumberOfMutings;
};

// TAttackWave
// =============================================================================
struct TAttackWave {
	TAttackWave(void);
	~TAttackWave(void);

	// DATA
	// =================
	int x;
	int y;
	int z;
	int Spread;
	int Race;
	int MinCount;
	int MaxCount;
	int Radius;
	int Lifetime;
	uint32 Message;
	int ExtraItems;
	vector<TItemData> ExtraItem;
};

// TSkillBase 
// =============================================================================
struct TSkill{
	TSkill(int SkNr, TCreature *Master);
	int Get(void);
	int GetProgress(void);
	void Check(void);
	void Change(int Amount);
	void SetMax(void);
	void DecreasePercent(int Percent);
	void SetMDAct(int MDAct);
	void Load(int Act, int Max, int Min, int DAct, int MDAct,
			int Cycle, int MaxCycle, int Count, int MaxCount, int AddLevel,
			int Exp, int FactorPercent, int NextLevel, int Delta);
	void Save(int *Act, int *Max, int *Min, int *DAct, int *MDAct,
			int *Cycle, int *MaxCycle, int *Count, int *MaxCount, int *AddLevel,
			int *Exp, int *FactorPercent, int *NextLevel, int *Delta);

	// VIRTUAL FUNCTIONS
	// =================
	virtual ~TSkill(void);																// VTABLE[ 0]
	// Duplicate destructor that also calls operator delete.							// VTABLE[ 1]
	virtual void Set(int Value);														// VTABLE[ 2]
	virtual void Increase(int Amount);													// VTABLE[ 3]
	virtual void Decrease(int Amount);													// VTABLE[ 4]
	virtual int GetExpForLevel(int Level);												// VTABLE[ 5]
	virtual void Advance(int Range);													// VTABLE[ 6]
	virtual void ChangeSkill(int FactorPercent, int Delta);								// VTABLE[ 7]
	virtual int ProbeValue(int Max, bool Increase);										// VTABLE[ 8]
	virtual bool Probe(int Diff, int Prob, bool Increase);								// VTABLE[ 9]
	virtual bool Process(void);															// VTABLE[10]
	virtual bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue);		// VTABLE[11]
	virtual bool DelTimer(void);														// VTABLE[12]
	virtual int TimerValue(void);														// VTABLE[13]
	virtual bool Jump(int Range);														// VTABLE[14]
	virtual void Event(int Range);														// VTABLE[15]
	virtual void Reset(void);															// VTABLE[16]

	// DATA
	// =================
	//void *VTABLE;		// IMPLICIT
	int DAct;			// Delta Value - Probably from equipment.
	int MDAct;			// Delta Magic Value - Probably from spells.
	uint16 SkNr;
	TCreature *Master;
	int Act;			// Actual Value (?)
	int Max;
	int Min;
	int FactorPercent;
	int LastLevel;
	int NextLevel;
	int Delta;
	int Exp;
	int Cycle;
	int MaxCycle;
	int Count;
	int MaxCount;
	int AddLevel;
};

struct TSkillLevel: TSkill {
	TSkillLevel(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Increase(int Amount) override;
	void Decrease(int Amount) override;
	int GetExpForLevel(int Level) override;
	bool Jump(int Range) override;
};

struct TSkillProbe: TSkill {
	TSkillProbe(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Increase(int Amount) override;
	void Decrease(int Amount) override;
	int GetExpForLevel(int Level) override;
	void ChangeSkill(int FactorPercent, int Delta) override;
	int ProbeValue(int Max, bool Increase) override;
	bool Probe(int Diff, int Prob, bool Increase) override;
	bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue) override;
	bool Jump(int Range) override;
	void Event(int Range) override;
};

struct TSkillAdd: TSkill {
	TSkillAdd(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Advance(int Range) override;
};

struct TSkillHitpoints: TSkillAdd {
	TSkillHitpoints(int SkNr, TCreature *Master) : TSkillAdd(SkNr, Master) {}
	void Set(int Value) override;
};

struct TSkillMana: TSkillAdd {
	TSkillMana(int SkNr, TCreature *Master) : TSkillAdd(SkNr, Master) {}
	void Set(int Value) override;
};

struct TSkillGoStrength: TSkillAdd {
	TSkillGoStrength(int SkNr, TCreature *Master) : TSkillAdd(SkNr, Master) {}
	bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue) override;
	void Event(int Range) override;
};

struct TSkillCarryStrength: TSkillAdd {
	TSkillCarryStrength(int SkNr, TCreature *Master) : TSkillAdd(SkNr, Master) {}
	void Set(int Value) override;
};

struct TSkillSoulpoints: TSkillAdd {
	TSkillSoulpoints(int SkNr, TCreature *Master) : TSkillAdd(SkNr, Master) {}
	void Set(int Value) override;
	int TimerValue(void) override;
	void Event(int Range) override;
};

struct TSkillFed: TSkill {
	TSkillFed(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Event(int Range) override;
};

struct TSkillLight: TSkill {
	TSkillLight(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue) override;
	void Event(int Range) override;
};

struct TSkillIllusion: TSkill {
	TSkillIllusion(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue) override;
	void Event(int Range) override;
};

struct TSkillPoison: TSkill {
	TSkillPoison(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	bool Process(void) override;
	bool SetTimer(int Cycle, int Count, int MaxCount, int AdditionalValue) override;
	void Event(int Range) override;
	void Reset(void) override;
};

struct TSkillBurning: TSkill {
	TSkillBurning(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Event(int Range) override;
};

struct TSkillEnergy: TSkill {
	TSkillEnergy(int SkNr, TCreature *Master) : TSkill(SkNr, Master) {}
	void Event(int Range) override;
};

struct TSkillBase{
	TSkillBase(void);
	~TSkillBase(void);
	bool NewSkill(uint16 SkillNo, TCreature *Creature);
	bool SetSkills(int Race);
	void ProcessSkills(void);
	bool SetTimer(uint16 SkNr, int Cycle, int Count, int MaxCount, int AdditionalValue);
	void DelTimer(uint16 SkNr);

	// DATA
	// =================
	TSkill *Skills[25];
	TSkill *TimerList[25];
	uint16 FirstFreeTimer;
};

// TCombat
// =============================================================================
struct TCombatEntry{
	uint32 ID;
	uint32 Damage;
	uint32 TimeStamp;
};

struct TCombat{
	TCombat(void);
	void GetWeapon(void);
	void GetAmmo(void);
	void CheckCombatValues(void);
	void GetAttackValue(int *Value, int *SkillNr);
	void GetDefendValue(int *Value, int *SkillNr);
	int GetAttackDamage(void);
	int GetDefendDamage(void);
	int GetArmorStrength(void);
	int GetDistance(void);
	void ActivateLearning(void);
	void SetAttackMode(uint8 AttackMode);
	void SetChaseMode(uint8 ChaseMode);
	void SetSecureMode(uint8 SecureMode);
	void SetAttackDest(uint32 TargetID, bool Follow);
	void CanToDoAttack(void);
	void StopAttack(int Delay);
	void DelayAttack(int Milliseconds);
	void Attack(void);
	void CloseAttack(TCreature *Target);
	void DistanceAttack(TCreature *Target);
	void WandAttack(TCreature *Target);
	void AddDamageToCombatList(uint32 Attacker, uint32 Damage);
	uint32 GetDamageByCreature(uint32 CreatureID);
	uint32 GetMostDangerousAttacker(void);
	void DistributeExperiencePoints(uint32 Exp);


	// DATA
	// =================
	TCreature *Master;
	uint32 EarliestAttackTime;
	uint32 EarliestDefendTime;
	uint32 LastDefendTime;
	uint32 LatestAttackTime;
	uint8 AttackMode;
	uint8 ChaseMode;
	uint8 SecureMode;
	uint32 AttackDest;
	bool Following;
	Object Shield;
	Object Close;
	Object Missile;
	Object Throw;
	Object Wand;
	Object Ammo;
	bool Fist;
	uint32 CombatDamage;
	int ActCombatEntry;
	TCombatEntry CombatList[20];
	int LearningPoints;
};

// TFindCreatures
// =============================================================================
enum : int {
	FIND_PLAYERS	= 0x01,
	FIND_NPCS		= 0x02,
	FIND_MONSTERS	= 0x04,
	FIND_ALL		= FIND_PLAYERS | FIND_NPCS | FIND_MONSTERS,
};

struct TFindCreatures {
	TFindCreatures(int RadiusX, int RadiusY, int CenterX, int CenterY, int Mask);
	TFindCreatures(int RadiusX, int RadiusY, uint32 CreatureID, int Mask);
	TFindCreatures(int RadiusX, int RadiusY, Object Obj, int Mask);
	void initSearch(int RadiusX, int RadiusY, int CenterX, int CenterY, int Mask);
	uint32 getNext(void);

	// DATA
	// =================
	int startx;
	int starty;
	int endx;
	int endy;
	int blockx;
	int blocky;
	uint32 ActID;
	uint32 SkipID;
	int Mask;
	bool finished;
};

// TCreature
// =============================================================================
enum : int {
	LOSE_INVENTORY_NONE		= 0,
	LOSE_INVENTORY_SOME		= 1,
	LOSE_INVENTORY_ALL		= 2,
};

struct TToDoEntry {
	ToDoType Code;
	union{
		struct{
			uint32 Time;
		} Wait;

		struct{
			int x;
			int y;
			int z;
		} Go;

		struct{
			int Direction;
		} Rotate;

		struct{
			uint32 Obj;
			int x;
			int y;
			int z;
			int Count;
		} Move;

		struct{
			uint32 Obj;
			uint32 Partner;
		} Trade;

		struct{
			uint32 Obj1;
			uint32 Obj2;
			int Dummy;
		} Use;

		struct{
			uint32 Obj;
		} Turn;

		struct{
			uint32 Text;
			int Mode;
			uint32 Addressee;
			bool CheckSpamming;
		} Talk;

		struct{
			int NewState;
		} ChangeState;
	};
};

struct TCreature: TSkillBase {
	// crmain.cc
	TCreature(void);
	void SetID(uint32 CharacterID);
	void DelID(void);
	void SetInCrList(void);
	void DelInCrList(void);
	void StartLogout(bool Force, bool StopFight);
	int LogoutPossible(void);
	void BlockLogout(int Delay, bool BlockProtectionZone);
	int GetHealth(void);
	int GetSpeed(void);
	int Damage(TCreature *Attacker, int Damage, int DamageType);

	// cract.cc
	bool SetOnMap(void);
	bool DelOnMap(void);
	void Go(int DestX, int DestY, int DestZ);
	void Rotate(int Direction);
	void Rotate(TCreature *Target);
	void Move(Object Obj, int DestX, int DestY, int DestZ, uint8 Count);
	void Trade(Object Obj, uint32 TradePartner);
	void Use(Object Obj1, Object Obj2, uint8 Dummy);
	void Turn(Object Obj);
	void Attack(void);
	void Execute(void);
	uint32 CalculateDelay(void);
	bool ToDoClear(void);
	void ToDoAdd(TToDoEntry TD);
	void ToDoStop(void);
	void ToDoStart(void);
	void ToDoYield(void);
	void ToDoWait(int Delay);
	void ToDoWaitUntil(uint32 Time);
	void ToDoGo(int DestX, int DestY, int DestZ, bool MustReach, int MaxSteps);
	void ToDoRotate(int Direction);
	void ToDoMove(int ObjX, int ObjY, int ObjZ, ObjectType Type, uint8 RNum,
				int DestX, int DestY, int DestZ, uint8 Count);
	void ToDoMove(Object Obj, int DestX, int DestY, int DestZ, uint8 Count);
	void ToDoTrade(int ObjX, int ObjY, int ObjZ, ObjectType Type, uint8 RNum,
				uint32 TradePartner);
	void ToDoUse(uint8 Count, int ObjX1, int ObjY1, int ObjZ1, ObjectType Type1, uint8 RNum1,
				uint8 Dummy, int ObjX2, int ObjY2, int ObjZ2, ObjectType Type2, uint8 RNum2);
	void ToDoUse(uint8 Count, Object Obj1, Object Obj2);
	void ToDoTurn(int ObjX, int ObjY, int ObjZ, ObjectType Type, uint8 RNum);
	void ToDoAttack(void);
	void ToDoTalk(int Mode, const char *Addressee, const char *Text, bool CheckSpamming);
	void ToDoChangeState(int NewState);
	void NotifyGo(void);
	void NotifyTurn(Object DestCon);
	void NotifyCreate(void);
	void NotifyDelete(void);
	void NotifyChangeInventory(void);

	void Kill(void){
		this->Skills[SKILL_HITPOINTS]->Set(0);
		this->Death();
	}

	bool IsInvisible(void){
		return this->Outfit == TOutfit::Invisible();
	}

	bool CanSeeFloor(int FloorZ){
		if(this->posz <= 7){
			return FloorZ <= 7;
		}else{
			return std::abs(this->posz - FloorZ) <= 2;
		}
	}

	// VIRTUAL FUNCTIONS
	// =================
	virtual ~TCreature(void);															// VTABLE[ 0]
	// Duplicate destructor that also calls operator delete.							// VTABLE[ 1]
	virtual void Death(void);															// VTABLE[ 2]
	virtual bool MovePossible(int x, int y, int z, bool Execute, bool Jump);			// VTABLE[ 3]
	virtual bool IsPeaceful(void);														// VTABLE[ 4]
	virtual uint32 GetMaster(void);														// VTABLE[ 5]
	virtual void TalkStimulus(uint32 SpeakerID, const char *Text);						// VTABLE[ 6]
	virtual void DamageStimulus(uint32 AttackerID, int Damage, int DamageType);			// VTABLE[ 7]
	virtual void IdleStimulus(void);													// VTABLE[ 8]
	virtual void CreatureMoveStimulus(uint32 CreatureID, int Type);						// VTABLE[ 9]
	virtual void AttackStimulus(uint32 AttackerID);										// VTABLE[10]

	// DATA
	// =================
	//void *VTABLE;					// IMPLICIT
	//TSkillBase super_TSkillBase;	// IMPLICIT
	TCombat Combat;
	uint32 ID;
	TCreature *NextHashEntry;
	uint32 NextChainCreature;
	char Name[31];
	char Murderer[31];
	TOutfit OrgOutfit;
	TOutfit Outfit;
	int startx;
	int starty;
	int startz;
	int posx;
	int posy;
	int posz;
	int Sex;
	int Race;
	int Direction;
	int Radius;
	CreatureType Type;
	bool IsDead;
	int LoseInventory;
	bool LoggingOut;
	bool LogoutAllowed;
	uint32 EarliestLogoutRound;
	uint32 EarliestProtectionZoneRound;
	uint32 EarliestYellRound;
	uint32 EarliestTradeChannelRound;
	uint32 EarliestSpellTime;
	uint32 EarliestMultiuseTime;
	uint32 EarliestWalkTime;
	uint32 LifeEndRound;
	TKnownCreature *FirstKnowingConnection;
	int SummonedCreatures;
	uint32 FireDamageOrigin;
	uint32 PoisonDamageOrigin;
	uint32 EnergyDamageOrigin;
	Object CrObject;
	vector<TToDoEntry> ToDoList;
	int ActToDo;
	int NrToDo;
	uint32 NextWakeup;
	bool Stop;
	bool LockToDo;
	uint8 Profession;
	TConnection *Connection;
};

// TNonPlayer
// =============================================================================
struct TNonplayer: TCreature {
	TNonplayer(void);
	~TNonplayer(void) override;
	void SetInList(void);
	void DelInList(void);

	// DATA
	// =================
	//TCreature super_TCreature;	// IMPLICIT
	STATE State;
};

// TNPC
// =============================================================================
struct TBehaviourNode {
	int Type;
	int Data;
	TBehaviourNode *Left;
	TBehaviourNode *Right;
};

struct TBehaviourCondition {
	bool set(int Type, void *Data);
	void clear(void);

	// DATA
	// =================
	int Type;
	uint32 Text;
	TBehaviourNode *Expression;
	int Property;
	int Number;
};

struct TBehaviourAction {
	bool set(int Type, void *Data, void *Data2, void *Data3, void *Data4);
	void clear(void);

	// DATA
	// =================
	int Type;
	uint32 Text;
	int Number;
	TBehaviourNode *Expression;
	TBehaviourNode *Expression2;
	TBehaviourNode *Expression3;
};

struct TBehaviour {
	TBehaviour(void);
	~TBehaviour(void);
	bool addCondition(int Type, void *Data);
	bool addAction(int Type, void *Data, void *Data2, void *Data3, void *Data4);

	// TODO(fusion): Same as `TChannel` in `operate.hh`.
	TBehaviour(const TBehaviour &Other);
	void operator=(const TBehaviour &Other);

	// DATA
	// =================
	vector<TBehaviourCondition> Condition;
	vector<TBehaviourAction> Action;
	int Conditions;
	int Actions;
};

struct TBehaviourDatabase {
	TBehaviourDatabase(TReadScriptFile *Script);

	// TODO(fusion): These could/should be standalone functions.
	TBehaviourNode *readValue(TReadScriptFile *Script);
	TBehaviourNode *readFactor(TReadScriptFile *Script);
	TBehaviourNode *readTerm(TReadScriptFile *Script);
	int evaluate(TNPC *Npc, TBehaviourNode *Node, int *Parameters);

	void react(TNPC *Npc, const char *Text, SITUATION Situation);

	// DATA
	// =================
	vector<TBehaviour> Behaviour;
	int Behaviours;
};

struct TNPC: TNonplayer {
	TNPC(const char *FileName);
	void GiveTo(ObjectType Type, int Amount);
	void GetFrom(ObjectType Type, int Amount);
	void GiveMoney(int Amount);
	void GetMoney(int Amount);
	void Enqueue(uint32 InterlocutorID, const char *Text);
	void TurnToInterlocutor(void);
	void ChangeState(STATE NewState, bool Stimulus);

	// VIRTUAL FUNCTIONS
	// =================
	~TNPC(void) override;
	bool MovePossible(int x, int y, int z, bool Execute, bool Jump) override;
	void TalkStimulus(uint32 SpeakerID, const char *Text) override;
	void DamageStimulus(uint32 AttackerID, int Damage, int DamageType) override;
	void IdleStimulus(void) override;
	void CreatureMoveStimulus(uint32 CreatureID, int Type) override;

	// DATA
	// =================
	//TNonplayer super_TNonplayer; 	// IMPLICIT
	uint32 Interlocutor;
	int Topic;
	int Price;
	int Amount;
	int TypeID;
	uint32 Data;
	uint32 LastTalk;
	vector<uint32> QueuedPlayers;
	vector<uint32> QueuedAddresses;
	int QueueLength;
	TBehaviourDatabase *Behaviour;
};

// TMonster
// =============================================================================
struct TMonsterhome {
	int Race;
	int x;
	int y;
	int z;
	int Radius;
	int MaxMonsters;
	int ActMonsters;
	int RegenerationTime;
	int Timer;
};

struct TMonster: TNonplayer {
	TMonster(int Race, int x, int y, int z, int Home, uint32 MasterID);
	bool CanKickBoxes(void);
	void KickBoxes(Object Obj);
	bool KickCreature(TCreature *Creature);
	void Convince(TCreature *NewMaster);
	void SetTarget(TCreature *NewTarget);
	bool IsPlayerControlled(void);
	bool IsFleeing(void);

	// VIRTUAL FUNCTIONS
	// =================
	~TMonster(void) override;
	bool MovePossible(int x, int y, int z, bool Execute, bool Jump) override;
	bool IsPeaceful(void) override;
	uint32 GetMaster(void) override;
	void DamageStimulus(uint32 AttackerID, int Damage, int DamageType) override;
	void IdleStimulus(void) override;
	void CreatureMoveStimulus(uint32 CreatureID, int Type) override;

	// DATA
	//TNonplayer super_TNonplayer;	// IMPLICIT
	int Home;
	uint32 Master;
	uint32 Target;
};

// TPlayer
// =============================================================================
struct TPlayerIndexNode {
	bool InternalNode;
};

struct TPlayerIndexInternalNode: TPlayerIndexNode {
	TPlayerIndexNode *Child[27];
};

struct TPlayerIndexEntry {
	char Name[30];
	uint32 CharacterID;
};

struct TPlayerIndexLeafNode: TPlayerIndexNode {
	int Count;
	TPlayerIndexEntry Entry[10];
};

struct TPlayer: TCreature {
	TPlayer(TConnection *Connection, uint32 CharacterID);
	void SetInList(void);
	void DelInList(void);
	void ClearRequest(void);
	void ClearConnection(void);
	void LoadData(void);
	void SaveData(void);
	void LoadInventory(bool SetStandardInventory);
	void SaveInventory(void);
	void StartCoordinates(void);
	void TakeOver(TConnection *Connection);
	void SetOpenContainer(int ContainerNr, Object Con);
	Object GetOpenContainer(int ContainerNr);
	void CloseAllContainers(void);
	Object InspectTrade(bool OwnOffer, int Position);
	void AcceptTrade(void);
	void RejectTrade(void);
	void ClearProfession(void);
	void SetProfession(uint8 Profession);
	uint8 GetRealProfession(void);
	uint8 GetEffectiveProfession(void);
	uint8 GetActiveProfession(void);
	bool GetActivePromotion(void);
	bool SpellKnown(int SpellNr);
	void LearnSpell(int SpellNr);
	int GetQuestValue(int QuestNr);
	void SetQuestValue(int QuestNr, int Value);
	void CheckOutfit(void);
	void CheckState(void);
	void SyncState(void);
	void AddBuddy(const char *Name);
	void RemoveBuddy(uint32 CharacterID);
	void SendBuddies(void);
	void Regenerate(void);
	bool IsAttacker(uint32 VictimID, bool CheckFormer);
	bool IsAggressor(bool CheckFormer);
	bool IsAttackJustified(uint32 VictimID);
	void RecordAttack(uint32 VictimID);
	void RecordMurder(uint32 VictimID);
	void RecordDeath(uint32 AttackerID, int OldLevel, const char *Remark);
	int CheckPlayerkilling(int Now);
	void ClearAttacker(uint32 VictimID);
	void ClearPlayerkillingMarks(void);
	int GetPlayerkillingMark(TPlayer *Observer);
	uint32 GetPartyLeader(bool CheckFormer);
	bool InPartyWith(TPlayer *Other, bool CheckFormer);
	void JoinParty(uint32 LeaderID);
	void LeaveParty(void);
	int GetPartyMark(TPlayer *Observer);
	int RecordTalk(void);
	int RecordMessage(uint32 AddresseeID);
	int CheckForMuting(void);

	// VIRTUAL FUNCTIONS
	// =================
	~TPlayer(void) override;
	void Death(void) override;
	bool MovePossible(int x, int y, int z, bool Execute, bool Jump) override;
	void DamageStimulus(uint32 AttackerID, int Damage, int DamageType) override;
	void IdleStimulus(void) override;
	void AttackStimulus(uint32 AttackerID) override;

	// DATA
	// =================
	//TCreature super_TCreature;	// IMPLICIT
	uint32 AccountID;
	char Guild[31];
	char Rank[31];
	char Title[31];
	char IPAddress[16];
	uint8 Rights[12];
	Object Depot;
	int DepotNr;
	int DepotSpace;
	RESULT ConstructError;
	TPlayerData *PlayerData;
	Object TradeObject;
	uint32 TradePartner;
	bool TradeAccepted;
	int OldState;
	uint32 Request;
	int RequestTimestamp;
	uint32 RequestProcessingGamemaster;
	int TutorActivities;
	uint8 SpellList[256];
	int QuestValues[500];
	Object OpenContainer[16];
	vector<uint32> AttackedPlayers;
	int NumberOfAttackedPlayers;
	bool Aggressor;
	vector<uint32> FormerAttackedPlayers;
	int NumberOfFormerAttackedPlayers;
	bool FormerAggressor;
	uint32 FormerLogoutRound;
	uint32 PartyLeader;
	uint32 PartyLeavingRound;
	uint32 TalkBufferFullTime;
	uint32 MutingEndRound;
	int NumberOfMutings;
	uint32 Addressees[20];
	uint32 AddresseesTimes[20];
};

// crmain.cc
// =============================================================================
#define MAX_RACES 512
extern TRaceData RaceData[MAX_RACES];
extern priority_queue<uint32, uint32> ToDoQueue;

bool IsCreaturePlayer(uint32 CreatureID);
TCreature *GetCreature(uint32 CreatureID);
TCreature *GetCreature(Object Obj);
void InsertChainCreature(TCreature *Creature, int CoordX, int CoordY);
void DeleteChainCreature(TCreature *Creature);
void MoveChainCreature(TCreature *Creature, int CoordX, int CoordY);
void ProcessCreatures(void);
void ProcessSkills(void);
void MoveCreatures(int Delay);

void AddKillStatistics(int AttackerRace, int DefenderRace);
void WriteKillStatistics(void);
void InitKillStatistics(void);
void ExitKillStatistics(void);

bool IsRaceValid(int Race);
int GetRaceByName(const char *RaceName);
const char *GetRaceName(int Race);
TOutfit GetRaceOutfit(int Race);
bool GetRaceNoSummon(int Race);
bool GetRaceNoConvince(int Race);
bool GetRaceNoIllusion(int Race);
bool GetRaceNoParalyze(int Race);
int GetRaceSummonCost(int Race);
int GetRacePoison(int Race);
bool GetRaceUnpushable(int Race);
TOutfit ReadOutfit(TReadScriptFile *Script);
void WriteOutfit(TWriteScriptFile *Script, TOutfit Outfit);
void LoadRace(const char *FileName);
void LoadRaces(void);

void LoadMonsterRaid(const char *FileName, int Start,
		bool *Type, int *Date, int *Interval, int *Duration);
void LoadMonsterRaids(void);
void ProcessMonsterRaids(void);

void InitCr(void);
void ExitCr(void);

// crskill.cc
// =============================================================================
int GetSkillByName(const char *Name);

// crnonpl.cc
// =============================================================================
void StartMonsterhomeTimer(int Nr);
void LoadMonsterhomes(void);
void ProcessMonsterhomes(void);
void NotifyMonsterhomeOfDeath(int Nr);
bool MonsterhomeInRange(int Nr, int x, int y, int z);

void ChangeNPCState(TCreature *Npc, int NewState, bool Stimulus);

TCreature *CreateMonster(int Race, int x, int y, int z,
		int Home, uint32 MasterID, bool ShowEffect);
void ConvinceMonster(TCreature *Master, TCreature *Slave);
void ChallengeMonster(TCreature *Challenger, TCreature *Monster);

void InitNPCs(void);
void InitNonplayer(void);
void ExitNonplayer(void);

// crplayer.cc
// =============================================================================
int GetNumberOfPlayers(void);
TPlayer *GetPlayer(uint32 CharacterID);
TPlayer *GetPlayer(const char *Name);
bool IsPlayerOnline(const char *Name);
int IdentifyPlayer(const char *Name, bool ExactMatch, bool IgnoreGamemasters, TPlayer **OutPlayer);
void LogoutAllPlayers(void);
void CloseProcessedRequests(uint32 CharacterID);
void NotifyBuddies(uint32 CharacterID, const char *Name, bool Login);
void CreatePlayerList(bool Online);
void PrintPlayerPositions(void);
void LoadDepot(TPlayerData *PlayerData, int DepotNr, Object Con);
void SaveDepot(TPlayerData *PlayerData, int DepotNr, Object Con);
void GetProfessionName(char *Buffer, int Profession, bool Article, bool Capitals);
void SendExistingRequests(TConnection *Connection);

void SavePlayerPoolSlot(TPlayerData *Slot);
void FreePlayerPoolSlot(TPlayerData *Slot);
TPlayerData *GetPlayerPoolSlot(uint32 CharacterID);
TPlayerData *AssignPlayerPoolSlot(uint32 CharacterID, bool DontWait);
TPlayerData *AttachPlayerPoolSlot(uint32 CharacterID, bool DontWait);
void AttachPlayerPoolSlot(TPlayerData *Slot, bool DontWait);
void IncreasePlayerPoolSlotSticky(TPlayerData *Slot);
void DecreasePlayerPoolSlotSticky(TPlayerData *Slot);
void DecreasePlayerPoolSlotSticky(uint32 CharacterID);
void ReleasePlayerPoolSlot(TPlayerData *Slot);
void SavePlayerPoolSlots(void);
void InitPlayerPool(void);
void ExitPlayerPool(void);

int GetPlayerIndexEntryNumber(const char *Name, int Position);
void InsertPlayerIndex(TPlayerIndexInternalNode *Node,
		int Position, const char *Name, uint32 CharacterID);
TPlayerIndexEntry *SearchPlayerIndex(const char *Name);
bool PlayerExists(const char *Name);
uint32 GetCharacterID(const char *Name);
const char *GetCharacterName(const char *Name);
void InitPlayerIndex(void);
void ExitPlayerIndex(void);

void InitPlayer(void);
void ExitPlayer(void);

// crskill.cc
// =============================================================================
void InitCrskill(void);
void ExitCrskill(void);

#endif //TIBIA_CREATURE_HH_
