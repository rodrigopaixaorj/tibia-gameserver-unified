#ifndef TIBIA_QUERY_HH_
#define TIBIA_QUERY_HH_ 1

#include "common.hh"
#include "../database/db.hh"

enum : int {
	QUERY_STATUS_OK			= 0,
	QUERY_STATUS_ERROR		= 1,
	QUERY_STATUS_FAILED		= 3,
};

struct TQueryManagerConnection {
	TQueryManagerConnection(int QueryBufferSize = 16384) {}
	~TQueryManagerConnection(void) {}

	void connect(void) {}
	void disconnect(void) {}
	bool isConnected(void) { return true; }

	int checkAccountPassword(uint32 AccountID, const char *Password, const char *IPAddress);
	int loginAdmin(uint32 AccountID, bool PrivateWorld, int *NumberOfCharacters,
			char (*Characters)[30], char (*Worlds)[30], uint8 (*IPAddresses)[4],
			uint16 *Ports, uint16 *PremiumDaysLeft);
	int loadWorldConfig(int *WorldType, int *RebootTime, int *IPAddress,
			int *Port, int *MaxPlayers, int *PremiumPlayerBuffer, int *MaxNewbies,
			int *PremiumNewbieBuffer);
	int loginGame(uint32 AccountID, char *PlayerName, const char *Password,
			const char *IPAddress, bool PrivateWorld, bool PremiumAccountRequired,
			bool GamemasterRequired, uint32 *CharacterID, int *Sex, char *Guild,
			char *Rank, char *Title, int *NumberOfBuddies, uint32 *BuddyIDs,
			char (*BuddyNames)[30], uint8 *Rights, bool *PremiumAccountActivated);
	int logoutGame(uint32 CharacterID, int Level, const char *Profession,
			const char *Residence, time_t LastLoginTime, int TutorActivities);
	int setNotation(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
			const char *Reason, const char *Comment, uint32 *BanishmentID);
	int setNamelock(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
			const char *Reason, const char *Comment);
	int banishAccount(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
			const char *Reason, const char *Comment, bool *FinalWarning, int *Days,
			uint32 *BanishmentID);
	int reportStatement(uint32 ReporterID, const char *PlayerName, const char *Reason,
			const char *Comment, uint32 BanishmentID, uint32 StatementID,
			int NumberOfStatements, uint32 *StatementIDs, int *TimeStamps,
			uint32 *CharacterIDs, const char (*Channels)[30], const char (*Texts)[256]);
	int banishIPAddress(uint32 GamemasterID, const char *PlayerName, const char *IPAddress,
			const char *Reason, const char *Comment);
	int logCharacterDeath(uint32 CharacterID, int Level, uint32 Offender,
			const char *Remark, bool Unjustified, time_t Time);
	int addBuddy(uint32 AccountID, uint32 Buddy);
	int removeBuddy(uint32 AccountID, uint32 Buddy);
	int decrementIsOnline(uint32 CharacterID);
	int finishAuctions(int *NumberOfAuctions, uint16 *HouseIDs,
			uint32 *CharacterIDs, char (*CharacterNames)[30], int *Bids);
	int excludeFromAuctions(uint32 CharacterID, bool Banish);
	int transferHouses(int *NumberOfTransfers, uint16 *HouseIDs,
			uint32 *NewOwnerIDs, char (*NewOwnerNames)[30], int *Prices);
	int cancelHouseTransfer(uint16 HouseID);
	int evictFreeAccounts(int *NumberOfEvictions, uint16 *HouseIDs, uint32 *OwnerIDs);
	int evictDeletedCharacters(int *NumberOfEvictions, uint16 *HouseIDs);
	int evictExGuildleaders(int NumberOfGuildhouses,
			int *NumberOfEvictions, uint16 *HouseIDs, uint32 *Guildleaders);
	int insertHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil);
	int updateHouseOwner(uint16 HouseID, uint32 OwnerID, int PaidUntil);
	int deleteHouseOwner(uint16 HouseID);
	int getHouseOwners(int *NumberOfOwners, uint16 *HouseIDs,
			uint32 *OwnerIDs, char (*OwnerNames)[30], int *PaidUntils);
	int getAuctions(int *NumberOfAuctions, uint16 *HouseIDs);
	int startAuction(uint16 HouseID);
	int insertHouses(int NumberOfHouses, uint16 *HouseIDs,
			const char **Names, int *Rents, const char **Descriptions,
			int *Sizes, int *PositionsX,int *PositionsY,int *PositionsZ,
			char (*Towns)[30], bool *Guildhouses);
	int clearIsOnline(int *NumberOfAffectedPlayers);
	int createPlayerlist(int NumberOfPlayers, const char **Names,
			int *Levels, const char (*Professions)[30], bool *NewRecord);
	int logKilledCreatures(int NumberOfRaces, const char **Names,
			int *KilledPlayers,int *KilledCreatures);
	int loadPlayers(int MinimumCharacterID, int *NumberOfPlayers,
			char (*PlayerNames)[30], uint32 *CharacterIDs);
};

struct TQueryManagerConnectionPool {
	TQueryManagerConnectionPool(int Count) : m_Connection() {}
	TQueryManagerConnection *get(void) { return &m_Connection; }
	void put(TQueryManagerConnection *conn) {}

	TQueryManagerConnection m_Connection;
};

struct TQueryManagerPoolConnection {
	TQueryManagerPoolConnection(TQueryManagerConnectionPool &Pool) : m_Conn(Pool.get()), m_Pool(&Pool) {}
	TQueryManagerPoolConnection(TQueryManagerConnectionPool *Pool) : m_Conn(Pool ? Pool->get() : nullptr), m_Pool(Pool) {}
	~TQueryManagerPoolConnection(void) { if(m_Pool && m_Conn) m_Pool->put(m_Conn); }
	TQueryManagerConnection *operator->(void) { return m_Conn; }
	TQueryManagerConnection &operator*(void) { return *m_Conn; }
	TQueryManagerConnection *get(void) { return m_Conn; }
	bool isConnected(void) const { return m_Conn != nullptr; }

	TQueryManagerConnection *m_Conn;
	TQueryManagerConnectionPool *m_Pool;
};

void InitQueryManager(void);
void ExitQueryManager(void);

#endif // TIBIA_QUERY_HH_
