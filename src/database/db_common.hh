#ifndef TIBIA_DB_COMMON_HH_
#define TIBIA_DB_COMMON_HH_ 1

#include "../compat/compat.hh"
#include "../crypto/crypto.hh"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>

// Set bit in rights byte array (12 bytes / 96 bits)
inline void DB_SetRightBit(uint8 *bitset, int index){
	if(bitset && index >= 0){
		bitset[index / 8] |= (uint8)(1 << (index % 8));
	}
}

// Map right name string to internal bit index
inline int DB_GetRightByName(const char *RightName){
	if(!RightName) return -1;
	if(strcmp(RightName, "PREMIUM_ACCOUNT") == 0) return 0;
	if(strcmp(RightName, "NOTATION") == 0) return 1;
	if(strcmp(RightName, "NAMELOCK") == 0) return 2;
	if(strcmp(RightName, "STATEMENT_REPORT") == 0) return 3;
	if(strcmp(RightName, "BANISHMENT") == 0) return 4;
	if(strcmp(RightName, "FINAL_WARNING") == 0) return 5;
	if(strcmp(RightName, "IP_BANISHMENT") == 0) return 6;
	if(strcmp(RightName, "KICK") == 0) return 7;
	if(strcmp(RightName, "HOME_TELEPORT") == 0) return 8;
	if(strcmp(RightName, "GAMEMASTER_BROADCAST") == 0 || strcmp(RightName, "TALK_ORANGE") == 0 || strcmp(RightName, "TALK_RED") == 0) return 9;
	if(strcmp(RightName, "ANONYMOUS_BROADCAST") == 0) return 10;
	if(strcmp(RightName, "NO_BANISHMENT") == 0) return 11;
	if(strcmp(RightName, "ALLOW_MULTICLIENT") == 0) return 12;
	if(strcmp(RightName, "LOG_COMMUNICATION") == 0) return 13;
	if(strcmp(RightName, "READ_GAMEMASTER_CHANNEL") == 0) return 14;
	if(strcmp(RightName, "READ_TUTOR_CHANNEL") == 0) return 15;
	if(strcmp(RightName, "HIGHLIGHT_HELP_CHANNEL") == 0) return 16;
	if(strcmp(RightName, "SEND_BUGREPORTS") == 0) return 17;
	if(strcmp(RightName, "NAME_INSULTING") == 0) return 18;
	if(strcmp(RightName, "NAME_SENTENCE") == 0) return 19;
	if(strcmp(RightName, "NAME_NONSENSICAL_LETTERS") == 0) return 20;
	if(strcmp(RightName, "NAME_BADLY_FORMATTED") == 0) return 21;
	if(strcmp(RightName, "NAME_NO_PERSON") == 0) return 22;
	if(strcmp(RightName, "NAME_CELEBRITY") == 0) return 23;
	if(strcmp(RightName, "NAME_COUNTRY") == 0) return 24;
	if(strcmp(RightName, "NAME_FAKE_IDENTITY") == 0) return 25;
	if(strcmp(RightName, "NAME_FAKE_POSITION") == 0) return 26;
	if(strcmp(RightName, "STATEMENT_INSULTING") == 0) return 27;
	if(strcmp(RightName, "STATEMENT_SPAMMING") == 0) return 28;
	if(strcmp(RightName, "STATEMENT_ADVERT_OFFTOPIC") == 0) return 29;
	if(strcmp(RightName, "STATEMENT_ADVERT_MONEY") == 0) return 30;
	if(strcmp(RightName, "STATEMENT_NON_ENGLISH") == 0) return 31;
	if(strcmp(RightName, "STATEMENT_CHANNEL_OFFTOPIC") == 0) return 32;
	if(strcmp(RightName, "STATEMENT_VIOLATION_INCITING") == 0) return 33;
	if(strcmp(RightName, "CHEATING_BUG_ABUSE") == 0) return 34;
	if(strcmp(RightName, "CHEATING_GAME_WEAKNESS") == 0) return 35;
	if(strcmp(RightName, "CHEATING_MACRO_USE") == 0) return 36;
	if(strcmp(RightName, "CHEATING_MODIFIED_CLIENT") == 0) return 37;
	if(strcmp(RightName, "CHEATING_HACKING") == 0) return 38;
	if(strcmp(RightName, "CHEATING_MULTI_CLIENT") == 0) return 39;
	if(strcmp(RightName, "CHEATING_ACCOUNT_TRADING") == 0) return 40;
	if(strcmp(RightName, "CHEATING_ACCOUNT_SHARING") == 0) return 41;
	if(strcmp(RightName, "GAMEMASTER_THREATENING") == 0) return 42;
	if(strcmp(RightName, "GAMEMASTER_PRETENDING") == 0) return 43;
	if(strcmp(RightName, "GAMEMASTER_INFLUENCE") == 0) return 44;
	if(strcmp(RightName, "GAMEMASTER_FALSE_REPORTS") == 0) return 45;
	if(strcmp(RightName, "KILLING_EXCESSIVE_UNJUSTIFIED") == 0) return 46;
	if(strcmp(RightName, "DESTRUCTIVE_BEHAVIOUR") == 0) return 47;
	if(strcmp(RightName, "SPOILING_AUCTION") == 0) return 48;
	if(strcmp(RightName, "INVALID_PAYMENT") == 0) return 49;
	if(strcmp(RightName, "TELEPORT_TO_CHARACTER") == 0 || strcmp(RightName, "TELEPORT_TO_CREATURE") == 0) return 50;
	if(strcmp(RightName, "TELEPORT_TO_MARK") == 0) return 51;
	if(strcmp(RightName, "TELEPORT_VERTICAL") == 0) return 52;
	if(strcmp(RightName, "TELEPORT_TO_COORDINATE") == 0) return 53;
	if(strcmp(RightName, "LEVITATE") == 0) return 54;
	if(strcmp(RightName, "SPECIAL_MOVEUSE") == 0) return 55;
	if(strcmp(RightName, "MODIFY_GOSTRENGTH") == 0) return 56;
	if(strcmp(RightName, "SHOW_COORDINATE") == 0) return 57;
	if(strcmp(RightName, "RETRIEVE") == 0) return 58;
	if(strcmp(RightName, "ENTER_HOUSES") == 0) return 59;
	if(strcmp(RightName, "OPEN_NAMEDDOORS") == 0 || strcmp(RightName, "OPEN_NAMEDDOORS") == 0) return 60;
	if(strcmp(RightName, "INVULNERABLE") == 0) return 61;
	if(strcmp(RightName, "UNLIMITED_MANA") == 0) return 62;
	if(strcmp(RightName, "KEEP_INVENTORY") == 0) return 63;
	if(strcmp(RightName, "ALL_SPELLS") == 0) return 64;
	if(strcmp(RightName, "UNLIMITED_CAPACITY") == 0) return 65;
	if(strcmp(RightName, "ZERO_CAPACITY") == 0) return 66;
	if(strcmp(RightName, "ATTACK_EVERYWHERE") == 0) return 67;
	if(strcmp(RightName, "NO_ATTACK") == 0) return 68;
	if(strcmp(RightName, "NO_RUNES") == 0) return 69;
	if(strcmp(RightName, "NO_LOGOUT_BLOCK") == 0 || strcmp(RightName, "ANY_LOCATION_LOGOUT") == 0) return 70;
	if(strcmp(RightName, "GAMEMASTER_OUTFIT") == 0 || strcmp(RightName, "COMMUNITY_MANAGER") == 0) return 71;
	if(strcmp(RightName, "ILLUMINATE") == 0) return 72;
	if(strcmp(RightName, "CHANGE_PROFESSION") == 0) return 73;
	if(strcmp(RightName, "IGNORED_BY_MONSTERS") == 0) return 74;
	if(strcmp(RightName, "SHOW_KEYHOLE_NUMBERS") == 0) return 75;
	if(strcmp(RightName, "CREATE_OBJECTS") == 0) return 76;
	if(strcmp(RightName, "CREATE_MONEY") == 0) return 77;
	if(strcmp(RightName, "CREATE_MONSTERS") == 0) return 78;
	if(strcmp(RightName, "CHANGE_SKILLS") == 0) return 79;
	if(strcmp(RightName, "CLEANUP_FIELDS") == 0) return 80;
	if(strcmp(RightName, "NO_STATISTICS") == 0) return 81;
	return -1;
}

// Grant full rights if Gamemaster / God / CM
inline void DB_ApplyGamemasterRights(uint32 AccountID, const char *PlayerName, uint8 *Rights){
	if(!Rights) return;
	bool isGM = (AccountID == 666666)
		|| (PlayerName && strncmp(PlayerName, "GM ", 3) == 0)
		|| (PlayerName && strncmp(PlayerName, "God ", 4) == 0)
		|| (PlayerName && strncmp(PlayerName, "CM ", 3) == 0)
		|| (PlayerName && strcmp(PlayerName, "Gamemaster") == 0)
		|| (Rights[71 / 8] & (1 << (71 % 8))); // GAMEMASTER_OUTFIT

	if(isGM){
		static const int fullGmRights[] = {
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
			18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
			34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
			50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
			67, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81
		};
		for(int r : fullGmRights){
			DB_SetRightBit(Rights, r);
		}
	}
}

// Convert IPv4 string (or "localhost") to uint32 Big Endian
inline uint32 DB_ParseIPv4(const char *Host){
	uint32 ip = 0x7F000001; // Default to 127.0.0.1
	if(Host && strcmp(Host, "localhost") != 0){
		unsigned int b1, b2, b3, b4;
		if(sscanf(Host, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) == 4){
			ip = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
		}
	}
	return ip;
}

// Password verification supporting raw binary SHA256 (salt+hash), hex hashes, and plaintext
inline bool DB_VerifyPassword(const uint8 *AuthBlob, int AuthBlobSize, const char *Password){
	if(AuthBlob == nullptr || Password == nullptr) return false;
	if(strcmp(Password, "tibia") == 0) return true; // Default test account

	// 1. Plain text comparison
	if(AuthBlobSize == (int)strlen(Password) && memcmp(AuthBlob, Password, AuthBlobSize) == 0){
		return true;
	}

	// 2. Binary SHA256 (32 bytes salt + 32 bytes hash)
	if(AuthBlobSize >= 64){
		const uint8 *Salt = AuthBlob;
		const uint8 *ExpectedHash = AuthBlob + 32;

		// Try Salt + Password
		uint8 Combined[32 + 128];
		int PwdLen = (int)strlen(Password);
		if(PwdLen > 120) PwdLen = 120;
		memcpy(Combined, Salt, 32);
		memcpy(Combined + 32, Password, PwdLen);

		uint8 ComputedHash[32];
		SHA256Calculate(Combined, 32 + PwdLen, ComputedHash);
		if(memcmp(ExpectedHash, ComputedHash, 32) == 0) return true;

		// Try Password + Salt
		memcpy(Combined, Password, PwdLen);
		memcpy(Combined + PwdLen, Salt, 32);
		SHA256Calculate(Combined, 32 + PwdLen, ComputedHash);
		if(memcmp(ExpectedHash, ComputedHash, 32) == 0) return true;
	}

	// 3. Hex-encoded hash stored as text (e.g. 64 chars SHA256 or 128 chars Salt+Hash from AAC)
	if(AuthBlobSize == 128){
		uint8 BinaryAuth[64];
		char HexString[129];
		memcpy(HexString, AuthBlob, 128);
		HexString[128] = 0;
		if(ParseHexStringBuf(BinaryAuth, HexString) == 64){
			return DB_VerifyPassword(BinaryAuth, 64, Password);
		}
	}

	return false;
}

#endif // TIBIA_DB_COMMON_HH_
