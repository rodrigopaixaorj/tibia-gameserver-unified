#include "query.hh"

void InitQueryManager(void){}
void ExitQueryManager(void){}

int TQueryManagerConnection::checkAccountPassword(uint32 AccountID, const char *Password, const char *IPAddress){
	TCharacterLoginData Chars[1];
	int NumChars = 0;
	int PremDays = 0;
	return DB_LoginAccount(AccountID, Password, IPAddress, 1, &NumChars, Chars, &PremDays);
}

int TQueryManagerConnection::loginAdmin(uint32 AccountID, bool PrivateWorld, int *NumberOfCharacters,
		char (*Characters)[30], char (*Worlds)[30], uint8 (*IPAddresses)[4],
		uint16 *Ports, uint16 *PremiumDaysLeft){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::loadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
		int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
		int *PremiumNewbieBuffer){
	int Res = DB_LoadWorldConfig(WorldType, RebootTime, IPAddress, Port, MaxPlayers, PremiumPlayerBuffer, MaxNewbies, PremiumNewbieBuffer);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::loginGame(uint32 AccountID, char *PlayerName, const char *Password,
		const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
		bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
		char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
		char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated){
	return DB_LoginGame(AccountID, PlayerName, Password, IPAddress, PrivateWorld, PremiumAccountRequired,
			GamemasterRequired, CharacterID, Sex, Guild, Rank, Title, NumberOfBuddies, BuddyIDs,
			BuddyNames, Rights, PremiumAccountActivated);
}

int TQueryManagerConnection::logoutGame(uint32 CharacterID, int Level, const char *Profession,
		const char *Residence, time_t LastLoginTime, int TutorActivities){
	int Res = DB_LogoutGame(CharacterID, Level, Profession, Residence, LastLoginTime, TutorActivities);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::setNotation(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
		const char *Reason, const char *Comment, uint32 *BanishmentID){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::setNamelock(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
		const char *Reason, const char *Comment){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::banishAccount(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
		const char *Reason, const char *Comment, bool *FinalWarning, int *Days,
		uint32 *BanishmentID){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::reportStatement(uint32 ReporterID, const char *PlayerName, const char *Reason,
		const char *Comment, uint32 BanishmentID, uint32 StatementID,
		int NumberOfStatements, uint32 *StatementIDs, int *TimeStamps,
		uint32 *CharacterIDs, const char (*Channels)[30], const char (*Texts)[256]){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::banishIPAddress(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
		const char *Reason, const char *Comment){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::logCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
		const char *Remark, bool Unjustified, time_t Time){
	int Res = DB_LogCharacterDeath(CharacterID, Level, Offender, Remark, Unjustified, Time);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::addBuddy(uint32 AccountID, uint32 Buddy){
	return DB_AddBuddy(AccountID, Buddy);
}

int TQueryManagerConnection::removeBuddy(uint32 AccountID, uint32 Buddy){
	return DB_RemoveBuddy(AccountID, Buddy);
}

int TQueryManagerConnection::decrementIsOnline(uint32 CharacterID){
	return DB_DecrementIsOnline(CharacterID);
}

int TQueryManagerConnection::finishAuctions(int *NumberOfAuctions, uint16 *HouseIDs,
		uint32 *CharacterIDs, char (*CharacterNames)[30], int *Bids){
	if(NumberOfAuctions) *NumberOfAuctions = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::excludeFromAuctions(uint32 CharacterID, bool Banish){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::transferHouses(int *NumberOfTransfers, uint16 *HouseIDs,
		uint32 *NewOwnerIDs, char (*NewOwnerNames)[30], int *Prices){
	if(NumberOfTransfers) *NumberOfTransfers = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::cancelHouseTransfer(uint16 HouseID){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::evictFreeAccounts(int *NumberOfEvictions, uint16 *HouseIDs, uint32 *OwnerIDs){
	if(NumberOfEvictions) *NumberOfEvictions = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::evictDeletedCharacters(int *NumberOfEvictions, uint16 *HouseIDs){
	if(NumberOfEvictions) *NumberOfEvictions = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::evictExGuildleaders(int NumberOfGuildhouses,
		int *NumberOfEvictions, uint16 *HouseIDs, uint32 *Guildleaders){
	if(NumberOfEvictions) *NumberOfEvictions = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::insertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	int Res = DB_InsertHouseOwner(HouseID, OwnerID, PaidUntil);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::updateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil){
	int Res = DB_UpdateHouseOwner(HouseID, OwnerID, PaidUntil);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::deleteHouseOwner(uint16 HouseID){
	int Res = DB_DeleteHouseOwner(HouseID);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::getHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
		uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils){
	int Res = DB_GetHouseOwners(NumberOfOwners, HouseIDs, OwnerIDs, OwnerNames, PaidUntils);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::getAuctions(int *NumberOfAuctions, uint16 *HouseIDs){
	if(NumberOfAuctions) *NumberOfAuctions = 0;
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::startAuction(uint16 HouseID){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::insertHouses(int NumberOfHouses, uint16 *HouseIDs,
		const char **Names, int *Rents, const char **Descriptions,
		int *Sizes, int *PositionsX,int *PositionsY,int *PositionsZ,
		char (*Towns)[30], bool *Guildhouses){
	return QUERY_STATUS_OK;
}

int TQueryManagerConnection::clearIsOnline(int *NumberOfAffectedPlayers){
	int Res = DB_ClearIsOnline(NumberOfAffectedPlayers);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::createPlayerlist(int NumberOfPlayers, const char **Names,
		int *Levels, const char (*Professions)[30], bool *NewRecord){
	int Res = DB_CreatePlayerlist(NumberOfPlayers, Names, Levels, Professions, NewRecord);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::logKilledCreatures(int NumberOfRaces, const char **Names,
		int *KilledPlayers, int *KilledCreatures){
	int Res = DB_LogKilledCreatures(NumberOfRaces, Names, KilledPlayers, KilledCreatures);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}

int TQueryManagerConnection::loadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
		char (*PlayerNames)[30], uint32 *CharacterIDs){
	int Res = DB_LoadPlayers(MinimumCharacterID, NumberOfPlayers, PlayerNames, CharacterIDs);
	return (Res == 0 ? QUERY_STATUS_OK : QUERY_STATUS_ERROR);
}
