#ifndef TIBIA_CONFIG_HH_
#define TIBIA_CONFIG_HH_ 1

#include "common.hh"
#include "enums.hh"

struct TDatabaseSettings {
    char Product[30];
    char Database[30];
    char Login[30];
    char Password[30];
    char Host[30];
    char Port[6];
};

struct TQueryManagerSettings {
    char Host[50];
    int Port;
    char Password[30];
};

extern char BINPATH[4096];
extern char DATAPATH[4096];
extern char LOGPATH[4096];
extern char MAPPATH[4096];
extern char MONSTERPATH[4096];
extern char NPCPATH[4096];
extern char ORIGMAPPATH[4096];
extern char SAVEPATH[4096];
extern char USERPATH[4096];
extern int SHMKey;
extern int AdminPort;
extern int GamePort;
extern int QueryManagerPort;
extern char AdminAddress[16];
extern char GameAddress[16];
extern char QueryManagerAddress[16];
extern char QueryManagerAdminPW[9];
extern char QueryManagerGamePW[9];
extern char QueryManagerWebPW[9];
extern int DebugLevel;
extern bool PrivateWorld;
extern TWorldType WorldType;
extern char WorldName[30];
extern int MaxPlayers;
extern int MaxNewbies;
extern int PremiumPlayerBuffer;
extern int PremiumNewbieBuffer;
extern int Beat;
extern int RebootTime;
extern TDatabaseSettings ADMIN_DATABASE;
extern TDatabaseSettings VOLATILE_DATABASE;
extern TDatabaseSettings WEB_DATABASE;
extern TDatabaseSettings FORUM_DATABASE;
extern TDatabaseSettings MANAGER_DATABASE;
extern int NumberOfQueryManagers;
extern TQueryManagerSettings QUERY_MANAGER[10];

// Database Engine Configuration (SQLite / MySQL / MariaDB)
extern char DB_Type[16];
extern char DB_File[256];
extern char MySQL_Host[128];
extern int  MySQL_Port;
extern char MySQL_User[64];
extern char MySQL_Password[64];
extern char MySQL_Database[64];
extern bool MySQL_Reconnect;

// Gameplay / Rates & Multipliers
extern int RateExp;
extern int RateSkill;
extern int RateMagic;
extern int RateLoot;
extern int RateSpawn;
extern int RateHPRegen;
extern int RateManaRegen;

// Gameplay / Item Consumption & QoL
extern bool LearnSpells;
extern bool RemoveChargesFromRunes;
extern bool RemoveChargesFromVials;
extern bool RemoveWeaponAmmunition;
extern bool RemoveWeaponCharges;
extern bool FreeCapacity;
extern bool AlwaysLight;
extern int  MaxSummonsPerPlayer;

// Gameplay / Combat & Timers
extern int AttackInterval;
extern int DefenseInterval;
extern int SpellExhaustion;
extern int RuneExhaustion;
extern int StairJumpExhaustion;

// Gameplay / PvP & Skulls
extern int ProtectionLevel;
extern int KillsToRedSkullDay;
extern int KillsToRedSkullWeek;
extern int KillsToRedSkullMonth;
extern int KillsToBanDay;
extern int KillsToBanWeek;
extern int KillsToBanMonth;
extern int WhiteSkullDuration;
extern int InFightDuration;
extern int RedSkullDuration;

// Gameplay / Death & Losses
extern int  DeathLosePercent;
extern bool ExperienceByKillingPlayers;
extern int  PVPEnforcedExpPercent;

// Gameplay / Soul Points
extern bool EnableSoulPoints;
extern int  SoulRegenInterval;
extern int  SoulRegenIntervalPromoted;

// Gameplay / Houses
extern int  HouseBuyLevel;
extern bool HouseOnlyPremium;

void ReadConfig(void);

#endif //TIBIA_CONFIG_HH_
