#ifndef TIBIA_OBJECTS_HH_
#define TIBIA_OBJECTS_HH_ 1

#include "common.hh"
#include "enums.hh"

enum : int{
	TYPEID_MAP_CONTAINER		= 0,
	TYPEID_HEAD_CONTAINER		= 1,
	TYPEID_NECK_CONTAINER		= 2,
	TYPEID_BAG_CONTAINER		= 3,
	TYPEID_TORSO_CONTAINER		= 4,
	TYPEID_RIGHTHAND_CONTAINER	= 5,
	TYPEID_LEFTHAND_CONTAINER	= 6,
	TYPEID_LEGS_CONTAINER		= 7,
	TYPEID_FEET_CONTAINER		= 8,
	TYPEID_FINGER_CONTAINER		= 9,
	TYPEID_AMMO_CONTAINER		= 10,
	TYPEID_CREATURE_CONTAINER	= 99,
};

struct ObjectType {
	ObjectType(void) { this->setTypeID(0); }
	ObjectType(int TypeID) { this->setTypeID(TypeID); }
	void setTypeID(int TypeID);
	bool getFlag(FLAG Flag);
	uint32 getAttribute(TYPEATTRIBUTE Attribute);
	int getAttributeOffset(INSTANCEATTRIBUTE Attribute);
	const char *getName(int Count);
	const char *getDescription(void);

	bool isMapContainer(void){
		return this->TypeID == TYPEID_MAP_CONTAINER;
	}

	bool isBodyContainer(void){
		return this->TypeID == TYPEID_HEAD_CONTAINER
			|| this->TypeID == TYPEID_NECK_CONTAINER
			|| this->TypeID == TYPEID_BAG_CONTAINER
			|| this->TypeID == TYPEID_TORSO_CONTAINER
			|| this->TypeID == TYPEID_RIGHTHAND_CONTAINER
			|| this->TypeID == TYPEID_LEFTHAND_CONTAINER
			|| this->TypeID == TYPEID_LEGS_CONTAINER
			|| this->TypeID == TYPEID_FEET_CONTAINER
			|| this->TypeID == TYPEID_FINGER_CONTAINER
			|| this->TypeID == TYPEID_AMMO_CONTAINER;
	}

	bool isCreatureContainer(void){
		return this->TypeID == TYPEID_CREATURE_CONTAINER;
	}

	bool isTwoHanded(void){
		return this->getFlag(CLOTHES)
			&& this->getAttribute(BODYPOSITION) == 0;
	}

	bool isWeapon(void){
		return this->getFlag(WEAPON)
			|| this->getFlag(BOW)
			|| this->getFlag(THROW)
			|| this->getFlag(WAND);
	}

	bool isCloseWeapon(void){
		if(!this->getFlag(WEAPON)){
			return false;
		}

		int WeaponType = this->getAttribute(WEAPONTYPE);
		return WeaponType == WEAPON_SWORD
			|| WeaponType == WEAPON_CLUB
			|| WeaponType == WEAPON_AXE;
	}

	ObjectType getDisguise(void){
		if(this->getFlag(DISGUISE)){
			return (int)this->getAttribute(DISGUISETARGET);
		}else{
			return *this;
		}
	}

	bool operator==(const ObjectType &Other) const {
		return this->TypeID == Other.TypeID;
	}

	bool operator!=(const ObjectType &Other) const {
		return this->TypeID != Other.TypeID;
	}

	// DATA
	// =================
	int TypeID;
};

struct TObjectType {
	const char *Name;
	const char *Description;
	uint8 Flags[9];
	uint32 Attributes[62];
	int AttributeOffsets[18];
};

int GetFlagByName(const char *Name);
int GetTypeAttributeByName(const char *Name);
int GetInstanceAttributeByName(const char *Name);
const char *GetFlagName(int Flag);
const char *GetTypeAttributeName(int Attribute);
const char *GetInstanceAttributeName(int Attribute);
bool ObjectTypeExists(int TypeID);
bool ObjectTypeExists(uint8 Group, uint8 Number);
ObjectType GetNewObjectType(uint8 Group, uint8 Number);
void GetOldObjectType(ObjectType Type, uint8 *Group, uint8 *Number);
ObjectType GetSpecialObject(SPECIALMEANING Meaning);
ObjectType GetObjectTypeByName(const char *SearchName, bool Movable);
void InitObjects(void);
void ExitObjects(void);

#endif //TIBIA_OBJECTS_HH_
