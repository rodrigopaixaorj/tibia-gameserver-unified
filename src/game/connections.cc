#include "connections.hh"
#include "cr.hh"
#include "info.hh"
#include "threads.hh"
#include "writer.hh"
#include "../database/db.hh"
#include "../database/db_common.hh"
#include "config.hh"

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cctype>

#if !OS_WINDOWS
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#endif

static Semaphore ConnectionMutex(1);
static int ConnectionIterator;
static TConnection Connections[MAX_CONNECTIONS];

// TConnection
// =============================================================================
TConnection::TConnection(void){
	this->State = CONNECTION_FREE;
	this->IsCastSpectator = false;
	this->CastingPlayerID = 0;
	this->SuppressCastBroadcast = false;
	this->LastCastSwitchMs = 0;
	this->LastCastMessageMs = 0;
	this->IsAccountManager = false;
	this->ManagerAccountID = 0;
	this->ManagerState = AM_STATE_NONE;
	this->DraftAccountID = 0;
	this->DraftPassword[0] = 0;
	this->DraftCharName[0] = 0;
	this->DraftCharSex = 1;
	this->DraftCharVocation = PROFESSION_NONE;
	this->DraftCharTown[0] = 0;
	this->LastManagerTalkMs = 0;
}

void TConnection::Process(void){
	if(this->InGame()){
		uint32 LastCommand = (RoundNr - this->TimeStamp);
		if(LastCommand == 30 || LastCommand == 60){
			SendPing(this);
		}

		uint32 LastAction = (RoundNr - this->TimeStampAction);
		if(LastAction == 900 && !CheckRight(this->CharacterID, NO_LOGOUT_BLOCK)){
			SendMessage(this, TALK_ADMIN_MESSAGE,
					"You have been idle for %d minutes. You will be disconnected"
					" in one minute if you are still idle then.", 15);
		}

		if(LastAction >= 960 && !CheckRight(this->CharacterID, NO_LOGOUT_BLOCK)){
			this->Logout(0, true);
		}else if(!GameRunning() || !this->ConnectionIsOk || (LastCommand >= 90)){
			this->Logout(0, false);
		}
	}else if(this->State == CONNECTION_LOGIN){
		if(!GameRunning() || !this->ConnectionIsOk){
			this->Disconnect();
		}
	}else if(this->State == CONNECTION_LOGOUT){
		// NOTE(fusion): `TimeStamp` has the logout round which is set by
		// `TConnection::Logout`.
		if(!GameRunning() || !this->ConnectionIsOk || this->TimeStamp <= RoundNr){
			this->Disconnect();
		}
	}
}

void TConnection::ResetTimer(int Command){
	if(this->InGame()){
		this->TimeStamp = RoundNr;
		if(Command != CL_CMD_PING
				&& Command != CL_CMD_GO_STOP
				&& Command != CL_CMD_CANCEL
				&& Command != CL_CMD_REFRESH_FIELD
				&& Command != CL_CMD_REFRESH_CONTAINER){
			this->TimeStampAction = RoundNr;
		}
	}
}

void TConnection::EmergencyPing(void){
	if(this->InGame()){
		uint32 LastCommand = (RoundNr - this->TimeStamp);
		if(LastCommand < 80){
			// TODO(fusion): This is only called by `NetLoadCheck`, when it detects
			// lag, which can only happen after some set number of rounds, usually
			// higher than 100. This is all to say this subtraction below is very
			// unlikely to wrap. Nevertheless, we should also have some helper
			// functions to do saturating addition or subtraction for both signed
			// and unsigned integers.
			this->TimeStamp = RoundNr - 100;
		}
		SendPing(this);
	}
}

pid_t TConnection::GetThreadID(void){
	if(this->State == CONNECTION_FREE){
		error("TConnection::GetThreadID: Verbindung ist nicht zugewiesen.\n");
		return 0;
	}

	return this->ThreadID;
}

bool TConnection::SetLoginTimer(int Timeout){
	if(this->State == CONNECTION_FREE){
		error("TConnection::SetLoginTimer: Verbindung ist nicht zugewiesen.\n");
		return false;
	}

	if(this->LoginTimer != 0){
		error("TConnection::SetLoginTimer: Timer already set.\n");
		return false;
	}

#if !OS_WINDOWS
	struct sigevent SigEvent = {};
	SigEvent.sigev_notify = SIGEV_THREAD_ID;
	SigEvent.sigev_signo = SIGALRM;
	SigEvent.sigev_notify_thread_id = this->ThreadID;
	if(timer_create(CLOCK_MONOTONIC, &SigEvent, &this->LoginTimer) == -1){
		error("TConnection::SetLoginTimer: Failed to create timer: (%d) %s\n",
				errno, GetErrorDescription(errno));
		return false;
	}

	struct itimerspec TimerSpec = {};
	TimerSpec.it_value.tv_sec = Timeout;
	if(timer_settime(this->LoginTimer, 0, &TimerSpec, NULL) == -1){
		error("TConnection::SetLoginTimer: Failed to start timer: (%d) %s\n",
				errno, GetErrorDescription(errno));
		return false;
	}
#else
	this->LoginTimer = 1;
#endif

	return true;
}

void TConnection::StopLoginTimer(void){
	if(this->State == CONNECTION_FREE){
		error("TConnection::SetLoginTimer: Verbindung ist nicht zugewiesen.\n");
		return;
	}

	if(this->LoginTimer == 0){
		error("TConnection::StopLoginTimer: Timer not set.\n");
		return;
	}

#if !OS_WINDOWS
	if(timer_delete(this->LoginTimer) == -1){
		error("TConnection::StopLoginTimer: Failed to delete timer: (%d) %s\n",
				errno, GetErrorDescription(errno));
	}
#endif

	this->LoginTimer = 0;
}

int TConnection::GetSocket(void){
	if(this->State == CONNECTION_FREE || this->State == CONNECTION_ASSIGNED){
		error("TConnection::GetSocket: Verbindung ist nicht angeschlossen.\n");
		return -1;
	}

	return this->Socket;
}

const char *TConnection::GetIPAddress(void){
	if(this->State == CONNECTION_FREE || this->State == CONNECTION_ASSIGNED){
		error("TConnection::GetIPAddress: Verbindung ist nicht angeschlossen.\n");
		return "Unknown";
	}

	return this->IPAddress;
}

void TConnection::Free(void){
	if(this->IsCastSpectator && this->CastingPlayerID != 0){
		TPlayer *Caster = ::GetPlayer(this->CastingPlayerID);
		if(Caster != NULL){
			Caster->RemoveSpectator(this);
		}
	}
	this->IsCastSpectator = false;
	this->CastingPlayerID = 0;
	this->SuppressCastBroadcast = false;
	this->LastCastSwitchMs = 0;
	this->LastCastMessageMs = 0;
	this->IsAccountManager = false;
	this->ManagerAccountID = 0;
	this->ManagerState = AM_STATE_NONE;
	this->DraftAccountID = 0;
	memset(this->DraftPassword, 0, sizeof(this->DraftPassword));
	this->DraftCharName[0] = 0;
	this->DraftCharSex = 1;
	this->DraftCharVocation = PROFESSION_NONE;
	this->DraftCharTown[0] = 0;
	this->LastManagerTalkMs = 0;
	this->State = CONNECTION_FREE;
}

void TConnection::Assign(void){
	if(this->State != CONNECTION_FREE){
		error("TConnection::Assign: Verbindung ist nicht frei.\n");
	}

	this->State = CONNECTION_ASSIGNED;
	this->ThreadID = gettid();
	this->LoginTimer = 0;
	this->IsCastSpectator = false;
	this->CastingPlayerID = 0;
	this->SuppressCastBroadcast = false;
	this->LastCastSwitchMs = 0;
	this->LastCastMessageMs = 0;
	this->IsAccountManager = false;
	this->ManagerAccountID = 0;
	this->ManagerState = AM_STATE_NONE;
	this->DraftAccountID = 0;
	this->DraftPassword[0] = 0;
	this->DraftCharName[0] = 0;
	this->DraftCharSex = 1;
	this->DraftCharVocation = PROFESSION_NONE;
	this->DraftCharTown[0] = 0;
	this->LastManagerTalkMs = 0;
}

void TConnection::Connect(int Socket){
	if(this->State != CONNECTION_ASSIGNED){
		error("TConnection::Connect: Verbindung ist keinem Thread zugewiesen.\n");
	}

	this->State = CONNECTION_CONNECTED;
	this->Socket = Socket;
	this->ConnectionIsOk = true;
	this->ClosingIsDelayed = true;
	this->RandomSeed = rand();

	struct sockaddr_in RemoteAddr;
	memset(&RemoteAddr, 0, sizeof(RemoteAddr));
	socklen_t RemoteAddrLen = sizeof(RemoteAddr);
	if(getpeername(Socket, (struct sockaddr*)&RemoteAddr, &RemoteAddrLen) == 0 && RemoteAddr.sin_addr.s_addr != 0){
		char *ipStr = inet_ntoa(RemoteAddr.sin_addr);
		if(ipStr){
			strncpy(this->IPAddress, ipStr, sizeof(this->IPAddress) - 1);
			this->IPAddress[sizeof(this->IPAddress) - 1] = 0;
		} else {
			strncpy(this->IPAddress, "127.0.0.1", sizeof(this->IPAddress) - 1);
			this->IPAddress[sizeof(this->IPAddress) - 1] = 0;
		}
	} else {
		strncpy(this->IPAddress, "127.0.0.1", sizeof(this->IPAddress) - 1);
		this->IPAddress[sizeof(this->IPAddress) - 1] = 0;
	}
}

void TConnection::Login(void){
	if(this->State != CONNECTION_CONNECTED){
		error("TConnection::Connect: Ungültiger Verbindungs-Zustand %d.\n", this->State);
	}

	this->State = CONNECTION_LOGIN;
}

bool TConnection::JoinGame(TReadBuffer *Buffer){
	if(this->State != CONNECTION_LOGIN){
		error("TConnection::JoinGame: Ungültiger Verbindungszustand %d.\n", this->State);
	}

	this->ClearKnownCreatureTable(false);

	try{
		this->TerminalType = (int)Buffer->readWord();
		this->TerminalVersion = (int)Buffer->readWord();
		this->CharacterID = Buffer->readQuad();
	}catch(const char *str){
		error("TConnection::JoinGame: Fehler beim Auslesen des Puffers (%s).\n", str);
	}

	this->TerminalType = (this->TerminalType & 0xFF);
	if(this->TerminalType == 0 || (this->TerminalType != 1 && this->TerminalType != 2)){
		this->TerminalType = 2;
	}

	this->TerminalOffsetX = 8;
	this->TerminalOffsetY = 6;
	this->TerminalWidth = 18;
	this->TerminalHeight = 14;

	if(this->IsCastSpectator){
		TPlayer *Caster = ::GetPlayer(this->CastingPlayerID);
		if(Caster == NULL || !Caster->IsCasting){
			return false;
		}

		this->CharacterID = 0;
		strncpy(this->Name, Caster->Name, sizeof(this->Name) - 1);
		this->Name[sizeof(this->Name) - 1] = 0;
		Caster->AddSpectator(this);

		this->EnterGame();
		SendInitGame(this, Caster->ID);
		SendRights(this);
		SendFullScreen(this);
		SendBodyInventory(this, Caster->ID);
		for(int c = 0; c < 16; ++c){
			Object Con = Caster->GetOpenContainer(c);
			if(Con != NONE && Con.exists()){
				SendContainer(this, c);
			}
		}
		SendAmbiente(this);
		SendPlayerData(this);
		SendPlayerSkills(this);
		SendMessage(this, TALK_LOGIN_MESSAGE, "Welcome to Live Cast! You are currently watching %s.", Caster->Name);
		SendMessage(this, TALK_LOGIN_MESSAGE, "Use Ctrl + Arrow Keys to browse active broadcasts.");
		this->TimeStamp = RoundNr;
		this->TimeStampAction = RoundNr;
		return true;
	}

	if(this->IsAccountManager){
		this->CharacterID = 0;
		strncpy(this->Name, "Account Manager", sizeof(this->Name) - 1);
		this->Name[sizeof(this->Name) - 1] = 0;

		this->EnterGame();
		SendInitGame(this, ACCOUNT_MANAGER_VIRTUAL_ID);
		SendRights(this);
		SendFullScreen(this);
		SendAmbiente(this);
		SendPlayerData(this);
		SendPlayerSkills(this);

		int PosX, PosY, PosZ;
		this->GetPosition(&PosX, &PosY, &PosZ);
		if(this->ManagerAccountID == 0){
			this->ManagerState = AM_STATE_GUEST_MENU;
			const char *msg = "Hello! Type 'account' to create a new account.";
			SendTalk(this, 0, "Account Manager", TALK_SAY, PosX, PosY, PosZ, msg);
		}else{
			this->ManagerState = AM_STATE_ACCOUNT_MENU;
			char msg[192];
			snprintf(msg, sizeof(msg), "Welcome to Account %u! Type 'character' to create a new character, or 'password' to change your password.", this->ManagerAccountID);
			SendTalk(this, 0, "Account Manager", TALK_SAY, PosX, PosY, PosZ, msg);
		}

		this->TimeStamp = RoundNr;
		this->TimeStampAction = RoundNr;
		return true;
	}

	LOG("JoinGame: Initializing CharacterID=%u for Socket=%d", this->CharacterID, this->GetSocket());

	TPlayer *Player = ::GetPlayer(this->CharacterID);
	if(Player == NULL){
		Player = new TPlayer(this, this->CharacterID);
		if(Player->ConstructError != NOERROR){
			LOG_ERR("JoinGame: Error in TPlayer constructor (%d)!", Player->ConstructError);
			delete Player;
			return false;
		}
	}else{
		if(Player->IsDead){
			Log("game", "Spieler %s ist gerade am Sterben - Einloggen gescheitert.\n", Player->Name);
			DecreasePlayerPoolSlotSticky(this->CharacterID);
			return false;
		}

		if(Player->LoggingOut && Player->LogoutPossible() == 0){
			Log("game", "Spieler %s loggt gerade aus - Einloggen gescheitert.\n", Player->Name);
			DecreasePlayerPoolSlotSticky(this->CharacterID);
			return false;
		}

		TConnection *OldConnection = Player->Connection;
		Player->ClearConnection();
		if(OldConnection != NULL){
			OldConnection->CharacterID = 0;
			OldConnection->Logout(0, true);
		}

		DecrementIsOnlineOrder(this->CharacterID);
		Player->TakeOver(this);
	}

	strcpy(this->Name, Player->Name);
	this->TimeStamp = RoundNr;
	this->TimeStampAction = RoundNr;
	return true;
}

void TConnection::EnterGame(void){
	if(this->State != CONNECTION_LOGIN){
		error("TConnection::EnterGame: Ungültiger Verbindungszustand %d.\n", this->State);
	}

	this->State = CONNECTION_GAME;
}

void TConnection::Die(void){
	if(this->State == CONNECTION_GAME){
		this->State = CONNECTION_DEAD;
	}
}

void TConnection::Logout(int Delay, bool StopFight){
	if(!this->InGame() && this->State != CONNECTION_LOGOUT){
		error("TConnection::Logout: Ungültiger Verbindungszustand %d.\n", this->State);
	}

	if(this->IsCastSpectator){
		if(this->CastingPlayerID != 0){
			TPlayer *Caster = ::GetPlayer(this->CastingPlayerID);
			if(Caster != NULL){
				Caster->RemoveSpectator(this);
			}
		}
		this->IsCastSpectator = false;
		this->CastingPlayerID = 0;
		this->Disconnect();
		return;
	}

	if(this->IsAccountManager){
		this->IsAccountManager = false;
		this->ManagerAccountID = 0;
		this->ManagerState = AM_STATE_NONE;
		memset(this->DraftPassword, 0, sizeof(this->DraftPassword));
		this->Disconnect();
		return;
	}

	this->State = CONNECTION_LOGOUT;
	if(this->CharacterID != 0){
		TPlayer *Player = ::GetPlayer(this->CharacterID);
		if(Player != NULL){
			Player->ClearConnection();
			Player->StartLogout(false, StopFight);
		}
	}

	if(Delay < 0){
		Delay = 0;
	}

	this->CharacterID = 0;
	this->TimeStamp = RoundNr + (uint32)Delay;
	this->ClosingIsDelayed = false;
}

void TConnection::Close(bool Delay){
	if(this->State == CONNECTION_FREE || this->State == CONNECTION_ASSIGNED){
		error("TConnection::Close: Ungültiger Verbindungszustand %d.\n", this->State);
	}

	if(this->State == CONNECTION_CONNECTED){
		this->State = CONNECTION_DISCONNECTED;
	}

	this->ConnectionIsOk = false;
	this->ClosingIsDelayed = Delay;
}

void TConnection::Disconnect(void){
	if(this->State == CONNECTION_FREE || this->State == CONNECTION_ASSIGNED){
		error("TConnection::Close: Ungültiger Verbindungszustand %d.\n", this->State);
	}

	this->ClearKnownCreatureTable(true);
	this->ConnectionIsOk = false;
	this->State = CONNECTION_DISCONNECTED;
#if !OS_WINDOWS
	tgkill(GetGameProcessID(), this->ThreadID, SIGHUP);
#endif
}

TPlayer *TConnection::GetPlayer(void){
	if(!this->Live()){
		error("TConnection::GetPlayer: Ungültiger Verbindungszustand %d.\n", this->State);
		return NULL;
	}

	TPlayer *Player = NULL;
	if(this->CharacterID != 0){
		Player = ::GetPlayer(this->CharacterID);
	}else if(this->IsCastSpectator && this->CastingPlayerID != 0){
		Player = ::GetPlayer(this->CastingPlayerID);
	}
	return Player;
}

const char *TConnection::GetName(void){
	if(!this->Live()){
		error("TConnection::GetName: Ungültiger Verbindungszustand %d.\n", this->State);
		return "";
	}

	return this->Name;
}

void TConnection::GetPosition(int *x, int *y, int *z){
	if(this->IsAccountManager){
		GetStartPosition(x, y, z, true);
		if(*x == 0 && *y == 0){
			*x = 32097;
			*y = 32219;
			*z = 7;
		}
		return;
	}
	TPlayer *Player = this->GetPlayer();
	if(Player == NULL && this->IsCastSpectator && this->CastingPlayerID != 0){
		Player = ::GetPlayer(this->CastingPlayerID);
	}
	if(Player != NULL){
		*x = Player->posx;
		*y = Player->posy;
		*z = Player->posz;
	}else{
		*x = 0;
		*y = 0;
		*z = 0;
	}
}

bool TConnection::IsVisible(int x, int y, int z){
	int PlayerX, PlayerY, PlayerZ;
	this->GetPosition(&PlayerX, &PlayerY, &PlayerZ);

	// TODO(fusion): Have a standalone version of `TCreature::CanSeeFloor`?
	if(PlayerZ <= 7){
		if(z > 7){
			return false;
		}
	}else{
		if(std::abs(PlayerZ - z) > 2){
			return false;
		}
	}

	int MinX = (PlayerX - this->TerminalOffsetX) + (PlayerZ - z);
	int MinY = (PlayerY - this->TerminalOffsetY) + (PlayerZ - z);
	int MaxX = MinX + this->TerminalWidth - 1;
	int MaxY = MinY + this->TerminalHeight - 1;
	return x >= MinX && x <= MaxX
		&& y >= MinY && y <= MaxY;
}

KNOWNCREATURESTATE TConnection::KnownCreature(uint32 ID, bool UpdateFollows){
	int EntryIndex = -1;
	for(int i = 0; i < NARRAY(this->KnownCreatureTable); i += 1){
		if(this->KnownCreatureTable[i].CreatureID == ID){
			EntryIndex = i;
			break;
		}
	}

	if(EntryIndex == -1){
		return KNOWNCREATURE_FREE;
	}

	KNOWNCREATURESTATE Result = this->KnownCreatureTable[EntryIndex].State;
	if(Result == KNOWNCREATURE_OUTDATED && UpdateFollows){
		this->KnownCreatureTable[EntryIndex].State = KNOWNCREATURE_UPTODATE;
	}
	return Result;
}

uint32 TConnection::NewKnownCreature(uint32 NewID){
	uint32 OldID = 0;
	int EntryIndex = -1;
	for(int i = 0; i < NARRAY(this->KnownCreatureTable); i += 1){
		if(this->KnownCreatureTable[i].CreatureID == NewID){
			OldID = NewID;
			EntryIndex = i;
			break;
		}
	}

	if(EntryIndex == -1){
		for(int i = 0; i < NARRAY(this->KnownCreatureTable); i += 1){
			if(this->KnownCreatureTable[i].State == KNOWNCREATURE_FREE){
				OldID = this->KnownCreatureTable[i].CreatureID;
				EntryIndex = i;
				break;
			}
		}
	}

	if(EntryIndex == -1){
		for(int i = 0; i < NARRAY(this->KnownCreatureTable); i += 1){
			TCreature *Creature = GetCreature(this->KnownCreatureTable[i].CreatureID);
			if(Creature == NULL || !this->IsVisible(Creature->posx, Creature->posy, Creature->posz)){
				OldID = this->KnownCreatureTable[i].CreatureID;
				EntryIndex = i;
				this->UnchainKnownCreature(OldID);
				break;
			}
		}
	}

	if(EntryIndex == -1){
		print(3, "KnownCreatureTable ausgelastet.\n");
		return 0;
	}

	if(this->KnownCreatureTable[EntryIndex].State != KNOWNCREATURE_FREE){
		error("TUserCom::NewKnownCreature: Slot ist nicht gelöscht.\n");
	}

	this->KnownCreatureTable[EntryIndex].State = KNOWNCREATURE_UPTODATE;
	this->KnownCreatureTable[EntryIndex].CreatureID = NewID;

	TCreature *Creature = GetCreature(NewID);
	if(Creature != NULL){
		this->KnownCreatureTable[EntryIndex].Next = Creature->FirstKnowingConnection;
		Creature->FirstKnowingConnection = &this->KnownCreatureTable[EntryIndex];
	}else{
		error("TUserCom::NewKnownCreature: Kreatur %u existiert nicht.\n", NewID);
	}

	return OldID;
}

void TConnection::ClearKnownCreatureTable(bool Unchain){
	for(int i = 0; i < NARRAY(this->KnownCreatureTable); i += 1){
		if(Unchain && this->KnownCreatureTable[i].State != KNOWNCREATURE_FREE){
			this->UnchainKnownCreature(this->KnownCreatureTable[i].CreatureID);
		}
		this->KnownCreatureTable[i].State = KNOWNCREATURE_FREE;
		this->KnownCreatureTable[i].CreatureID = 0;
		this->KnownCreatureTable[i].Connection = this;
	}
}

void TConnection::UnchainKnownCreature(uint32 ID){
	TCreature *Creature = GetCreature(ID);
	if(Creature == NULL){
		error("TUserCom::UnchainKnownCreature: Kreatur %u existiert nicht.\n", ID);
		return;
	}

	if(Creature->FirstKnowingConnection == NULL){
		error("TUserCom::UnchainKnownCreature: Kreatur %u kennt niemand.\n", ID);
		return;
	}

	TKnownCreature *Prev = NULL;
	TKnownCreature *Cur = Creature->FirstKnowingConnection;
	while(Cur != NULL){
		if(Cur->Connection == this){
			break;
		}
		Prev = Cur;
		Cur = Cur->Next;
	}

	if(Cur == NULL){
		error("TUserCom::UnchainKnownCreature: Kreatur %u ist nicht bekannt.\n", ID);
		return;
	}

	Cur->State = KNOWNCREATURE_FREE;
	if(Prev == NULL){
		Creature->FirstKnowingConnection = Cur->Next;
	}else{
		Prev->Next = Cur->Next;
	}
}

// Connection Utility
// =============================================================================
TConnection *AssignFreeConnection(void){
	TConnection *Connection = NULL;
	ConnectionMutex.down();
	for(int i = 0; i < MAX_CONNECTIONS; i += 1){
		if(Connections[i].State == CONNECTION_FREE){
			Connection = &Connections[i];
			Connection->Assign();
			break;
		}
	}
	ConnectionMutex.up();
	return Connection;
}

TConnection *GetFirstConnection(void){
	ConnectionIterator = 0;
	return GetNextConnection();
}

TConnection *GetNextConnection(void){
	TConnection *NextConnection = NULL;
	while(ConnectionIterator < MAX_CONNECTIONS){
		TConnection *Connection = &Connections[ConnectionIterator];
		ConnectionIterator += 1;
		if(Connection->State != CONNECTION_FREE){
			NextConnection = Connection;
			break;
		}
	}
	return NextConnection;
}

TConnection *GetConnectionByIndex(int index){
	if(index < 0 || index >= MAX_CONNECTIONS){
		return NULL;
	}
	return &Connections[index];
}

void ProcessConnections(void){
	TConnection *Connection = GetFirstConnection();
	while(Connection != NULL){
		Connection->Process();
		Connection = GetNextConnection();
	}
}

void InitConnections(void){
	InitSending();
	InitReceiving();

	ConnectionIterator = 0;
	for(int i = 0; i < MAX_CONNECTIONS; i += 1){
		Connections[i].State = CONNECTION_FREE;
	}
}

void ExitConnections(void){
	ExitSending();
	ExitReceiving();
}

// Live Cast System implementation
static std::vector<TPlayer*> GetActiveCasters(void){
	std::vector<TPlayer*> casters;
	for(int i = 0; i < MAX_CONNECTIONS; ++i){
		TConnection *Conn = &Connections[i];
		if(Conn->InGame() && !Conn->IsCastSpectator && Conn->CharacterID != 0){
			TPlayer *Player = Conn->GetPlayer();
			if(Player != NULL && Player->IsCasting){
				casters.push_back(Player);
			}
		}
	}
	return casters;
}

int GetActiveCastCount(void){
	int count = 0;
	for(int i = 0; i < MAX_CONNECTIONS; ++i){
		TConnection *Conn = &Connections[i];
		if(Conn->InGame() && !Conn->IsCastSpectator && Conn->CharacterID != 0){
			TPlayer *Player = Conn->GetPlayer();
			if(Player != NULL && Player->IsCasting){
				count++;
			}
		}
	}
	return count;
}

int GetActiveCastList(struct TCharacterLoginData *OutData, int MaxCount, const char *PasswordFilter){
	if(OutData == NULL || MaxCount <= 0){
		return 0;
	}

	int count = 0;
	for(int i = 0; i < MAX_CONNECTIONS && count < MaxCount; ++i){
		TConnection *Conn = &Connections[i];
		if(Conn->InGame() && !Conn->IsCastSpectator && Conn->CharacterID != 0){
			TPlayer *Player = Conn->GetPlayer();
			if(Player != NULL && Player->IsCasting){
				bool hasPassword = (Player->CastPassword[0] != 0);
				if(hasPassword && PasswordFilter != NULL && PasswordFilter[0] != 0){
					if(strcmp(Player->CastPassword, PasswordFilter) != 0){
						continue;
					}
				}

				TCharacterLoginData &charData = OutData[count];
				memset(&charData, 0, sizeof(charData));
				if(hasPassword && (PasswordFilter == NULL || PasswordFilter[0] == 0)){
					snprintf(charData.Name, sizeof(charData.Name), "%s*", Player->Name);
				}else{
					strncpy(charData.Name, Player->Name, sizeof(charData.Name) - 1);
				}

				snprintf(charData.WorldName, sizeof(charData.WorldName), "Cast [%d/%d]",
					(int)Player->Spectators.size(), LiveCastMaxSpectators);

				if(WorldAddress[0] != 0 && strcmp(WorldAddress, "0.0.0.0") != 0){
					charData.WorldAddress = DB_ParseIPv4(WorldAddress);
				}else{
					charData.WorldAddress = DB_ParseIPv4("127.0.0.1");
				}
				charData.WorldPort = GamePort > 0 ? GamePort : 7172;
				count++;
			}
		}
	}
	return count;
}

void TConnection::SwitchCast(int Direction){
	if(!this->IsCastSpectator){
		return;
	}

	int64 now = GetClockMonotonicMS();
	constexpr int64 SwitchCooldownMs = 1500;
	if(now - this->LastCastSwitchMs < SwitchCooldownMs){
		int remainingSecs = (int)((SwitchCooldownMs - (now - this->LastCastSwitchMs) + 999) / 1000);
		SendMessage(this, TALK_FAILURE_MESSAGE, "Please wait %d second%s before switching casts again.",
			remainingSecs, remainingSecs == 1 ? "" : "s");
		SendSnapback(this);
		return;
	}

	std::vector<TPlayer*> casters = GetActiveCasters();
	if(casters.size() <= 1 && this->CastingPlayerID != 0){
		SendSnapback(this);
		SendMessage(this, TALK_LOGIN_MESSAGE, "There are no other active broadcasts to switch to.");
		return;
	}

	this->LastCastSwitchMs = now;

	std::sort(casters.begin(), casters.end(), [](TPlayer *a, TPlayer *b){
		return strcmp(a->Name, b->Name) < 0;
	});

	int currentIndex = -1;
	for(size_t i = 0; i < casters.size(); ++i){
		if(casters[i]->ID == this->CastingPlayerID){
			currentIndex = (int)i;
			break;
		}
	}

	int newIndex = currentIndex;
	if(currentIndex == -1){
		newIndex = 0;
	}else{
		if(Direction == CL_CMD_ROTATE_NORTH || Direction == CL_CMD_ROTATE_EAST){
			newIndex = (currentIndex + 1) % (int)casters.size();
		}else{
			newIndex = (currentIndex - 1 + (int)casters.size()) % (int)casters.size();
		}
	}

	if(newIndex >= 0 && newIndex < (int)casters.size()){
		TPlayer *NewCaster = casters[newIndex];
		if(NewCaster->ID == this->CastingPlayerID && casters.size() > 1){
			return;
		}

		if(this->CastingPlayerID != 0){
			TPlayer *OldCaster = ::GetPlayer(this->CastingPlayerID);
			if(OldCaster != NULL){
				OldCaster->RemoveSpectator(this);
			}
		}

		this->CastingPlayerID = NewCaster->ID;
		strncpy(this->Name, NewCaster->Name, sizeof(this->Name) - 1);
		this->Name[sizeof(this->Name) - 1] = 0;
		NewCaster->AddSpectator(this);

		this->ClearKnownCreatureTable(false);
		SendInitGame(this, NewCaster->ID);
		SendRights(this);
		SendFullScreen(this);
		SendBodyInventory(this, NewCaster->ID);
		for(int c = 0; c < 16; ++c){
			Object Con = NewCaster->GetOpenContainer(c);
			if(Con != NONE && Con.exists()){
				SendContainer(this, c);
			}
		}
		SendAmbiente(this);
		SendPlayerData(this);
		SendPlayerSkills(this);
		SendMessage(this, TALK_LOGIN_MESSAGE, "You are now watching: %s.", NewCaster->Name);
	}
}

// Account Manager System Implementation
// =============================================================================
static std::unordered_map<std::string, int64> g_AccountCreationIpCooldown;

static void SendManagerReply(TConnection *Connection, const char *Format, ...) ATTR_PRINTF(2, 3);
static void SendManagerReply(TConnection *Connection, const char *Format, ...){
	if(Connection == NULL || !Connection->InGame()) return;

	char Buffer[512];
	va_list args;
	va_start(args, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, args);
	va_end(args);

	int PosX, PosY, PosZ;
	Connection->GetPosition(&PosX, &PosY, &PosZ);
	SendTalk(Connection, 0, "Account Manager", TALK_SAY, PosX, PosY, PosZ, Buffer);
}

static bool IsValidPasswordString(const char *Pwd){
	if(Pwd == NULL) return false;
	size_t len = strlen(Pwd);
	if(len < 1 || len > 25) return false;
	if(strcmp(Pwd, "1") == 0) return false;

	for(size_t i = 0; i < len; ++i){
		unsigned char ch = (unsigned char)Pwd[i];
		if(ch < 0x21 || ch > 0x7E){
			return false;
		}
	}
	return true;
}

static bool IsValidCharacterName(const char *Name, const char **OutErrorReason){
	if(Name == NULL){
		*OutErrorReason = "Name cannot be empty.";
		return false;
	}

	size_t len = strlen(Name);
	if(len < 3 || len > 20){
		*OutErrorReason = "Character name must have between 3 and 20 characters.";
		return false;
	}

	if(Name[0] == ' ' || Name[len - 1] == ' '){
		*OutErrorReason = "Character name cannot start or end with a space.";
		return false;
	}

	bool prevSpace = false;
	for(size_t i = 0; i < len; ++i){
		char ch = Name[i];
		if(ch == ' '){
			if(prevSpace){
				*OutErrorReason = "Character name cannot contain consecutive spaces.";
				return false;
			}
			prevSpace = true;
		}else if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')){
			prevSpace = false;
		}else{
			*OutErrorReason = "Character name may only contain letters (A-Z, a-z) and single spaces.";
			return false;
		}
	}

	if(stricmp(Name, "Account Manager") == 0
			|| stricmp(Name, "Gamemaster") == 0
			|| stricmp(Name, "Admin") == 0
			|| stricmp(Name, "Administrator") == 0
			|| stricmp(Name, "God") == 0
			|| stricmp(Name, "Tutor") == 0
			|| stricmp(Name, "CipSoft") == 0
			|| stricmp(Name, "Customer Support") == 0
			|| strnicmp(Name, "GM ", 3) == 0
			|| strnicmp(Name, "CM ", 3) == 0
			|| strnicmp(Name, "God ", 4) == 0
			|| GetRaceByName(Name) != 0){
		*OutErrorReason = "This name is reserved and cannot be used.";
		return false;
	}

	return true;
}

static uint32 GenerateUniqueAccountID(void){
	for(int attempt = 0; attempt < 200; ++attempt){
		uint32 r1 = (uint32)(rand() & 0x7FFF);
		uint32 r2 = (uint32)(rand() & 0x7FFF);
		uint32 candidate = 100000U + ((r1 << 15) ^ r2) % 99899999U;
		if(candidate <= 1 || candidate == 666666U) continue;
		if(!DB_AccountExists(candidate)){
			return candidate;
		}
	}
	return 0;
}

void HandleAccountManagerTalk(TConnection *Connection, const char *RawText){
	if(Connection == NULL || !Connection->IsAccountManager || RawText == NULL){
		return;
	}

	int64 nowMs = GetClockMonotonicMS();
	if(nowMs - Connection->LastManagerTalkMs < 400){
		SendMessage(Connection, TALK_FAILURE_MESSAGE, "Please wait a moment before typing again.");
		return;
	}
	Connection->LastManagerTalkMs = nowMs;

	// Trim leading and trailing whitespace
	char Input[256];
	StringCopy(Input, sizeof(Input), RawText);
	char *Text = Input;
	while(*Text == ' ' || *Text == '\t' || *Text == '\r' || *Text == '\n'){
		++Text;
	}
	size_t len = strlen(Text);
	while(len > 0 && (Text[len - 1] == ' ' || Text[len - 1] == '\t' || Text[len - 1] == '\r' || Text[len - 1] == '\n')){
		Text[--len] = 0;
	}
	if(len == 0){
		return;
	}

	int PosX, PosY, PosZ;
	Connection->GetPosition(&PosX, &PosY, &PosZ);
	SendTalk(Connection, 0, "You", TALK_SAY, PosX, PosY, PosZ, Text);

	// Global cancel command
	if(stricmp(Text, "cancel") == 0 || stricmp(Text, "bye") == 0){
		if(Connection->ManagerAccountID == 0){
			Connection->ManagerState = AM_STATE_GUEST_MENU;
			SendManagerReply(Connection, "Operation cancelled. Type 'account' if you want to create a new account.");
		}else{
			Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
			SendManagerReply(Connection, "Operation cancelled. Type 'character' to create a character, or 'password' to change your password.");
		}
		return;
	}

	// -------------------------------------------------------------------------
	// 1. GUEST MODE (Logged in with 1/1 -> Account Creation)
	// -------------------------------------------------------------------------
	if(Connection->ManagerAccountID == 0){
		switch(Connection->ManagerState){
			case AM_STATE_GUEST_MENU:
			default:{
				if(stricmp(Text, "account") == 0 || stricmp(Text, "acc") == 0 || stricmp(Text, "yes") == 0){
					Connection->ManagerState = AM_STATE_GUEST_ASK_ACC;
					SendManagerReply(Connection, "What would you like your account number to be? Enter up to 8 numbers (e.g. 123456), or type 'random' to generate one automatically.");
				}else{
					SendManagerReply(Connection, "Hello! Type 'account' to create your account.");
				}
				break;
			}

			case AM_STATE_GUEST_ASK_ACC:{
				if(stricmp(Text, "random") == 0 || stricmp(Text, "auto") == 0 || stricmp(Text, "generate") == 0){
					uint32 genAcc = GenerateUniqueAccountID();
					if(genAcc == 0){
						SendManagerReply(Connection, "Failed to generate a random account number right now. Please type your desired account number (up to 8 digits).");
						return;
					}
					Connection->DraftAccountID = genAcc;
					Connection->ManagerState = AM_STATE_GUEST_ASK_PASS;
					SendManagerReply(Connection, "Your generated account number is '%u' (write it down!). Now, what would you like your password to be? (up to 25 characters)", genAcc);
					return;
				}

				if(len < 1 || len > 8){
					SendManagerReply(Connection, "Invalid length! Account number must have between 1 and 8 digits (only numbers), or type 'random'.");
					return;
				}

				for(size_t i = 0; i < len; ++i){
					if(Text[i] < '0' || Text[i] > '9'){
						SendManagerReply(Connection, "Account number may only contain numbers (0-9), up to 8 digits! Or type 'random' to generate one.");
						return;
					}
				}

				uint32 accNum = (uint32)strtoul(Text, nullptr, 10);
				if(accNum <= 1 || accNum == 666666U){
					SendManagerReply(Connection, "Account number '%u' is reserved. Please choose another number (up to 8 digits).", accNum);
					return;
				}

				if(DB_AccountExists(accNum)){
					SendManagerReply(Connection, "Account number '%u' already exists! Please choose a different account number, or type 'random'.", accNum);
					return;
				}

				Connection->DraftAccountID = accNum;
				Connection->ManagerState = AM_STATE_GUEST_ASK_PASS;
				SendManagerReply(Connection, "Account number '%u' is available! What would you like your password to be? (1 to 25 characters)", accNum);
				break;
			}

			case AM_STATE_GUEST_ASK_PASS:{
				if(strlen(Text) > 25){
					SendManagerReply(Connection, "Password is too long! Maximum allowed is 25 characters. Please enter a shorter password:");
					return;
				}
				if(!IsValidPasswordString(Text)){
					SendManagerReply(Connection, "Invalid password! Use 1 to 25 printable characters (cannot be '1' or contain spaces). Please enter your password:");
					return;
				}

				StringCopy(Connection->DraftPassword, sizeof(Connection->DraftPassword), Text);
				Connection->ManagerState = AM_STATE_GUEST_CONFIRM_ACC;
				SendManagerReply(Connection, "Do you want to create Account '%u' with Password '%s'? Type 'yes' to confirm or 'no' to cancel.",
						Connection->DraftAccountID, Connection->DraftPassword);
				break;
			}

			case AM_STATE_GUEST_CONFIRM_ACC:{
				if(stricmp(Text, "yes") == 0){
					std::string ipKey = Connection->GetIPAddress();
					auto it = g_AccountCreationIpCooldown.find(ipKey);
					if(it != g_AccountCreationIpCooldown.end() && (nowMs - it->second) < 15000){
						int waitSec = (int)((15000 - (nowMs - it->second) + 999) / 1000);
						SendManagerReply(Connection, "Please wait %d second(s) before creating another account.", waitSec);
						return;
					}

					if(DB_AccountExists(Connection->DraftAccountID)){
						Connection->ManagerState = AM_STATE_GUEST_ASK_ACC;
						SendManagerReply(Connection, "Sorry, account number '%u' was just taken! Please choose another account number (up to 8 digits).", Connection->DraftAccountID);
						return;
					}

					if(DB_CreateAccount(Connection->DraftAccountID, Connection->DraftPassword)){
						g_AccountCreationIpCooldown[ipKey] = nowMs;
						uint32 createdAcc = Connection->DraftAccountID;
						Connection->DraftAccountID = 0;
						memset(Connection->DraftPassword, 0, sizeof(Connection->DraftPassword));
						Connection->ManagerState = AM_STATE_GUEST_MENU;
						SendManagerReply(Connection, "Your account '%u' has been created! Please logout (Ctrl+L) and log in with your new account number and password to create your character.", createdAcc);
					}else{
						Connection->ManagerState = AM_STATE_GUEST_MENU;
						SendManagerReply(Connection, "An error occurred while creating your account. Type 'account' to try again.");
					}
				}else if(stricmp(Text, "no") == 0){
					Connection->DraftAccountID = 0;
					memset(Connection->DraftPassword, 0, sizeof(Connection->DraftPassword));
					Connection->ManagerState = AM_STATE_GUEST_MENU;
					SendManagerReply(Connection, "Account creation cancelled. Type 'account' to start again.");
				}else{
					SendManagerReply(Connection, "Please type 'yes' to create Account '%u', or 'no' to cancel.", Connection->DraftAccountID);
				}
				break;
			}
		}
		return;
	}

	// -------------------------------------------------------------------------
	// 2. AUTHENTICATED ACCOUNT MODE (Logged in with real Account -> Create Char / Password)
	// -------------------------------------------------------------------------
	switch(Connection->ManagerState){
		case AM_STATE_ACCOUNT_MENU:
		default:{
			if(stricmp(Text, "character") == 0 || stricmp(Text, "char") == 0){
				int maxChars = (AccountManagerMaxCharacters > 0 ? AccountManagerMaxCharacters : 15);
				int curChars = DB_GetAccountCharacterCount(Connection->ManagerAccountID);
				if(curChars >= maxChars){
					SendManagerReply(Connection, "Your account already has the maximum number of characters (%d).", maxChars);
					return;
				}
				Connection->ManagerState = AM_STATE_CHAR_ASK_NAME;
				SendManagerReply(Connection, "What would you like your character's name to be? (3 to 20 letters, exact capitalization will be preserved)");
			}else if(stricmp(Text, "password") == 0 || stricmp(Text, "pass") == 0){
				Connection->ManagerState = AM_STATE_ACC_ASK_NEW_PASS;
				SendManagerReply(Connection, "What would you like your new password to be? (up to 25 characters)");
			}else{
				SendManagerReply(Connection, "Type 'character' to create a new character, or 'password' to change your account password.");
			}
			break;
		}

		case AM_STATE_ACC_ASK_NEW_PASS:{
			if(strlen(Text) > 25){
				SendManagerReply(Connection, "Password is too long! Maximum allowed is 25 characters. Please enter a shorter password:");
				return;
			}
			if(!IsValidPasswordString(Text)){
				SendManagerReply(Connection, "Invalid password! Use 1 to 25 printable characters (cannot be '1' or contain spaces). Please enter your new password:");
				return;
			}

			StringCopy(Connection->DraftPassword, sizeof(Connection->DraftPassword), Text);
			Connection->ManagerState = AM_STATE_ACC_CONFIRM_PASS;
			SendManagerReply(Connection, "Do you want to change your password to '%s'? Type 'yes' to confirm or 'no' to cancel.", Connection->DraftPassword);
			break;
		}

		case AM_STATE_ACC_CONFIRM_PASS:{
			if(stricmp(Text, "yes") == 0){
				if(DB_ChangeAccountPassword(Connection->ManagerAccountID, Connection->DraftPassword)){
					memset(Connection->DraftPassword, 0, sizeof(Connection->DraftPassword));
					Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
					SendManagerReply(Connection, "Your account password has been changed successfully! Type 'character' if you wish to create a character.");
				}else{
					Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
					SendManagerReply(Connection, "Failed to change password. Please try again later.");
				}
			}else if(stricmp(Text, "no") == 0){
				memset(Connection->DraftPassword, 0, sizeof(Connection->DraftPassword));
				Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
				SendManagerReply(Connection, "Password change cancelled. Type 'character' or 'password'.");
			}else{
				SendManagerReply(Connection, "Please type 'yes' to confirm changing your password, or 'no' to cancel.");
			}
			break;
		}

		case AM_STATE_CHAR_ASK_NAME:{
			const char *errReason = nullptr;
			if(!IsValidCharacterName(Text, &errReason)){
				SendManagerReply(Connection, "%s Please choose another character name:", errReason);
				return;
			}

			if(PlayerExists(Text) || DB_CharacterNameExists(Text)){
				SendManagerReply(Connection, "A character with the name '%s' already exists! Please choose another name:", Text);
				return;
			}

			// Preserve the exact capitalization entered by the player
			StringCopy(Connection->DraftCharName, sizeof(Connection->DraftCharName), Text);
			Connection->ManagerState = AM_STATE_CHAR_ASK_SEX;
			SendManagerReply(Connection, "Character name '%s' is available! Should your character be 'male' or 'female'?", Connection->DraftCharName);
			break;
		}

		case AM_STATE_CHAR_ASK_SEX:{
			if(stricmp(Text, "male") == 0 || stricmp(Text, "m") == 0 || stricmp(Text, "boy") == 0 || stricmp(Text, "masculino") == 0 || stricmp(Text, "homem") == 0){
				Connection->DraftCharSex = 1;
			}else if(stricmp(Text, "female") == 0 || stricmp(Text, "f") == 0 || stricmp(Text, "girl") == 0 || stricmp(Text, "feminino") == 0 || stricmp(Text, "mulher") == 0){
				Connection->DraftCharSex = 2;
			}else{
				SendManagerReply(Connection, "Invalid sex! Please type 'male' or 'female'.");
				return;
			}

			if(AccountManagerChooseVocation){
				Connection->ManagerState = AM_STATE_CHAR_ASK_VOCATION;
				SendManagerReply(Connection, "Which vocation would you like to be: 'sorcerer', 'druid', 'paladin' or 'knight'?");
			}else{
				Connection->DraftCharVocation = PROFESSION_NONE;
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Rookgaard");
				Connection->ManagerState = AM_STATE_CHAR_CONFIRM;
				SendManagerReply(Connection, "Do you want to create '%s' (%s, Rookgaard, Level 1)? Type 'yes' to confirm or 'no' to cancel.",
						Connection->DraftCharName,
						Connection->DraftCharSex == 1 ? "male" : "female");
			}
			break;
		}

		case AM_STATE_CHAR_ASK_VOCATION:{
			if(stricmp(Text, "sorcerer") == 0 || stricmp(Text, "sorc") == 0 || stricmp(Text, "ms") == 0){
				Connection->DraftCharVocation = PROFESSION_SORCERER;
			}else if(stricmp(Text, "druid") == 0 || stricmp(Text, "ed") == 0){
				Connection->DraftCharVocation = PROFESSION_DRUID;
			}else if(stricmp(Text, "paladin") == 0 || stricmp(Text, "pally") == 0 || stricmp(Text, "rp") == 0){
				Connection->DraftCharVocation = PROFESSION_PALADIN;
			}else if(stricmp(Text, "knight") == 0 || stricmp(Text, "kina") == 0 || stricmp(Text, "ek") == 0){
				Connection->DraftCharVocation = PROFESSION_KNIGHT;
			}else{
				SendManagerReply(Connection, "Invalid vocation! Please choose 'sorcerer', 'druid', 'paladin' or 'knight'.");
				return;
			}

			Connection->ManagerState = AM_STATE_CHAR_ASK_TOWN;
			SendManagerReply(Connection, "Which town would you like to live in: 'Thais', 'Carlin', 'Venore', 'Ab'Dendriel' or 'Kazordoon'?");
			break;
		}

		case AM_STATE_CHAR_ASK_TOWN:{
			if(stricmp(Text, "thais") == 0){
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Thais");
			}else if(stricmp(Text, "carlin") == 0){
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Carlin");
			}else if(stricmp(Text, "venore") == 0){
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Venore");
			}else if(stricmp(Text, "ab'dendriel") == 0 || stricmp(Text, "abdendriel") == 0 || stricmp(Text, "ab") == 0){
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Ab'Dendriel");
			}else if(stricmp(Text, "kazordoon") == 0 || stricmp(Text, "kazz") == 0){
				StringCopy(Connection->DraftCharTown, sizeof(Connection->DraftCharTown), "Kazordoon");
			}else{
				SendManagerReply(Connection, "Invalid town! Please choose 'Thais', 'Carlin', 'Venore', 'Ab'Dendriel' or 'Kazordoon'.");
				return;
			}

			const char *vocName = "None";
			if(Connection->DraftCharVocation == PROFESSION_SORCERER) vocName = "Sorcerer";
			else if(Connection->DraftCharVocation == PROFESSION_DRUID) vocName = "Druid";
			else if(Connection->DraftCharVocation == PROFESSION_PALADIN) vocName = "Paladin";
			else if(Connection->DraftCharVocation == PROFESSION_KNIGHT) vocName = "Knight";

			Connection->ManagerState = AM_STATE_CHAR_CONFIRM;
			SendManagerReply(Connection, "Do you want to create '%s' (%s, %s, Level 8, citizen of %s)? Type 'yes' to confirm or 'no' to cancel.",
					Connection->DraftCharName,
					Connection->DraftCharSex == 1 ? "male" : "female",
					vocName,
					Connection->DraftCharTown);
			break;
		}

		case AM_STATE_CHAR_CONFIRM:{
			if(stricmp(Text, "yes") == 0){
				int maxChars = (AccountManagerMaxCharacters > 0 ? AccountManagerMaxCharacters : 15);
				if(DB_GetAccountCharacterCount(Connection->ManagerAccountID) >= maxChars){
					Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
					SendManagerReply(Connection, "Your account has reached the maximum number of characters (%d).", maxChars);
					return;
				}

				if(PlayerExists(Connection->DraftCharName) || DB_CharacterNameExists(Connection->DraftCharName)){
					Connection->ManagerState = AM_STATE_CHAR_ASK_NAME;
					SendManagerReply(Connection, "Sorry, the character name '%s' was just taken! Please choose another name:", Connection->DraftCharName);
					return;
				}

				const char *profStr = "";
				int startLevel = 1;
				if(Connection->DraftCharVocation == PROFESSION_SORCERER){ profStr = "Sorcerer"; startLevel = 8; }
				else if(Connection->DraftCharVocation == PROFESSION_DRUID){ profStr = "Druid"; startLevel = 8; }
				else if(Connection->DraftCharVocation == PROFESSION_PALADIN){ profStr = "Paladin"; startLevel = 8; }
				else if(Connection->DraftCharVocation == PROFESSION_KNIGHT){ profStr = "Knight"; startLevel = 8; }

				uint32 newCharID = 0;
				if(DB_CreateCharacter(Connection->ManagerAccountID, Connection->DraftCharName, Connection->DraftCharSex,
						startLevel, profStr, Connection->DraftCharTown, &newCharID)){
					RegisterNewPlayerInIndex(Connection->DraftCharName, newCharID);
					Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
					SendManagerReply(Connection, "Character '%s' has been created! Please logout (Ctrl+L) and log back into your account to play with '%s', or type 'character' to create another.",
							Connection->DraftCharName, Connection->DraftCharName);
				}else{
					Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
					SendManagerReply(Connection, "An error occurred while creating your character. Type 'character' to try again.");
				}
			}else if(stricmp(Text, "no") == 0){
				Connection->ManagerState = AM_STATE_ACCOUNT_MENU;
				SendManagerReply(Connection, "Character creation cancelled. Type 'character' to start over, or 'password' to change your password.");
			}else{
				SendManagerReply(Connection, "Please type 'yes' to create character '%s', or 'no' to cancel.", Connection->DraftCharName);
			}
			break;
		}
	}
}

