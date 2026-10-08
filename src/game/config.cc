#include "config.hh"
#include "script.hh"
#include <filesystem>
#include <cstring>

static void NormalizeDataPath(char *Path, const char *SubDir){
	namespace fs = std::filesystem;
	if(Path[0] != 0){
		fs::path direct(Path);
		if(fs::exists(direct) && fs::is_directory(direct)){
			return;
		}
		fs::path sub = direct / SubDir;
		if(fs::exists(sub) && fs::is_directory(sub)){
			std::string s = sub.string();
			strncpy(Path, s.c_str(), 4096);
			return;
		}
	}
	const char *candidates[] = {
		"../CipSoft Server/tibia-game_data",
		"../../CipSoft Server/tibia-game_data",
		"../tibia-game_data",
		"tibia-game_data",
		"../../tibia-game_data",
		"../data",
		"data",
		"."
	};
	for(const char *cand : candidates){
		fs::path p;
		if(strcmp(cand, ".") == 0){
			p = fs::path(SubDir);
		}else{
			p = fs::path(cand) / SubDir;
		}
		if(fs::exists(p) && fs::is_directory(p)){
			std::string s = p.string();
			strncpy(Path, s.c_str(), 4096);
			return;
		}
	}
}

char BINPATH[4096];
char DATAPATH[4096];
char LOGPATH[4096];
char MAPPATH[4096];
char MONSTERPATH[4096];
char NPCPATH[4096];
char ORIGMAPPATH[4096];
char SAVEPATH[4096];
char USERPATH[4096];

int SHMKey;
int AdminPort;
int GamePort;
int QueryManagerPort;

char AdminAddress[16];
char GameAddress[16];
char QueryManagerAddress[16];
char QueryManagerAdminPW[9];
char QueryManagerGamePW[9];
char QueryManagerWebPW[9];

int DebugLevel;
bool PrivateWorld;
TWorldType WorldType;
char WorldName[30];
int MaxPlayers;
int MaxNewbies;
int PremiumPlayerBuffer;
int PremiumNewbieBuffer;
int Beat;
int RebootTime;

TDatabaseSettings ADMIN_DATABASE;
TDatabaseSettings VOLATILE_DATABASE;
TDatabaseSettings WEB_DATABASE;
TDatabaseSettings FORUM_DATABASE;
TDatabaseSettings MANAGER_DATABASE;

int NumberOfQueryManagers;
TQueryManagerSettings QUERY_MANAGER[10];

// Database Engine Configuration (SQLite / MySQL / MariaDB)
char DB_Type[16];
char DB_File[256];
char MySQL_Host[128];
int  MySQL_Port;
char MySQL_User[64];
char MySQL_Password[64];
char MySQL_Database[64];
bool MySQL_Reconnect;

// Gameplay / Rates & Multipliers
int RateExp;
int RateSkill;
int RateMagic;
int RateLoot;
int RateSpawn;
int RateHPRegen;
int RateManaRegen;

// Gameplay / Item Consumption & QoL
bool RemoveChargesFromRunes;
bool RemoveChargesFromVials;
bool RemoveWeaponAmmunition;
bool RemoveWeaponCharges;
bool FreeCapacity;
bool AlwaysLight;
int  MaxSummonsPerPlayer;

// Gameplay / Combat & Timers
int AttackInterval;
int DefenseInterval;
int SpellExhaustion;
int RuneExhaustion;
int StairJumpExhaustion;

// Gameplay / PvP & Skulls
int ProtectionLevel;
int KillsToRedSkullDay;
int KillsToRedSkullWeek;
int KillsToRedSkullMonth;
int KillsToBanDay;
int KillsToBanWeek;
int KillsToBanMonth;
int WhiteSkullDuration;
int InFightDuration;
int RedSkullDuration;

// Gameplay / Death & Losses
int  DeathLosePercent;
bool ExperienceByKillingPlayers;
int  PVPEnforcedExpPercent;

// Gameplay / Soul Points
bool EnableSoulPoints;
int  SoulRegenInterval;
int  SoulRegenIntervalPromoted;

// Gameplay / Houses
int  HouseBuyLevel;
bool HouseOnlyPremium;

static char PasswordKey[9] = "Pm-,o%yD";

static void DisguisePassword(char *Password, char *Key){
	if(Password == NULL){
		throw "password is null";
	}

	if(Key == NULL){
		throw "key for disguising password is null";
	}

	usize KeyLen = strlen(Key);
	usize PasswordLen = strlen(Password);
	if(KeyLen < PasswordLen){
		throw "key for disguising password is too short";
	}

	for(usize Index = 0; Index < PasswordLen; Index += 1){
		Password[Index] = (Key[Index] - Password[Index] + 0x5E) % 0x5E + 0x21;
	}
}

void ReadConfig(void){
	PrivateWorld = false;
	strncpy(GameAddress, "0.0.0.0", 8);
	SHMKey = 0;
	QueryManagerPort = 0;
	strncpy(QueryManagerAddress, "0.0.0.0", 8);
	AdminPort = 0;
	GamePort = 0;
	strncpy(AdminAddress, "0.0.0.0", 8);
	MaxPlayers = 0;
	DebugLevel = 1;
	WorldType = NORMAL;
	BINPATH[0] = 0;
	DATAPATH[0] = 0;
	LOGPATH[0] = 0;
	MAPPATH[0] = 0;
	MONSTERPATH[0] = 0;
	NPCPATH[0] = 0;
	ORIGMAPPATH[0] = 0;
	SAVEPATH[0] = 0;
	USERPATH[0] = 0;
	QueryManagerAdminPW[0] = 0;
	QueryManagerGamePW[0] = 0;
	QueryManagerWebPW[0] = 0;
	MaxNewbies = 0;
	PremiumPlayerBuffer = 0;
	PremiumNewbieBuffer = 0;
	NumberOfQueryManagers = 0;
	Beat = 200;
	RebootTime = 540;
	ADMIN_DATABASE.Database[0] = 0;
	VOLATILE_DATABASE.Database[0] = 0;
	WEB_DATABASE.Database[0] = 0;
	FORUM_DATABASE.Database[0] = 0;
	MANAGER_DATABASE.Database[0] = 0;

	// Default Database Settings
	strncpy(DB_Type, "sqlite", sizeof(DB_Type) - 1);
	strncpy(DB_File, "tibia.db", sizeof(DB_File) - 1);
	strncpy(MySQL_Host, "127.0.0.1", sizeof(MySQL_Host) - 1);
	MySQL_Port = 3306;
	strncpy(MySQL_User, "root", sizeof(MySQL_User) - 1);
	MySQL_Password[0] = 0;
	strncpy(MySQL_Database, "tibia", sizeof(MySQL_Database) - 1);
	MySQL_Reconnect = true;

	// Default Gameplay Settings
	RateExp = 1;
	RateSkill = 1;
	RateMagic = 1;
	RateLoot = 1;
	RateSpawn = 1;
	RateHPRegen = 1;
	RateManaRegen = 1;

	RemoveChargesFromRunes = true;
	RemoveChargesFromVials = true;
	RemoveWeaponAmmunition = true;
	RemoveWeaponCharges = true;
	FreeCapacity = false;
	AlwaysLight = false;
	MaxSummonsPerPlayer = 2;

	AttackInterval = 2000;
	DefenseInterval = 2000;
	SpellExhaustion = 1000;
	RuneExhaustion = 2000;
	StairJumpExhaustion = 2000;

	ProtectionLevel = 1;
	KillsToRedSkullDay = 3;
	KillsToRedSkullWeek = 5;
	KillsToRedSkullMonth = 10;
	KillsToBanDay = 6;
	KillsToBanWeek = 10;
	KillsToBanMonth = 20;
	WhiteSkullDuration = 900;
	InFightDuration = 60;
	RedSkullDuration = 2592000;

	DeathLosePercent = 10;
	ExperienceByKillingPlayers = false;
	PVPEnforcedExpPercent = 5;

	EnableSoulPoints = true;
	SoulRegenInterval = 120;
	SoulRegenIntervalPromoted = 15;

	HouseBuyLevel = 1;
	HouseOnlyPremium = true;

	char FileName[4096] = {0};
	const char *configCandidates[] = {
		".tibia",
		"../tibia-game_data/.tibia",
		"tibia-game_data/.tibia",
		"../../tibia-game_data/.tibia",
		"config.cfg"
	};
	for(const char *cand : configCandidates){
		if(FileExists(cand)){
			strncpy(FileName, cand, sizeof(FileName));
			break;
		}
	}

	if(FileName[0] != 0 && FileExists(FileName)){
		TReadScriptFile Script;
		Script.open(FileName);
		while(true){
			Script.nextToken();
			if(Script.Token == ENDOFFILE){
				Script.close();
				break;
			}

			char Identifier[MAX_IDENT_LENGTH];
			strncpy(Identifier, Script.getIdentifier(), sizeof(Identifier) - 1);
			Identifier[sizeof(Identifier) - 1] = 0;
			Script.readSymbol('=');

		if(strcmp(Identifier, "binpath") == 0 || strcmp(Identifier, "bin_path") == 0){
			strncpy(BINPATH, Script.readString(), sizeof(BINPATH) - 1);
		}else if(strcmp(Identifier, "mappath") == 0 || strcmp(Identifier, "map_path") == 0){
			strncpy(MAPPATH, Script.readString(), sizeof(MAPPATH) - 1);
		}else if(strcmp(Identifier, "origmappath") == 0 || strcmp(Identifier, "orig_map_path") == 0){
			strncpy(ORIGMAPPATH, Script.readString(), sizeof(ORIGMAPPATH) - 1);
		}else if(strcmp(Identifier, "datapath") == 0 || strcmp(Identifier, "data_path") == 0){
			strncpy(DATAPATH, Script.readString(), sizeof(DATAPATH) - 1);
		}else if(strcmp(Identifier, "monsterpath") == 0 || strcmp(Identifier, "monster_path") == 0){
			strncpy(MONSTERPATH, Script.readString(), sizeof(MONSTERPATH) - 1);
		}else if(strcmp(Identifier, "npcpath") == 0 || strcmp(Identifier, "npc_path") == 0){
			strncpy(NPCPATH, Script.readString(), sizeof(NPCPATH) - 1);
		}else if(strcmp(Identifier, "userpath") == 0 || strcmp(Identifier, "user_path") == 0){
			strncpy(USERPATH, Script.readString(), sizeof(USERPATH) - 1);
		}else if(strcmp(Identifier, "logpath") == 0 || strcmp(Identifier, "log_path") == 0){
			strncpy(LOGPATH, Script.readString(), sizeof(LOGPATH) - 1);
		}else if(strcmp(Identifier, "savepath") == 0 || strcmp(Identifier, "save_path") == 0){
			strncpy(SAVEPATH, Script.readString(), sizeof(SAVEPATH) - 1);
		}else if(strcmp(Identifier, "shm") == 0){
			SHMKey = Script.readNumber();
		}else if(strcmp(Identifier, "gameport") == 0 || strcmp(Identifier, "game_port") == 0){
			GamePort = Script.readNumber();
		}else if(strcmp(Identifier, "loginport") == 0 || strcmp(Identifier, "login_port") == 0){
			Script.readNumber();
		}else if(strcmp(Identifier, "adminport") == 0 || strcmp(Identifier, "admin_port") == 0){
			AdminPort = Script.readNumber();
		}else if(strcmp(Identifier, "adminaddress") == 0 || strcmp(Identifier, "admin_address") == 0){
			strncpy(AdminAddress, Script.readString(), sizeof(AdminAddress) - 1);
		}else if(strcmp(Identifier, "bindaddress") == 0 || strcmp(Identifier, "bind_address") == 0){
			strncpy(GameAddress, Script.readString(), sizeof(GameAddress) - 1);
		}else if(strcmp(Identifier, "worldaddress") == 0 || strcmp(Identifier, "world_address") == 0){
			Script.readString();
		}else if(strcmp(Identifier, "maxplayers") == 0 || strcmp(Identifier, "max_players") == 0){
			MaxPlayers = Script.readNumber();
		}else if(strcmp(Identifier, "maxconnections") == 0 || strcmp(Identifier, "max_connections") == 0){
			Script.readNumber();
		}else if(strcmp(Identifier, "connectiontimeout") == 0 || strcmp(Identifier, "connection_timeout") == 0){
			Script.readNumber();
		}else if(strcmp(Identifier, "db_type") == 0 || strcmp(Identifier, "dbtype") == 0 || strcmp(Identifier, "sql_type") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(DB_Type, val, sizeof(DB_Type) - 1);
				DB_Type[sizeof(DB_Type) - 1] = 0;
			}
		}else if(strcmp(Identifier, "databasefile") == 0 || strcmp(Identifier, "database_file") == 0 || strcmp(Identifier, "db_file") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(DB_File, val, sizeof(DB_File) - 1);
				DB_File[sizeof(DB_File) - 1] = 0;
			}
		}else if(strcmp(Identifier, "mysql_host") == 0 || strcmp(Identifier, "mysqlhost") == 0 || strcmp(Identifier, "sql_host") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(MySQL_Host, val, sizeof(MySQL_Host) - 1);
				MySQL_Host[sizeof(MySQL_Host) - 1] = 0;
			}
		}else if(strcmp(Identifier, "mysql_port") == 0 || strcmp(Identifier, "mysqlport") == 0 || strcmp(Identifier, "sql_port") == 0){
			MySQL_Port = Script.readNumber();
		}else if(strcmp(Identifier, "mysql_user") == 0 || strcmp(Identifier, "mysqluser") == 0 || strcmp(Identifier, "sql_user") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(MySQL_User, val, sizeof(MySQL_User) - 1);
				MySQL_User[sizeof(MySQL_User) - 1] = 0;
			}
		}else if(strcmp(Identifier, "mysql_password") == 0 || strcmp(Identifier, "mysql_pass") == 0 || strcmp(Identifier, "mysqlpassword") == 0 || strcmp(Identifier, "sql_password") == 0 || strcmp(Identifier, "sql_pass") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(MySQL_Password, val, sizeof(MySQL_Password) - 1);
				MySQL_Password[sizeof(MySQL_Password) - 1] = 0;
			}
		}else if(strcmp(Identifier, "mysql_database") == 0 || strcmp(Identifier, "mysql_db") == 0 || strcmp(Identifier, "mysqldatabase") == 0 || strcmp(Identifier, "sql_db") == 0 || strcmp(Identifier, "sql_database") == 0){
			const char *val = Script.readString();
			if(val){
				strncpy(MySQL_Database, val, sizeof(MySQL_Database) - 1);
				MySQL_Database[sizeof(MySQL_Database) - 1] = 0;
			}
		}else if(strcmp(Identifier, "mysql_reconnect") == 0 || strcmp(Identifier, "sql_reconnect") == 0){
			const char *val = Script.readIdentifier();
			MySQL_Reconnect = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "rate_exp") == 0 || strcmp(Identifier, "rateexp") == 0){
			RateExp = Script.readNumber();
		}else if(strcmp(Identifier, "rate_skill") == 0 || strcmp(Identifier, "rateskill") == 0){
			RateSkill = Script.readNumber();
		}else if(strcmp(Identifier, "rate_magic") == 0 || strcmp(Identifier, "ratemagic") == 0){
			RateMagic = Script.readNumber();
		}else if(strcmp(Identifier, "rate_loot") == 0 || strcmp(Identifier, "rateloot") == 0){
			RateLoot = Script.readNumber();
		}else if(strcmp(Identifier, "rate_spawn") == 0 || strcmp(Identifier, "ratespawn") == 0){
			RateSpawn = Script.readNumber();
		}else if(strcmp(Identifier, "rate_hp_regen") == 0 || strcmp(Identifier, "ratehpregen") == 0){
			RateHPRegen = Script.readNumber();
		}else if(strcmp(Identifier, "rate_mana_regen") == 0 || strcmp(Identifier, "ratemanaregen") == 0){
			RateManaRegen = Script.readNumber();
		}else if(strcmp(Identifier, "remove_charges_from_runes") == 0 || strcmp(Identifier, "removechargesfromrunes") == 0){
			const char *val = Script.readIdentifier();
			RemoveChargesFromRunes = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "remove_charges_from_vials") == 0 || strcmp(Identifier, "removechargesfromvials") == 0 || strcmp(Identifier, "remove_charges_from_potions") == 0 || strcmp(Identifier, "removechargesfrompotions") == 0){
			const char *val = Script.readIdentifier();
			RemoveChargesFromVials = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "remove_weapon_ammunition") == 0 || strcmp(Identifier, "removeweaponammunition") == 0){
			const char *val = Script.readIdentifier();
			RemoveWeaponAmmunition = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "remove_weapon_charges") == 0 || strcmp(Identifier, "removeweaponcharges") == 0){
			const char *val = Script.readIdentifier();
			RemoveWeaponCharges = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "free_capacity") == 0 || strcmp(Identifier, "freecapacity") == 0){
			const char *val = Script.readIdentifier();
			FreeCapacity = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "always_light") == 0 || strcmp(Identifier, "alwayslight") == 0 || strcmp(Identifier, "ambient_light") == 0){
			const char *val = Script.readIdentifier();
			AlwaysLight = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "max_summons_per_player") == 0 || strcmp(Identifier, "maxsummonsperplayer") == 0){
			MaxSummonsPerPlayer = Script.readNumber();
		}else if(strcmp(Identifier, "attack_interval") == 0 || strcmp(Identifier, "attackinterval") == 0 || strcmp(Identifier, "attack_speed") == 0){
			AttackInterval = Script.readNumber();
		}else if(strcmp(Identifier, "defense_interval") == 0 || strcmp(Identifier, "defenseinterval") == 0){
			DefenseInterval = Script.readNumber();
		}else if(strcmp(Identifier, "spell_exhaustion") == 0 || strcmp(Identifier, "spellexhaustion") == 0){
			SpellExhaustion = Script.readNumber();
		}else if(strcmp(Identifier, "rune_exhaustion") == 0 || strcmp(Identifier, "runeexhaustion") == 0){
			RuneExhaustion = Script.readNumber();
		}else if(strcmp(Identifier, "stair_jump_exhaustion") == 0 || strcmp(Identifier, "stairjumpexhaustion") == 0){
			StairJumpExhaustion = Script.readNumber();
		}else if(strcmp(Identifier, "protection_level") == 0 || strcmp(Identifier, "protectionlevel") == 0){
			ProtectionLevel = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_red_skull") == 0 || strcmp(Identifier, "killstoredskull") == 0 || strcmp(Identifier, "kills_to_red_skull_day") == 0){
			KillsToRedSkullDay = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_red_skull_week") == 0){
			KillsToRedSkullWeek = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_red_skull_month") == 0){
			KillsToRedSkullMonth = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_ban") == 0 || strcmp(Identifier, "killstoban") == 0 || strcmp(Identifier, "kills_to_ban_day") == 0 || strcmp(Identifier, "kills_to_black_skull") == 0){
			KillsToBanDay = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_ban_week") == 0){
			KillsToBanWeek = Script.readNumber();
		}else if(strcmp(Identifier, "kills_to_ban_month") == 0){
			KillsToBanMonth = Script.readNumber();
		}else if(strcmp(Identifier, "white_skull_time") == 0 || strcmp(Identifier, "whiteskulltime") == 0 || strcmp(Identifier, "white_skull_duration") == 0){
			WhiteSkullDuration = Script.readNumber();
		}else if(strcmp(Identifier, "in_fight_time") == 0 || strcmp(Identifier, "infighttime") == 0 || strcmp(Identifier, "pz_locked") == 0 || strcmp(Identifier, "pzlocked") == 0){
			InFightDuration = Script.readNumber();
			if(InFightDuration > 1000) InFightDuration /= 1000;
		}else if(strcmp(Identifier, "red_skull_duration") == 0 || strcmp(Identifier, "redskullduration") == 0){
			RedSkullDuration = Script.readNumber();
		}else if(strcmp(Identifier, "death_lose_percent") == 0 || strcmp(Identifier, "deathlosepercent") == 0){
			DeathLosePercent = Script.readNumber();
		}else if(strcmp(Identifier, "experience_by_killing_players") == 0 || strcmp(Identifier, "experiencebykillingplayers") == 0){
			const char *val = Script.readIdentifier();
			ExperienceByKillingPlayers = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "pvp_enforced_exp_percent") == 0 || strcmp(Identifier, "pvpenforcedexppercent") == 0){
			PVPEnforcedExpPercent = Script.readNumber();
		}else if(strcmp(Identifier, "enable_soul_points") == 0 || strcmp(Identifier, "enablesoulpoints") == 0 || strcmp(Identifier, "stamina_system") == 0){
			const char *val = Script.readIdentifier();
			EnableSoulPoints = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "soul_regen_interval") == 0 || strcmp(Identifier, "soulregeninterval") == 0){
			SoulRegenInterval = Script.readNumber();
		}else if(strcmp(Identifier, "soul_regen_interval_promoted") == 0 || strcmp(Identifier, "soulregenintervalpromoted") == 0){
			SoulRegenIntervalPromoted = Script.readNumber();
		}else if(strcmp(Identifier, "house_buy_level") == 0 || strcmp(Identifier, "housebuylevel") == 0){
			HouseBuyLevel = Script.readNumber();
		}else if(strcmp(Identifier, "house_only_premium") == 0 || strcmp(Identifier, "houseonlypremium") == 0){
			const char *val = Script.readIdentifier();
			HouseOnlyPremium = (val && (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0));
		}else if(strcmp(Identifier, "world_type") == 0 || strcmp(Identifier, "worldtype") == 0){
			const char *val = Script.readString();
			if(val && val[0] != 0){
				if(strcmp(val, "pvp") == 0 || strcmp(val, "normal") == 0 || strcmp(val, "open-pvp") == 0){
					WorldType = NORMAL;
				}else if(strcmp(val, "no-pvp") == 0 || strcmp(val, "non-pvp") == 0 || strcmp(val, "optional-pvp") == 0){
					WorldType = NON_PVP;
				}else if(strcmp(val, "pvp-enforced") == 0 || strcmp(val, "hardcore-pvp") == 0){
					WorldType = PVP_ENFORCED;
				}
			}
		}else if(strcmp(Identifier, "rsakeyfile") == 0 || strcmp(Identifier, "rsa_key_file") == 0){
			Script.readString();
		}else if(strcmp(Identifier, "motd") == 0){
			Script.readString();
		}else if(strcmp(Identifier, "querymanagerport") == 0){
			QueryManagerPort = Script.readNumber();
		}else if(strcmp(Identifier, "querymanageraddress") == 0){
			strncpy(QueryManagerAddress, Script.readString(), sizeof(QueryManagerAddress) - 1);
		}else if(strcmp(Identifier, "querymanageradminpw") == 0){
			strncpy(QueryManagerAdminPW, Script.readString(), sizeof(QueryManagerAdminPW) - 1);
		}else if(strcmp(Identifier, "querymanagergamepw") == 0){
			strncpy(QueryManagerGamePW, Script.readString(), sizeof(QueryManagerGamePW) - 1);
		}else if(strcmp(Identifier, "querymanagerwebpw") == 0){
			strncpy(QueryManagerWebPW, Script.readString(), sizeof(QueryManagerWebPW) - 1);
		}else if(strcmp(Identifier, "debuglevel") == 0){
			DebugLevel = Script.readNumber();
		}else if(strcmp(Identifier, "state") == 0){
			PrivateWorld = (strcmp(Script.readIdentifier(), "private") == 0);
		}else if(strcmp(Identifier, "world") == 0 || strcmp(Identifier, "world_name") == 0 || strcmp(Identifier, "worldname") == 0){
			strncpy(WorldName, Script.readString(), sizeof(WorldName) - 1);
		}else if(strcmp(Identifier, "beat") == 0){
			Beat = Script.readNumber();
		}else if(strcmp(Identifier, "admindatabase") == 0){
			Script.readSymbol('(');
			strncpy(ADMIN_DATABASE.Product, Script.readIdentifier(), sizeof(ADMIN_DATABASE.Product) - 1);
			Script.readSymbol(',');
			strncpy(ADMIN_DATABASE.Database, Script.readString(), sizeof(ADMIN_DATABASE.Database) - 1);
			Script.readSymbol(',');
			strncpy(ADMIN_DATABASE.Login, Script.readString(), sizeof(ADMIN_DATABASE.Login) - 1);
			Script.readSymbol(',');
			strncpy(ADMIN_DATABASE.Password, Script.readString(), sizeof(ADMIN_DATABASE.Password) - 1);
			DisguisePassword(ADMIN_DATABASE.Password, PasswordKey);
			Script.readSymbol(',');
			strncpy(ADMIN_DATABASE.Host, Script.readString(), sizeof(ADMIN_DATABASE.Host) - 1);
			Script.readSymbol(',');
			strncpy(ADMIN_DATABASE.Port, Script.readString(), sizeof(ADMIN_DATABASE.Port) - 1);
			Script.readSymbol(')');
		}else if(strcmp(Identifier, "volatiledatabase") == 0){
			Script.readSymbol('(');
			strncpy(VOLATILE_DATABASE.Product, Script.readIdentifier(), sizeof(VOLATILE_DATABASE.Product) - 1);
			Script.readSymbol(',');
			strncpy(VOLATILE_DATABASE.Database, Script.readString(), sizeof(VOLATILE_DATABASE.Database) - 1);
			Script.readSymbol(',');
			strncpy(VOLATILE_DATABASE.Login, Script.readString(), sizeof(VOLATILE_DATABASE.Login) - 1);
			Script.readSymbol(',');
			strncpy(VOLATILE_DATABASE.Password, Script.readString(), sizeof(VOLATILE_DATABASE.Password) - 1);
			DisguisePassword(VOLATILE_DATABASE.Password, PasswordKey);
			Script.readSymbol(',');
			strncpy(VOLATILE_DATABASE.Host, Script.readString(), sizeof(VOLATILE_DATABASE.Host) - 1);
			Script.readSymbol(',');
			strncpy(VOLATILE_DATABASE.Port, Script.readString(), sizeof(VOLATILE_DATABASE.Port) - 1);
			Script.readSymbol(')');
		}else if(strcmp(Identifier, "webdatabase") == 0){
			Script.readSymbol('(');
			strncpy(WEB_DATABASE.Product, Script.readIdentifier(), sizeof(WEB_DATABASE.Product) - 1);
			Script.readSymbol(',');
			strncpy(WEB_DATABASE.Database, Script.readString(), sizeof(WEB_DATABASE.Database) - 1);
			Script.readSymbol(',');
			strncpy(WEB_DATABASE.Login, Script.readString(), sizeof(WEB_DATABASE.Login) - 1);
			Script.readSymbol(',');
			strncpy(WEB_DATABASE.Password, Script.readString(), sizeof(WEB_DATABASE.Password) - 1);
			DisguisePassword(WEB_DATABASE.Password, PasswordKey);
			Script.readSymbol(',');
			strncpy(WEB_DATABASE.Host, Script.readString(), sizeof(WEB_DATABASE.Host) - 1);
			Script.readSymbol(',');
			strncpy(WEB_DATABASE.Port, Script.readString(), sizeof(WEB_DATABASE.Port) - 1);
			Script.readSymbol(')');
		}else if(strcmp(Identifier, "forumdatabase") == 0){
			Script.readSymbol('(');
			strncpy(FORUM_DATABASE.Product, Script.readIdentifier(), sizeof(FORUM_DATABASE.Product) - 1);
			Script.readSymbol(',');
			strncpy(FORUM_DATABASE.Database, Script.readString(), sizeof(FORUM_DATABASE.Database) - 1);
			Script.readSymbol(',');
			strncpy(FORUM_DATABASE.Login, Script.readString(), sizeof(FORUM_DATABASE.Login) - 1);
			Script.readSymbol(',');
			strncpy(FORUM_DATABASE.Password, Script.readString(), sizeof(FORUM_DATABASE.Password) - 1);
			DisguisePassword(FORUM_DATABASE.Password, PasswordKey);
			Script.readSymbol(',');
			strncpy(FORUM_DATABASE.Host, Script.readString(), sizeof(FORUM_DATABASE.Host) - 1);
			Script.readSymbol(',');
			strncpy(FORUM_DATABASE.Port, Script.readString(), sizeof(FORUM_DATABASE.Port) - 1);
			Script.readSymbol(')');
		}else if(strcmp(Identifier, "managerdatabase") == 0){
			Script.readSymbol('(');
			strncpy(MANAGER_DATABASE.Product, Script.readIdentifier(), sizeof(MANAGER_DATABASE.Product) - 1);
			Script.readSymbol(',');
			strncpy(MANAGER_DATABASE.Database, Script.readString(), sizeof(MANAGER_DATABASE.Database) - 1);
			Script.readSymbol(',');
			strncpy(MANAGER_DATABASE.Login, Script.readString(), sizeof(MANAGER_DATABASE.Login) - 1);
			Script.readSymbol(',');
			strncpy(MANAGER_DATABASE.Password, Script.readString(), sizeof(MANAGER_DATABASE.Password) - 1);
			DisguisePassword(MANAGER_DATABASE.Password, PasswordKey);
			Script.readSymbol(',');
			strncpy(MANAGER_DATABASE.Host, Script.readString(), sizeof(MANAGER_DATABASE.Host) - 1);
			Script.readSymbol(',');
			strncpy(MANAGER_DATABASE.Port, Script.readString(), sizeof(MANAGER_DATABASE.Port) - 1);
			Script.readSymbol(')');
		}else if(strcmp(Identifier, "querymanager") == 0){
			Script.readSymbol('{');
			do{
				if(NumberOfQueryManagers >= NARRAY(QUERY_MANAGER)){
					Script.error("Cannot handle more query managers");
				}
				Script.readSymbol('(');
				strncpy(QUERY_MANAGER[NumberOfQueryManagers].Host, Script.readString(), sizeof(QUERY_MANAGER[NumberOfQueryManagers].Host) - 1);
				Script.readSymbol(',');
				QUERY_MANAGER[NumberOfQueryManagers].Port = Script.readNumber();
				Script.readSymbol(',');
				strncpy(QUERY_MANAGER[NumberOfQueryManagers].Password, Script.readString(), sizeof(QUERY_MANAGER[NumberOfQueryManagers].Password) - 1);
				DisguisePassword(QUERY_MANAGER[NumberOfQueryManagers].Password, PasswordKey);
				Script.readSymbol(')');
				NumberOfQueryManagers += 1;
			}while(Script.readSpecial() != '}');
		}else{
			// Safely skip unhandled value
			Script.nextToken();
			if(Script.Token == SPECIAL && (Script.getSpecial() == '{' || Script.getSpecial() == '(')){
				char closeChar = (Script.getSpecial() == '{') ? '}' : ')';
				while(Script.Token != ENDOFFILE && !(Script.Token == SPECIAL && Script.getSpecial() == closeChar)){
					Script.nextToken();
				}
			}
		}
		}
	}

	NormalizeDataPath(BINPATH, "bin");
	NormalizeDataPath(MAPPATH, "map");
	NormalizeDataPath(ORIGMAPPATH, "origmap");
	NormalizeDataPath(DATAPATH, "dat");
	NormalizeDataPath(MONSTERPATH, "mon");
	NormalizeDataPath(NPCPATH, "npc");
	NormalizeDataPath(USERPATH, "usr");
	NormalizeDataPath(LOGPATH, "log");
	NormalizeDataPath(SAVEPATH, "save");

	if(GamePort == 0){
		GamePort = 7172;
	}
	if(Beat == 0){
		Beat = 50;
	}
}
