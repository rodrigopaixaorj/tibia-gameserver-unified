#include "writer.hh"
#include "config.hh"
#include "cr.hh"
#include "info.hh"
#include "query.hh"
#include "threads.hh"

extern uint32 RoundNr;

static ThreadHandle ProtocolThread;
static ThreadHandle WriterThread;

static TProtocolThreadOrder ProtocolBuffer[1000];
static int ProtocolPointerWrite;
static int ProtocolPointerRead;
static Semaphore ProtocolMutex(1);
static Semaphore ProtocolBufferEmpty(NARRAY(ProtocolBuffer));
static Semaphore ProtocolBufferFull(0);

static TWriterThreadOrder OrderBuffer[2000];
static int OrderPointerWrite;
static int OrderPointerRead;
static Semaphore OrderBufferEmpty(NARRAY(OrderBuffer));
static Semaphore OrderBufferFull(0);

static TWriterThreadReply ReplyBuffer[100];
static int ReplyPointerWrite;
static int ReplyPointerRead;

static TQueryManagerConnection *QueryManagerConnection;

// Protocol Orders
// =============================================================================
void InitProtocol(void){
	ProtocolPointerWrite = 0;
	ProtocolPointerRead = 0;
}

void InsertProtocolOrder(const char *ProtocolName, const char *Text){
	if(ProtocolName == NULL){
		error("InsertProtocolOrder: Protokoll-Name nicht angegeben.\n");
		return;
	}

	if(Text == NULL){
		error("InsertProtocolOrder: Text nicht angegeben.\n");
		return;
	}

	int Orders = (ProtocolPointerWrite - ProtocolPointerRead);
	if(Orders >= NARRAY(ProtocolBuffer) && strcmp(ProtocolName, "error") != 0){
		error("InsertProtocolOrder: Protokoll-Puffer ist voll => Vergrößern.\n");
	}

	ProtocolMutex.down();
	ProtocolBufferEmpty.down();
	int WritePos = ProtocolPointerWrite % NARRAY(ProtocolBuffer);
	strcpy(ProtocolBuffer[WritePos].ProtocolName, ProtocolName);
	strcpy(ProtocolBuffer[WritePos].Text, Text);
	ProtocolPointerWrite += 1;
	ProtocolBufferFull.up();
	ProtocolMutex.up();
}

void GetProtocolOrder(TProtocolThreadOrder *Order){
	ProtocolBufferFull.down();
	int ReadPos = ProtocolPointerRead % NARRAY(ProtocolBuffer);
	*Order = ProtocolBuffer[ReadPos];
	ProtocolPointerRead += 1;
	ProtocolBufferEmpty.up();
}

void WriteProtocol(const char *ProtocolName, const char *Text){
	char FileName[4096];
	snprintf(FileName, sizeof(FileName), "%s/%s.log", LOGPATH, ProtocolName);

	FILE *File = fopen(FileName, "at");
	if(File != NULL){
		fprintf(File, "%s", Text);
		if(fclose(File) != 0){
			error("WriteProtocol: Fehler %d beim Schließen der Datei.\n", errno);
		}
	}
}

int ProtocolThreadLoop(void *Unused){
	TProtocolThreadOrder Order = {};
	while(true){
		GetProtocolOrder(&Order);
		if(Order.ProtocolName[0] == 0){
			break;
		}

		WriteProtocol(Order.ProtocolName, Order.Text);
	}
	return 0;
}

void InitLog(const char *ProtocolName){
	if(ProtocolName == NULL){
		error("InitLog: Protokoll-Name nicht angegeben.\n");
		return;
	}

	char FileName[4096];
	snprintf(FileName, sizeof(FileName), "%s/%s.log", LOGPATH, ProtocolName);

	FILE *File = fopen(FileName, "at");
	if(File ==NULL){
		error("InitLog: Kann Protokoll %s nicht anlegen.\n", ProtocolName);
		return;
	}

	// NOTE(fusion): `ctime` will already add a new line character.
	time_t Time = time(NULL);
	fprintf(File, "-------------------------------------------------------------------------------\n");
	fprintf(File, "Tibia - Graphical Multi-User-Dungeon\n");
	fprintf(File, "%s.log - gestartet %s", ProtocolName, ctime(&Time));
	fclose(File);
}

void Log(const char *ProtocolName, const char *Text, ...){
	if(ProtocolName == NULL || ProtocolName[0] == 0){
		error("Log: Protokoll-Name nicht angegeben.\n");
		return;
	}

	bool WriteDate = (strcmp(ProtocolName, "bugreport") != 0)
			&& (strcmp(ProtocolName, "client-error") != 0)
			&& (strcmp(ProtocolName, "load") != 0);

	char Output[256];
	va_list ap;
	va_start(ap, Text);
	vsnprintf(Output, sizeof(Output), Text, ap);
	va_end(ap);

	char Line[256];
	if(WriteDate){
		print(2, "%s.log: %s", ProtocolName, Output);
		struct tm LocalTime = GetLocalTimeTM(time(NULL));
		snprintf(Line, sizeof(Line), "%02d.%02d.%04d %02d:%02d:%02d (%u): %s",
				LocalTime.tm_mday, LocalTime.tm_mon + 1, LocalTime.tm_year + 1900,
				LocalTime.tm_hour, LocalTime.tm_min, LocalTime.tm_sec, RoundNr, Output);
	}else{
		strcpy(Line, Output);
	}

	if(Line[0] != 0){
		int LineEnd = (int)strlen(Line);
		if(Line[LineEnd - 1] != '\n'){
			if(LineEnd < (int)(sizeof(Line) - 1)){
				Line[LineEnd] = '\n';
				Line[LineEnd + 1] = 0;
			}else{
				Line[LineEnd - 1] = '\n';
			}
		}

		if(ProtocolThread != INVALID_THREAD_HANDLE){
			InsertProtocolOrder(ProtocolName, Line);
		}else{
			WriteProtocol(ProtocolName, Line);
		}
	}
}

// Writer Orders
// =============================================================================
void InitWriterBuffers(void){
	OrderPointerWrite = 0;
	ReplyPointerWrite = 0;
	OrderPointerRead = 0;
	ReplyPointerRead = 0;
}

int GetOrderBufferSpace(void){
	int Result = INT_MAX;
	if(WriterThread != INVALID_THREAD_HANDLE){
		int Orders = (OrderPointerWrite - OrderPointerRead);
		Result = NARRAY(OrderBuffer) - Orders;
	}
	return Result;
}

void InsertOrder(TWriterThreadOrderType OrderType, const void *Data){
	if(WriterThread != INVALID_THREAD_HANDLE){
		int Orders = (OrderPointerWrite - OrderPointerRead);
		if(Orders >= NARRAY(OrderBuffer)){
			error("InsertOrder (Writer): Order-Puffer ist voll => Vergrößern.\n");
		}

		OrderBufferEmpty.down();
		int WritePos = OrderPointerWrite % NARRAY(OrderBuffer);
		OrderBuffer[WritePos].OrderType = OrderType;
		OrderBuffer[WritePos].Data = Data;
		OrderPointerWrite += 1;
		OrderBufferFull.up();
	}
}

void GetOrder(TWriterThreadOrder *Order){
	OrderBufferFull.down();
	*Order = OrderBuffer[OrderPointerRead % NARRAY(OrderBuffer)];
	OrderPointerRead += 1;
	OrderBufferEmpty.up();
}

void TerminateWriterOrder(void){
	InsertOrder(WRITER_ORDER_TERMINATE, NULL);
}

void LogoutOrder(TPlayer *Player){
	if(Player == NULL){
		error("LogoutOrder: Übergebener Spieler existiert nicht.\n");
		return;
	}

	TLogoutOrderData *Data = new TLogoutOrderData;
	Data->CharacterID = Player->ID;
	Data->Level = Player->Skills[SKILL_LEVEL]->Get();
	Data->Profession = Player->GetActiveProfession();
	if(Player->PlayerData == NULL){
		error("LogoutOrder: PlayerData ist NULL.\n");
		Data->LastLoginTime = 0;
	}else{
		Data->LastLoginTime = Player->PlayerData->LastLoginTime;
	}
	Data->TutorActivities = Player->TutorActivities;

	// TODO(fusion): LOL.
	if(Player->startx == 32097){
		strcpy(Data->Residence, "Rookgaard");
	}else if(Player->startx == 32360){
		strcpy(Data->Residence, "Carlin");
	}else if(Player->startx == 32369){
		strcpy(Data->Residence, "Thais");
	}else if(Player->startx == 32595){
		strcpy(Data->Residence, "Port Hope");
	}else if(Player->startx == 32649){
		strcpy(Data->Residence, "Kazordoon");
	}else if(Player->startx == 32732){
		strcpy(Data->Residence, "Ab'Dendriel");
	}else if(Player->startx == 32957){
		strcpy(Data->Residence, "Venore");
	}else if(Player->startx == 33194){
		strcpy(Data->Residence, "Ankrahmun");
	}else if(Player->startx == 33213){
		strcpy(Data->Residence, "Darashia");
	}else if(Player->startx == 33217){
		strcpy(Data->Residence, "Edron");
	}else{
		error("LogoutOrder: Unbekannte Startkoordinate [%d,%d,%d] bei Spieler %s.\n",
				Player->startx, Player->starty, Player->startz, Player->Name);
		strcpy(Data->Residence, "Unknown");
	}

	InsertOrder(WRITER_ORDER_LOGOUT, Data);
}

void PlayerlistOrder(int NumberOfPlayers, const char *PlayerNames,
		int *PlayerLevels, int *PlayerProfessions){
	if(PlayerNames == NULL){
		error("PlayerlistOrder: PlayerNames ist NULL.\n");
		return;
	}

	if(PlayerLevels == NULL){
		error("PlayerlistOrder: PlayerLevels ist NULL.\n");
		return;
	}

	if(PlayerProfessions == NULL){
		error("PlayerlistOrder: PlayerProfessions ist NULL.\n");
		return;
	}

	TPlayerlistOrderData *Data = new TPlayerlistOrderData;
	Data->NumberOfPlayers = NumberOfPlayers;
	Data->PlayerNames = PlayerNames;
	Data->PlayerLevels = PlayerLevels;
	Data->PlayerProfessions = PlayerProfessions;

	InsertOrder(WRITER_ORDER_PLAYERLIST, Data);
}

void KillStatisticsOrder(int NumberOfRaces, const char *RaceNames,
		int *KilledPlayers, int *KilledCreatures){
	if(RaceNames == NULL){
		error("KillStatisticsOrder: RaceNames ist NULL.\n");
		return;
	}

	if(KilledPlayers == NULL){
		error("KillStatisticsOrder: KilledPlayers ist NULL.\n");
		return;
	}

	if(KilledCreatures == NULL){
		error("KillStatisticsOrder: KilledCreatures ist NULL.\n");
		return;
	}

	TKillStatisticsOrderData *Data = new TKillStatisticsOrderData;
	Data->NumberOfRaces = NumberOfRaces;
	Data->RaceNames = RaceNames;
	Data->KilledPlayers = KilledPlayers;
	Data->KilledCreatures = KilledCreatures;

	InsertOrder(WRITER_ORDER_KILLSTATISTICS, Data);
}

void PunishmentOrder(TCreature *Gamemaster, const char *Name, const char *IPAddress,
		int Reason, int Action, const char *Comment, int NumberOfStatements,
		vector<TReportedStatement> *ReportedStatements, uint32 StatementID,
		bool IPBanishment){
	if(Name == NULL){
		error("PunishmentOrder: Name ist NULL.\n");
		return;
	}

	if(Comment == NULL){
		error("PunishmentOrder: Comment ist NULL.\n");
		return;
	}

	TPunishmentOrderData *Data = new TPunishmentOrderData;
	if(Gamemaster != NULL){
		Data->GamemasterID = Gamemaster->ID;
		strcpy(Data->GamemasterName, Gamemaster->Name);
	}else{
		Data->GamemasterID = 0;
		strcpy(Data->GamemasterName, "automatic");
	}
	strcpy(Data->CriminalName, Name);
	strcpy(Data->CriminalIPAddress, (IPAddress != NULL ? IPAddress : ""));
	Data->Reason = Reason;
	Data->Action = Action;
	strcpy(Data->Comment, Comment);
	Data->NumberOfStatements = NumberOfStatements;
	Data->ReportedStatements = ReportedStatements;
	Data->StatementID = StatementID;
	Data->IPBanishment = IPBanishment;

	InsertOrder(WRITER_ORDER_PUNISHMENT, Data);
}

void CharacterDeathOrder(TCreature *Creature, int OldLevel,
		uint32 OffenderID, const char *Remark, bool Unjustified){
	if(Creature == NULL){
		error("CharacterDeathOrder: cr ist NULL.\n");
		return;
	}

	if(Remark == NULL){
		error("CharacterDeathOrder: Remark ist NULL.\n");
		return;
	}

	TCharacterDeathOrderData *Data = new TCharacterDeathOrderData;
	Data->CharacterID = Creature->ID;
	Data->Level = OldLevel;
	Data->Offender = OffenderID;
	strcpy(Data->Remark, Remark);
	Data->Unjustified = Unjustified;
	Data->Time = time(NULL);

	InsertOrder(WRITER_ORDER_CHARACTERDEATH, Data);
}

void AddBuddyOrder(TCreature *Creature, uint32 BuddyID){
	if(Creature == NULL){
		error("AddBuddyOrder: cr ist NULL.\n");
		return;
	}

	if(Creature->Type != PLAYER){
		error("AddBuddyOrder: Kreatur ist kein Spieler.\n");
		return;
	}

	TBuddyOrderData *Data = new TBuddyOrderData;
	Data->AccountID = ((TPlayer*)Creature)->AccountID;
	Data->Buddy = BuddyID;

	InsertOrder(WRITER_ORDER_ADDBUDDY, Data);
}

void RemoveBuddyOrder(TCreature *Creature, uint32 BuddyID){
	if(Creature == NULL){
		error("RemoveBuddyOrder: cr ist NULL.\n");
		return;
	}

	if(Creature->Type != PLAYER){
		error("RemoveBuddyOrder: Kreatur ist kein Spieler.\n");
		return;
	}

	TBuddyOrderData *Data = new TBuddyOrderData;
	Data->AccountID = ((TPlayer*)Creature)->AccountID;
	Data->Buddy = BuddyID;

	InsertOrder(WRITER_ORDER_REMOVEBUDDY, Data);
}

void DecrementIsOnlineOrder(uint32 CharacterID){
	void *Data = (void*)((uintptr)CharacterID);
	InsertOrder(WRITER_ORDER_DECREMENTISONLINE, Data);
}

void SavePlayerDataOrder(void){
	InsertOrder(WRITER_ORDER_SAVEPLAYERDATA, NULL);
}

void ProcessLogoutOrder(TLogoutOrderData *Data){
	if(Data == NULL){
		error("ProcessLogoutOrder: Keine Daten übergeben.\n");
		return;
	}

	if(Data->TutorActivities > 0){
		print(3, "%d Tutor-Punkte für Spieler %u.\n",
				Data->TutorActivities, Data->CharacterID);
	}

	char ProfessionName[30];
	GetProfessionName(ProfessionName, Data->Profession, false, true);
	int Ret = QueryManagerConnection->logoutGame(Data->CharacterID, Data->Level,
			ProfessionName, Data->Residence, Data->LastLoginTime, Data->TutorActivities);
	if(Ret != 0){
		error("ProcessLogoutOrder: Logout für Spieler %u fehlgeschlagen.\n",
				Data->CharacterID);
	}

	delete Data;
}

void ProcessPlayerlistOrder(TPlayerlistOrderData *Data){
	if(Data == NULL){
		error("ProcessPlayerlistOrder: Keine Daten übergeben.\n");
		return;
	}

	bool NewRecord = false;
	if(Data->NumberOfPlayers <= 0){
		int Ret = QueryManagerConnection->createPlayerlist(Data->NumberOfPlayers,
				NULL, NULL, NULL, &NewRecord);
		if(Ret != 0){
			error("ProcessPlayerlistOrder: Anfrage fehlgeschlagen (1).\n");
		}
	}else{
		const char **Names      = (const char**)alloca(Data->NumberOfPlayers * sizeof(const char*));
		int *Levels             = (int*)alloca(Data->NumberOfPlayers * sizeof(int));
		char (*Professions)[30] = (char(*)[30])alloca(Data->NumberOfPlayers * 30);
		for(int PlayerNr = 0; PlayerNr < Data->NumberOfPlayers; PlayerNr += 1){
			Names[PlayerNr] = &Data->PlayerNames[PlayerNr * 30];
			Levels[PlayerNr] = Data->PlayerLevels[PlayerNr];
			GetProfessionName(Professions[PlayerNr],
					Data->PlayerProfessions[PlayerNr], false, true);
		}

		int Ret = QueryManagerConnection->createPlayerlist(Data->NumberOfPlayers,
				Names, Levels, Professions, &NewRecord);
		if(Ret != 0){
			error("ProcessPlayerlistOrder: Anfrage fehlgeschlagen (2).\n");
		}
	}

	if(NewRecord){
		BroadcastReply("New record: %d players are logged in.", Data->NumberOfPlayers);
	}

	delete[] Data->PlayerNames;
	delete[] Data->PlayerLevels;
	delete[] Data->PlayerProfessions;
	delete Data;
}

void ProcessKillStatisticsOrder(TKillStatisticsOrderData *Data){
	if(Data == NULL){
		error("ProcessKillStatisticsOrder: Keine Daten übergeben.\n");
		return;
	}

	const char **RaceNames = (const char**)alloca(Data->NumberOfRaces * sizeof(const char*));
	int *KilledPlayers = (int*)alloca(Data->NumberOfRaces * sizeof(int));
	int *KilledCreatures = (int*)alloca(Data->NumberOfRaces * sizeof(int));
	for(int RaceNr = 0; RaceNr < Data->NumberOfRaces; RaceNr += 1){
		RaceNames[RaceNr] = &Data->RaceNames[RaceNr * 30];
		KilledPlayers[RaceNr] = Data->KilledPlayers[RaceNr];
		KilledCreatures[RaceNr] = Data->KilledCreatures[RaceNr];
	}

	int Ret = QueryManagerConnection->logKilledCreatures(Data->NumberOfRaces,
			RaceNames, KilledPlayers, KilledCreatures);
	if(Ret != 0){
		error("ProcessKillStatisticsOrder: Anfrage fehlgeschlagen.\n");
	}

	delete[] Data->RaceNames;
	delete[] Data->KilledPlayers;
	delete[] Data->KilledCreatures;
	delete Data;
}

static const char *GetStatementOutputChannel(int Mode, int Channel){
	const char *Result;
	switch(Mode){
		case TALK_SAY:						Result = "Say"; break;
		case TALK_WHISPER:					Result = "Whisper"; break;
		case TALK_YELL:						Result = "Yell"; break;
		case TALK_PRIVATE_MESSAGE:			Result = "Private Message"; break;
		case TALK_GAMEMASTER_REQUEST:		Result = "Gamemaster Request"; break;
		case TALK_GAMEMASTER_ANSWER:		Result = "Gamemaster Answer"; break;
		case TALK_PLAYER_ANSWER:			Result = "Player Answer"; break;
		case TALK_GAMEMASTER_BROADCAST:		Result = "Broadcast"; break;
		case TALK_GAMEMASTER_MESSAGE:		Result = "Private Message"; break;

		case TALK_CHANNEL_CALL:
		case TALK_GAMEMASTER_CHANNELCALL:
		case TALK_HIGHLIGHT_CHANNELCALL:{
			switch(Channel){
				case CHANNEL_GUILD:			Result = "Guild Channel"; break;
				case CHANNEL_GAMEMASTER:	Result = "Gamemaster Channel"; break;
				case CHANNEL_TUTOR:			Result = "Tutor Channel"; break;
				case CHANNEL_GAMECHAT:		Result = "Game Chat"; break;
				case CHANNEL_TRADE:			Result = "Trade Channel"; break;
				case CHANNEL_RLCHAT:		Result = "Reallife Chat"; break;
				case CHANNEL_HELP:			Result = "Help Channel"; break;
				default:					Result = "Private Chat Channel"; break;
			}
			break;
		}

		default:{
			error("ProcessPunishmentOrder: Ungültiger Modus %d.\n", Mode);
			Result = "Unknown";
			break;
		}
	}

	return Result;
}

void ProcessPunishmentOrder(TPunishmentOrderData *Data){
	// TODO(fusion): I feel this it too complex for handling a simple banishment system.
	if(Data == NULL){
		error("ProcessPunishmentOrder: Keine Daten übergeben.\n");
		return;
	}

	bool Ok = true;
	const char *Reason = GetBanishmentReason(Data->Reason);
	uint32 BanishmentID = 0;

	if(Ok && Data->Action == 0){
		int Ret = QueryManagerConnection->setNotation(Data->GamemasterID,
				Data->CriminalName, Data->CriminalIPAddress, Reason,
				Data->Comment, &BanishmentID);
		switch(Ret){
			case 0:{
				DirectReply(Data->GamemasterID, "Notation for player %s inserted.", Data->CriminalName);
				Log("banish", "%s notiert zu %s: %s.\n", Data->GamemasterName, Data->CriminalName, Data->Comment);
				break;
			}

			case 1:{
				DirectReply(Data->GamemasterID, "A player with this name does not exist. Perhaps he/she has been renamed?");
				Ok = false;
				break;
			}

			case 2:{
				DirectReply(Data->GamemasterID, "You may not report a god or gamemaster.");
				Ok = false;
				break;
			}
		}
	}

	if(Ok && (Data->Action == 1 || Data->Action == 3 || Data->Action == 5)){
		int Ret = QueryManagerConnection->setNamelock(Data->GamemasterID,
				Data->CriminalName, Data->CriminalIPAddress, Reason,
				Data->Comment);
		switch(Ret){
			case 0:{
				DirectReply(Data->GamemasterID, "Player %s reported for renaming.", Data->CriminalName);
				Log("banish", "%s meldet %s zur Namensänderung.\n", Data->GamemasterName, Data->CriminalName);
				break;
			}

			case 3:{
				DirectReply(Data->GamemasterID, "This player has already been reported.");
				break;
			}

			case 1:{
				DirectReply(Data->GamemasterID, "A player with this name does not exist. Perhaps he/she has already been renamed?");
				Ok = false;
				break;
			}

			case 2:
			case 4:{
				DirectReply(Data->GamemasterID, "This name has already been approved.");
				Ok = false;
				break;
			}
		}
	}

	if(Ok && (Data->Action == 2 || Data->Action == 3 || Data->Action == 4 || Data->Action == 5)){
		int Days;
		bool FinalWarning = (Data->Action == 4 || Data->Action == 5);
		int Ret = QueryManagerConnection->banishAccount(Data->GamemasterID,
				Data->CriminalName, Data->CriminalIPAddress, Reason,
				Data->Comment, &FinalWarning, &Days, &BanishmentID);
		switch(Ret){
			case 0:{
				if(Days == -1){
					DirectReply(Data->GamemasterID, "Account of player %s banished infinitely.", Data->CriminalName);
				}else if(FinalWarning){
					DirectReply(Data->GamemasterID, "Account of player %s banished for %d days with final warning.", Data->CriminalName, Days);
				}else{
					DirectReply(Data->GamemasterID, "Account of player %s banished for %d days.", Data->CriminalName, Days);
				}
				LogoutReply(Data->CriminalName);
				Log("banish", "%s verbannt Account von Spieler %s.\n", Data->GamemasterName, Data->CriminalName);
				break;
			}

			case 3:{
				DirectReply(Data->GamemasterID, "Player %s has already been banished.", Data->CriminalName);
				LogoutReply(Data->CriminalName);
				break;
			}

			case 1:{
				DirectReply(Data->GamemasterID, "A player with this name does not exist. Perhaps he/she has been renamed?");
				Ok = false;
				break;
			}

			case 2:{
				DirectReply(Data->GamemasterID, "You may not report a god or gamemaster.");
				Ok = false;
				break;
			}
		}
	}

	if(Ok && Data->StatementID != 0){
		if(Data->NumberOfStatements > 0 && Data->ReportedStatements != NULL){
			uint32 *StatementIDs = (uint32*)alloca(Data->NumberOfStatements * sizeof(uint32));
			int *TimeStamps      = (int*)alloca(Data->NumberOfStatements * sizeof(int));
			uint32 *CharacterIDs = (uint32*)alloca(Data->NumberOfStatements * sizeof(uint32));
			char (*Channels)[30] = (char(*)[30])alloca(Data->NumberOfStatements * 30);
			char (*Texts)[256]   = (char(*)[256])alloca(Data->NumberOfStatements * 256);
			for(int StatementNr = 0; StatementNr < Data->NumberOfStatements; StatementNr += 1){
				TReportedStatement *Statement = Data->ReportedStatements->at(StatementNr);
				StatementIDs[StatementNr] = Statement->StatementID;
				TimeStamps[StatementNr] = Statement->TimeStamp;
				CharacterIDs[StatementNr] = Statement->CharacterID;
				strcpy(Channels[StatementNr], GetStatementOutputChannel(Statement->Mode, Statement->Channel));
				strcpy(Texts[StatementNr], Statement->Text);
			}

			int Ret = QueryManagerConnection->reportStatement(Data->GamemasterID,
					Data->CriminalName, Reason, Data->Comment, BanishmentID,
					Data->StatementID, Data->NumberOfStatements, StatementIDs,
					TimeStamps, CharacterIDs, Channels, Texts);
			switch(Ret){
				case 0:{
					if(Data->Action == 6){
						DirectReply(Data->GamemasterID, "Statement of %s reported.", Data->CriminalName);
					}
					Log("banish", "%s meldet Äußerung von Spieler %s.\n", Data->GamemasterName, Data->CriminalName);
					break;
				}

				case 1:{
					if(Data->Action == 6){
						DirectReply(Data->GamemasterID, "A player with this name does not exist. Perhaps he/she has been renamed?");
					}
					Ok = false;
					break;
				}

				case 2:{
					if(Data->Action == 6){
						DirectReply(Data->GamemasterID, "Statement has already been reported.");
					}
					Ok = false;
					break;
				}
			}
		}else{
			error("ProcessPunishmentOrder: Statements existieren nicht.\n");
		}
	}

	if(Ok && Data->IPBanishment){
		int Ret = QueryManagerConnection->banishIPAddress(Data->GamemasterID,
				Data->CriminalName, Data->CriminalIPAddress, Reason, Data->Comment);
		switch(Ret){
			case 0:{
				DirectReply(Data->GamemasterID, "IP address of %s banished.", Data->CriminalName);
				LogoutReply(Data->CriminalName);
				Log("banish", "%s sperrt die IP-Adresse von %s.\n", Data->GamemasterName, Data->CriminalName);
				break;
			}

			case 1:{
				DirectReply(Data->GamemasterID, "A player with this name does not exist. Perhaps he/she has been renamed?");
				Ok = false;
				break;
			}

			case 2:{
				DirectReply(Data->GamemasterID, "You may not report a god or gamemaster.");
				Ok = false;
				break;
			}
		}
	}

	delete Data->ReportedStatements;
	delete Data;
}

void ProcessCharacterDeathOrder(TCharacterDeathOrderData *Data){
	if(Data == NULL){
		error("ProcessCharacterDeathOrder: Keine Daten übergeben.\n");
		return;
	}

	int Ret = QueryManagerConnection->logCharacterDeath(Data->CharacterID,
			Data->Level, Data->Offender, Data->Remark, Data->Unjustified,
			Data->Time);
	if(Ret != 0){
		error("ProcessCharacterDeathOrder: Protokollierung fehlgeschlagen.\n");
	}

	delete Data;
}

void ProcessAddBuddyOrder(TBuddyOrderData *Data){
	if(Data == NULL){
		error("ProcessAddBuddyOrder: Keine Daten übergeben.\n");
		return;
	}

	int Ret = QueryManagerConnection->addBuddy(Data->AccountID, Data->Buddy);
	if(Ret != 0){
		error("ProcessAddBuddyOrder: Aufnahme fehlgeschlagen.\n");
	}

	delete Data;
}

void ProcessRemoveBuddyOrder(TBuddyOrderData *Data){
	if(Data == NULL){
		error("ProcessRemoveBuddyOrder: Keine Daten übergeben.\n");
		return;
	}

	int Ret = QueryManagerConnection->removeBuddy(Data->AccountID, Data->Buddy);
	if(Ret != 0){
		error("ProcessRemoveBuddyOrder: Entfernen fehlgeschlagen.\n");
	}

	delete Data;
}

void ProcessDecrementIsOnlineOrder(uint32 CharacterID){
	int Ret = QueryManagerConnection->decrementIsOnline(CharacterID);
	if(Ret != 0){
		error("ProcessDecrementIsOnlineOrder: Verringerung fehlgeschlagen.\n");
	}
}

int WriterThreadLoop(void *Unused){
	TWriterThreadOrder Order = {};
	while(true){
		GetOrder(&Order);
		if(Order.OrderType == WRITER_ORDER_TERMINATE){
			break;
		}

		switch(Order.OrderType){
			case WRITER_ORDER_LOGOUT:{
				ProcessLogoutOrder((TLogoutOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_PLAYERLIST:{
				ProcessPlayerlistOrder((TPlayerlistOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_KILLSTATISTICS:{
				ProcessKillStatisticsOrder((TKillStatisticsOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_PUNISHMENT:{
				ProcessPunishmentOrder((TPunishmentOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_CHARACTERDEATH:{
				ProcessCharacterDeathOrder((TCharacterDeathOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_ADDBUDDY:{
				ProcessAddBuddyOrder((TBuddyOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_REMOVEBUDDY:{
				ProcessRemoveBuddyOrder((TBuddyOrderData*)Order.Data);
				break;
			}

			case WRITER_ORDER_DECREMENTISONLINE:{
				uint32 CharacterID = (uint32)((uintptr)Order.Data);
				ProcessDecrementIsOnlineOrder(CharacterID);
				break;
			}

			case WRITER_ORDER_SAVEPLAYERDATA:{
				SavePlayerPoolSlots();
				break;
			}

			default:{
				error("WriterThreadLoop: Unbekanntes Kommando %d.\n", Order.OrderType);
				break;
			}
		}
	}

	return 0;
}

// Writer Replies
// =============================================================================
void InsertReply(TWriterThreadReplyType ReplyType, const void *Data){
	int Replies = (ReplyPointerWrite - ReplyPointerRead);
	if(Replies >= NARRAY(ReplyBuffer)){
		error("InsertReply (Writer): Puffer ist voll; Rückmeldung wird verworfen.\n");
		return;
	}

	int WritePos = ReplyPointerWrite % NARRAY(ReplyBuffer);
	ReplyBuffer[WritePos].ReplyType = ReplyType;
	ReplyBuffer[WritePos].Data = Data;
	ReplyPointerWrite += 1;
}

bool GetReply(TWriterThreadReply *Reply){
	bool Result = (ReplyPointerRead < ReplyPointerWrite);
	if(Result){
		*Reply = ReplyBuffer[ReplyPointerRead % NARRAY(ReplyBuffer)];
		ReplyPointerRead += 1;
	}
	return Result;
}

void BroadcastReply(const char *Text, ...){
	if(Text == NULL){
		error("BroadcastReply: Kein Text angegeben.\n");
		return;
	}

	TBroadcastReplyData *Data = new TBroadcastReplyData;

	va_list ap;
	va_start(ap, Text);
	vsnprintf(Data->Message, sizeof(Data->Message), Text, ap);
	va_end(ap);

	InsertReply(WRITER_REPLY_BROADCAST, Data);
}

void DirectReply(uint32 CharacterID, const char *Text, ...){
	if(CharacterID == 0){
		return;
	}

	if(Text == NULL){
		error("SendDirectReply: Kein Text angegeben.\n");
		return;
	}

	TDirectReplyData *Data = new TDirectReplyData;
	Data->CharacterID = CharacterID;

	va_list ap;
	va_start(ap, Text);
	vsnprintf(Data->Message, sizeof(Data->Message), Text, ap);
	va_end(ap);

	InsertReply(WRITER_REPLY_DIRECT, Data);
}

void LogoutReply(const char *PlayerName){
	if(PlayerName == NULL){
		return;
	}

	// TODO(fusion): Probably some string dup function?
	char *Data = new char[strlen(PlayerName) + 1];
	strcpy(Data, PlayerName);
	InsertReply(WRITER_REPLY_LOGOUT, Data);
}

void ProcessBroadcastReply(TBroadcastReplyData *Data){
	if(Data == NULL){
		error("ProcessBroadcastReply: Keine Daten übergeben.\n");
		return;
	}

	BroadcastMessage(TALK_STATUS_MESSAGE, "%s", Data->Message);

	delete Data;
}

void ProcessDirectReply(TDirectReplyData *Data){
	if(Data == NULL){
		error("ProcessDirectReply: Keine Daten übergeben.\n");
		return;
	}

	TPlayer *Player = GetPlayer(Data->CharacterID);
	if(Player != NULL){
		SendMessage(Player->Connection, TALK_INFO_MESSAGE, "%s", Data->Message);
	}

	delete Data;
}

void ProcessLogoutReply(const char *Name){
	if(Name == NULL){
		error("ProcessLogoutReply: Keine Daten übergeben.\n");
		return;
	}

	TPlayer *Player = GetPlayer(Name);
	if(Player != NULL){
		GraphicalEffect(Player->CrObject, EFFECT_MAGIC_GREEN);
		Player->StartLogout(true, true);
	}

	delete[] Name;
}

void ProcessWriterThreadReplies(void){
	TWriterThreadReply Reply = {};
	while(GetReply(&Reply)){
		switch(Reply.ReplyType){
			case WRITER_REPLY_BROADCAST:{
				ProcessBroadcastReply((TBroadcastReplyData*)Reply.Data);
				break;
			}

			case WRITER_REPLY_DIRECT:{
				ProcessDirectReply((TDirectReplyData*)Reply.Data);
				break;
			}

			case WRITER_REPLY_LOGOUT:{
				ProcessLogoutReply((const char*)Reply.Data);
				break;
			}

			default:{
				error("ProcessWriterThreadReplies: Unbekannte Rückmeldung %d.\n", Reply.ReplyType);
				break;
			}
		}
	}
}

// Initialization
// =============================================================================
void ClearPlayers(void){
	int NumberOfAffectedPlayers;
	int Ret = QueryManagerConnection->clearIsOnline(&NumberOfAffectedPlayers);
	if(Ret != 0){
		error("ClearPlayers: Kann IsOnline-Flags nicht löschen.\n");
	}else if(NumberOfAffectedPlayers != 0){
		error("ClearPlayers: %d Spieler waren als eingeloggt markiert.\n",
				NumberOfAffectedPlayers);
	}
}

void InitWriter(void){
	// TODO(fusion): No idea what's this about.
	int QueryBufferSize = std::max<int>(KB(16), MaxPlayers * 66 + 2);
	QueryManagerConnection = new TQueryManagerConnection(QueryBufferSize);
	if(!QueryManagerConnection->isConnected()){
		throw "cannot connect to query manager";
	}

	ClearPlayers();

	InitProtocol();
	ProtocolThread = StartThread(ProtocolThreadLoop, NULL, false);
	if(ProtocolThread == INVALID_THREAD_HANDLE){
		throw "cannot start protocol thread";
	}

	InitWriterBuffers();
	WriterThread = StartThread(WriterThreadLoop, NULL, false);
	if(WriterThread == INVALID_THREAD_HANDLE){
		throw "cannot start writer thread";
	}
}

void AbortWriter(void){
	if(WriterThread != INVALID_THREAD_HANDLE){
		TerminateWriterOrder();
		JoinThread(WriterThread);
		WriterThread = INVALID_THREAD_HANDLE;
		OrderBufferEmpty.up();
	}
}

void ExitWriter(void){
	if(ProtocolThread != INVALID_THREAD_HANDLE){
		InsertProtocolOrder("", "");
		JoinThread(ProtocolThread);
		ProtocolThread = INVALID_THREAD_HANDLE;
	}

	if(WriterThread != INVALID_THREAD_HANDLE){
		TerminateWriterOrder();
		JoinThread(WriterThread);
		WriterThread = INVALID_THREAD_HANDLE;
	}

	delete QueryManagerConnection;
}
