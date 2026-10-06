#ifndef TIBIA_MAGIC_HH_
#define TIBIA_MAGIC_HH_ 1

#include "common.hh"
#include "cr.hh"
#include "map.hh"

enum : int {
	FIELD_TYPE_FIRE = 1,
	FIELD_TYPE_POISON = 2,
	FIELD_TYPE_ENERGY = 3,
	FIELD_TYPE_MAGICWALL = 4,
	FIELD_TYPE_WILDGROWTH = 5,
};

struct TImpact{
	// VIRTUAL FUNCTIONS
	// =========================================================================
	virtual void handleField(int x, int y, int z);										// VTABLE[0]
	virtual void handleCreature(TCreature *Victim);										// VTABLE[1]
	virtual bool isAggressive(void);													// VTABLE[2]

	// NOTE(fusion): I don't think the original version had a destructor declared
	// here but the compiler complains when calling delete (which seems to only be
	// used in `TMonster::IdleStimulus`).
	virtual ~TImpact(void){
		// no-op
	}

	// DATA
	// =========================================================================
	//void *VTABLE;	// IMPLICIT
};

struct TDamageImpact: TImpact {
	TDamageImpact(TCreature *Actor, int DamageType, int Power, bool AllowDefense);
	void handleCreature(TCreature *Victim) override;

	TCreature *Actor;
	int DamageType;
	int Power;
	bool AllowDefense;
};

struct TFieldImpact: TImpact {
	TFieldImpact(TCreature *Actor, int FieldType);
	void handleField(int x, int y, int z) override;

	TCreature *Actor;
	int FieldType;
};

struct THealingImpact: TImpact {
	THealingImpact(TCreature *Actor, int Power);
	void handleCreature(TCreature *Victim) override;
	bool isAggressive(void) override;

	TCreature *Actor;
	int Power;
};

struct TSpeedImpact: TImpact {
	TSpeedImpact(TCreature *Actor, int Percent, int Duration);
	void handleCreature(TCreature *Victim) override;

	TCreature *Actor;
	int Percent;
	int Duration;
};

struct TDrunkenImpact: TImpact {
	TDrunkenImpact(TCreature *Actor, int Power, int Duration);
	void handleCreature(TCreature *Victim) override;

	TCreature *Actor;
	int Power;
	int Duration;
};

struct TStrengthImpact: TImpact {
	TStrengthImpact(TCreature *Actor, int Skills, int Percent, int Duration);
	void handleCreature(TCreature *Victim) override;

	TCreature *Actor;
	int Skills;
	int Percent;
	int Duration;
};

struct TOutfitImpact: TImpact {
	TOutfitImpact(TCreature *Actor, TOutfit Outfit, int Duration);
	void handleCreature(TCreature *Victim) override;

	TCreature *Actor;
	TOutfit Outfit;
	int Duration;
};

struct TSummonImpact: TImpact {
	TSummonImpact(TCreature *Actor, int Race, int Maximum);
	void handleField(int x, int y, int z) override;

	TCreature *Actor;
	int Race;
	int Maximum;
};

void ActorShapeSpell(TCreature *Actor, TImpact *Impact, int Effect);
void VictimShapeSpell(TCreature *Actor, TCreature *Victim,
		int Range, int Animation, TImpact *Impact, int Effect);
void OriginShapeSpell(TCreature *Actor, int Radius, TImpact *Impact, int Effect);
void CircleShapeSpell(TCreature *Actor, int DestX, int DestY, int DestZ,
		int Range, int Animation, int Radius, TImpact *Impact, int Effect);
void DestinationShapeSpell(TCreature *Actor, TCreature *Victim,
		int Range, int Animation, int Radius, TImpact *Impact, int Effect);
void AngleShapeSpell(TCreature *Actor, int Angle, int Range, TImpact *Impact, int Effect);
void CheckSpellbook(TCreature *Actor, int SpellNr);
void CheckAccount(TCreature *Actor, int SpellNr);
void CheckLevel(TCreature *Actor, int SpellNr);
void CheckRuneLevel(TCreature *Actor, int SpellNr);
void CheckMagicItem(TCreature *Actor, ObjectType Type);
void CheckRing(TCreature *Actor, int SpellNr);
void CheckAffectedPlayers(TCreature *Actor, int x, int y, int z);
void CheckMana(TCreature *Actor, int ManaPoints, int SoulPoints, int Delay);
int ComputeDamage(TCreature *Actor, int SpellNr, int Damage, int Variation);
bool IsAggressiveSpell(int SpellNr);
void MassCombat(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints,
		int Damage, int Effect, int Radius, int DamageType, int Animation);
void AngleCombat(TCreature *Actor, int ManaPoints, int SoulPoints,
		int Damage, int Effect, int Range, int Angle, int DamageType);
void Combat(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints,
		int Damage, int Effect, int Animation, int DamageType);
int GetDirection(int dx, int dy); // TODO(fusion): Move this one elsewhere? Maybe `info.cc`.
void KillAllMonsters(TCreature *Actor, int Effect, int Radius);
void CreateField(int x, int y, int z, int FieldType, uint32 Owner, bool Peaceful);
void CreateField(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints, int FieldType);
void CreateField(TCreature *Actor, int ManaPoints, int SoulPoints, int FieldType);
void MassCreateField(TCreature *Actor, Object Target,
		int ManaPoints, int SoulPoints, int FieldType, int Radius);
void CreateFieldWall(TCreature *Actor, Object Target,
		int ManaPoints, int SoulPoints, int FieldType, int Width);
void DeleteField(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints);
void CleanupField(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints);
void CleanupField(TCreature *Actor);
void Teleport(TCreature *Actor, const char *Param);
void TeleportToCreature(TCreature *Actor, const char *Name);
void TeleportPlayerToMe(TCreature *Actor, const char *Name);
void MagicRope(TCreature *Actor, int ManaPoints, int SoulPoints);
void MagicClimbing(TCreature *Actor, int ManaPoints, int SoulPoints, const char *Param);
void MagicClimbing(TCreature *Actor, const char *Param);
void CreateThing(TCreature *Actor, const char *Param1, const char *Param2);
void CreateMoney(TCreature *Actor, const char *Param);
void CreateFood(TCreature *Actor, int ManaPoints, int SoulPoints);
void CreateArrows(TCreature *Actor, int ManaPoints, int SoulPoints, int ArrowType, int Count);
void SummonCreature(TCreature *Actor, int ManaPoints, int Race, bool God);
void SummonCreature(TCreature *Actor, int ManaPoints, const char *RaceName, bool God);
void StartMonsterraid(TCreature *Actor, const char *RaidName);
void RaiseDead(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints);
void MassRaiseDead(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints, int Radius);
void Heal(TCreature *Actor, int ManaPoints, int SoulPoints, int Amount);
void MassHeal(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints, int Amount, int Radius);
void HealFriend(TCreature *Actor, const char *TargetName, int ManaPoints, int SoulPoints, int Amount);
void RefreshMana(TCreature *Actor, int ManaPoints, int SoulPoints, int Amount);
void MagicGoStrength(TCreature *Actor, TCreature *Target, int ManaPoints, int SoulPoints, int Percent, int Duration);
void Shielding(TCreature *Actor, int ManaPoints, int SoulPoints, int Duration);
void NegatePoison(TCreature *Actor, TCreature *Target, int ManaPoints, int SoulPoints);
void Enlight(TCreature *Actor, int ManaPoints, int SoulPoints, int Radius, int Duration);
void Invisibility(TCreature *Actor, int ManaPoints, int SoulPoints, int Duration);
void CancelInvisibility(TCreature *Actor, Object Target, int ManaPoints, int SoulPoints, int Radius);
void CreatureIllusion(TCreature *Actor, int ManaPoints, int SoulPoints, const char *RaceName, int Duration);
void ObjectIllusion(TCreature *Actor, int ManaPoints, int SoulPoints, Object Target, int Duration);
void ChangeData(TCreature *Actor, const char *Param);
void EnchantObject(TCreature *Actor, int ManaPoints, int SoulPoints, ObjectType OldType, ObjectType NewType);
void Convince(TCreature *Actor, TCreature *Target);
void Challenge(TCreature *Actor, int ManaPoints, int SoulPoints, int Radius);
void FindPerson(TCreature *Actor, int ManaPoints, int SoulPoints, const char *TargetName);
void GetPosition(TCreature *Actor);
void GetQuestValue(TCreature *Actor, const char *Param);
void SetQuestValue(TCreature *Actor, const char *Param1, const char *Param2);
void ClearQuestValues(TCreature *Actor);
void CreateKnowledge(TCreature *Actor, const char *Param1, const char *Param2);
void ChangeProfession(TCreature *Actor, const char *Param);
void EditGuests(TCreature *Actor);
void EditSubowners(TCreature *Actor);
void EditNameDoor(TCreature *Actor);
void KickGuest(TCreature *Actor, const char *GuestName);
void Notation(TCreature *Actor, const char *Name, const char *Comment);
void NameLock(TCreature *Actor, const char *Name);
void BanishAccount(TCreature *Actor, const char *Name, int Duration, const char *Reason);
void DeleteAccount(TCreature *Actor, const char *Name, const char *Reason);
void BanishCharacter(TCreature *Actor, const char *Name, int Duration, const char *Reason);
void DeleteCharacter(TCreature *Actor, const char *Name, const char *Reason);
void IPBanishment(TCreature *Actor, const char *Name, const char *Reason);
void SetNameRule(TCreature *Actor, const char *Name);
void KickPlayer(TCreature *Actor, const char *Name);
void HomeTeleport(TCreature *Actor, const char *Name);

// TODO(fusion): These are unsafe like strcpy.
void GetMagicItemDescription(Object Obj, char *SpellString, int *MagicLevel);
void GetSpellbook(uint32 CharacterID, char *Buffer);

int GetSpellLevel(int SpellNr);

int CheckForSpell(uint32 CreatureID, const char *Text);
void UseMagicItem(uint32 CreatureID, Object Obj, Object Dest);
void DrinkPotion(uint32 CreatureID, Object Obj);

void InitMagic(void);
void ExitMagic(void);

#endif //TIBIA_MAGIC_HH_
