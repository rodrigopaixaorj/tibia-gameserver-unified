#include "communication.hh"
#include "config.hh"
#include "connections.hh"
#include "containers.hh"
#include "crypto.hh"
#include "query.hh"
#include "threads.hh"
#include "writer.hh"

#if !OS_WINDOWS
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#endif

#define PACKET_AVERAGE_SIZE_OVERHEAD 48
#define MAX_COMMUNICATION_THREADS 1100
#define COMMUNICATION_THREAD_STACK_SIZE ((int)KB(64))

#ifndef TIBIA772
#define TIBIA772 1
#endif

#if TIBIA772
static const int TERMINALVERSION[] = {772, 772, 772};
#else
static const int TERMINALVERSION[] = {770, 770, 770};
#endif

static socket_t TCPSocket = SOCKET_INVALID;
static ThreadHandle AcceptorThread = INVALID_THREAD_HANDLE;
static uint32 AcceptorThreadID = 0;
static int ActiveConnections = 0;

static Semaphore RSAMutex(1);
static TRSAPrivateKey PrivateKey;

static TQueryManagerConnectionPool QueryManagerConnectionPool(10);
static int LoadHistory[360];
static int LoadHistoryPointer;
static int TotalLoad;
static int TotalSend;
static int TotalRecv;
static uint32 LagEnd;
static uint32 EarliestFreeAccountAdmissionRound;
static store<TWaitinglistEntry, 100> Waitinglist;
static TWaitinglistEntry *WaitinglistHead;

static Semaphore CommunicationThreadMutex(1);
static bool UseOwnStacks = false;

// Communication Thread Stacks
void GetCommunicationThreadStack(int *StackNumber, void **Stack){
	*StackNumber = -1;
	*Stack = NULL;
}
void AttachCommunicationThreadStack(int StackNumber){}
void ReleaseCommunicationThreadStack(int StackNumber){}
void InitCommunicationThreadStacks(void){}
void ExitCommunicationThreadStacks(void){}

// Load History
bool LagDetected(void){
	return RoundNr <= LagEnd;
}

void NetLoad(int Amount, bool Send){
	CommunicationThreadMutex.down();
	if(Send){
		TotalSend += Amount;
	}else{
		TotalRecv += Amount;
	}
	CommunicationThreadMutex.up();
}

void NetLoadSummary(void){
	CommunicationThreadMutex.down();
	Log("netload", "gesendet:  %d Bytes.\n", TotalSend);
	Log("netload", "empfangen: %d Bytes.\n", TotalRecv);
	TotalSend = 0;
	TotalRecv = 0;
	CommunicationThreadMutex.up();
}

void NetLoadCheck(void){
	static int LastRecv;
	int DeltaRecv = TotalRecv - LastRecv;
	LastRecv = TotalRecv;
	if(DeltaRecv < 0){
		return;
	}

	int DeltaRecvPerPlayer = 0;
	int PlayersOnline = GetPlayersOnline();
	if(PlayersOnline > 0){
		DeltaRecvPerPlayer = DeltaRecv / PlayersOnline;
	}

	TotalLoad -= LoadHistory[LoadHistoryPointer];
	TotalLoad += DeltaRecvPerPlayer;
	LoadHistory[LoadHistoryPointer] = DeltaRecvPerPlayer;
	LoadHistoryPointer += 1;
	if(LoadHistoryPointer >= NARRAY(LoadHistory)){
		LoadHistoryPointer = 0;
	}

	constexpr uint32 EarliestLagCheckRound = 10 * NARRAY(LoadHistory);
	if(RoundNr >= EarliestLagCheckRound && PlayersOnline >= 50){
		int AvgDeltaRecvPerPlayer = (TotalLoad / NARRAY(LoadHistory));
		if(DeltaRecvPerPlayer < (AvgDeltaRecvPerPlayer / 2)){
			Log("game", "Lag erkannt!\n");
			LagEnd = RoundNr + 30;

			int FreeAccountAdmissionDelay = 60;
			if(PremiumPlayerBuffer != 0){
				FreeAccountAdmissionDelay = (PlayersOnline - (MaxPlayers - PremiumPlayerBuffer * 2));
				FreeAccountAdmissionDelay = (FreeAccountAdmissionDelay * 30) / PremiumPlayerBuffer;
				if(FreeAccountAdmissionDelay < 0){
					FreeAccountAdmissionDelay = 0;
				}
			}

			uint32 FreeAccountAdmissionRound = RoundNr + (uint32)FreeAccountAdmissionDelay;
			if(EarliestFreeAccountAdmissionRound < FreeAccountAdmissionRound){
				EarliestFreeAccountAdmissionRound = FreeAccountAdmissionRound;
			}

			TConnection *Connection = GetFirstConnection();
			while(Connection != NULL){
				if(Connection->Live()){
					Connection->EmergencyPing();
				}
				Connection = GetNextConnection();
			}
		}
	}
}

void InitLoadHistory(void){
	for(int i = 0; i < NARRAY(LoadHistory); i += 1){
		LoadHistory[i] = 0;
	}
	LoadHistoryPointer = 0;
	TotalLoad = 0;
	TotalSend = 0;
	TotalRecv = 0;
	LagEnd = 0;
	EarliestFreeAccountAdmissionRound = 0;
	InitLog("netload");
}

void ExitLoadHistory(void){}

// Connection Output
static constexpr int GetPacketSize(int DataSize){
	return (((DataSize + 2) + 7) & ~7) + 2;
}

bool WriteToSocket(TConnection *Connection, uint8 *Buffer, int Size, int MaxSize){
	ASSERT(Size >= 4 && Size <= MaxSize && MaxSize <= UINT16_MAX);

	int DataSize = Size - 4;
	while((Size % 8) != 2 && Size < MaxSize){
		Buffer[Size] = (uint8)(rand() & 0xFF);
		Size += 1;
	}

	if((Size % 8) != 2){
		error("WriteToSocket: Failed to add padding (Size: %d, MaxSize: %d)\n", Size, MaxSize);
		return false;
	}

	TWriteBuffer WriteBuffer(Buffer, 4);
	WriteBuffer.writeWord((uint16)(Size - 2));
	WriteBuffer.writeWord((uint16)(DataSize));
	for(int i = 2; i < Size; i += 8){
		Connection->SymmetricKey.encrypt(&Buffer[i]);
	}

	int Attempts = 50;
	int BytesToWrite = Size;
	uint8 *WritePtr = Buffer;
	while(BytesToWrite > 0){
		int BytesWritten = (int)send(Connection->GetSocket(), (const char*)WritePtr, BytesToWrite, 0);
		if(BytesWritten > 0){
			BytesToWrite -= BytesWritten;
			WritePtr += BytesWritten;
		}else if(BytesWritten == 0){
			error("WriteToSocket: Error sending to Socket %d.\n", Connection->GetSocket());
			return false;
		}else{
#if OS_WINDOWS
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK && Attempts > 0){
				DelayThread(0, 10000);
				Attempts -= 1;
				continue;
			}
			return false;
#else
			if(errno == EAGAIN && Attempts > 0){
				DelayThread(0, 10000);
				Attempts -= 1;
				continue;
			}
			return false;
#endif
		}
	}

	NetLoad(PACKET_AVERAGE_SIZE_OVERHEAD + Size, true);
	return true;
}

bool SendLoginMessage(TConnection *Connection, int Type, const char *Message, int WaitingTime){
	if(Type != LOGIN_MESSAGE_ERROR
			&& Type != LOGIN_MESSAGE_PREMIUM
			&& Type != LOGIN_MESSAGE_WAITINGLIST){
		error("SendLoginMessage: Ungültiger Meldungstyp %d.\n", Type);
		return false;
	}

	if(Message == NULL){
		error("SendLoginMessage: Message ist NULL.\n");
		return false;
	}

	if(Type == LOGIN_MESSAGE_WAITINGLIST && (WaitingTime < 0 || WaitingTime > UINT8_MAX)){
		error("SendLoginMessage: Ungültige Wartezeit %d.\n", WaitingTime);
		return false;
	}

	if(strlen(Message) > 290){
		error("SendLoginMessage: Botschaft zu lang (%s).\n", Message);
		return false;
	}

	try{
		uint8 Data[GetPacketSize(300)];
		TWriteBuffer WriteBuffer(Data, sizeof(Data));
		WriteBuffer.writeWord(0); // EncryptedSize
		WriteBuffer.writeWord(0); // DataSize
		WriteBuffer.writeByte((uint8)Type);
		WriteBuffer.writeString(Message);
		if(Type == LOGIN_MESSAGE_WAITINGLIST){
			WriteBuffer.writeByte(WaitingTime);
		}

		return WriteToSocket(Connection, Data, WriteBuffer.Position, WriteBuffer.Size);
	}catch(const char *str){
		error("SendLoginMessage: Fehler beim Füllen des Puffers (%s)\n", str);
		return false;
	}
}

bool SendData(TConnection *Connection){
	if(Connection == NULL){
		error("SendData: Verbindung ist NULL.\n");
		return false;
	}

	int DataSize = Connection->NextToCommit - Connection->NextToSend;
	if(DataSize <= 0) return true;

	int PacketSize = GetPacketSize(DataSize);
	uint8 *Buffer = (uint8*)alloca(PacketSize);
	TWriteBuffer WriteBuffer(Buffer, PacketSize);
	WriteBuffer.writeWord(0); // EncryptedSize
	WriteBuffer.writeWord(0); // DataSize

	constexpr int OutDataSize = sizeof(Connection->OutData);
	int DataStart = Connection->NextToSend % OutDataSize;
	int DataEnd = DataStart + DataSize;
	if(DataEnd < OutDataSize){
		WriteBuffer.writeBytes(&Connection->OutData[DataStart], DataSize);
	}else{
		WriteBuffer.writeBytes(&Connection->OutData[DataStart], OutDataSize - DataStart);
		WriteBuffer.writeBytes(&Connection->OutData[0],         DataEnd - OutDataSize);
	}

	bool Result = WriteToSocket(Connection, Buffer, WriteBuffer.Position, WriteBuffer.Size);
	if(Result){
		Connection->NextToSend += DataSize;
	}
	return Result;
}

// Waiting List
bool GetWaitinglistEntry(const char *Name, uint32 *NextTry, bool *FreeAccount, bool *Newbie){
	bool Result = false;
	CommunicationThreadMutex.down();
	TWaitinglistEntry *Entry = WaitinglistHead;
	while(Entry != NULL){
		if(StringEqCI(Entry->Name, Name)){
			break;
		}
		Entry = Entry->Next;
	}

	if(Entry != NULL){
		*NextTry = Entry->NextTry;
		*FreeAccount = Entry->FreeAccount;
		*Newbie = Entry->Newbie;
		Result = true;
	}
	CommunicationThreadMutex.up();
	return Result;
}

void InsertWaitinglistEntry(const char *Name, uint32 NextTry, bool FreeAccount, bool Newbie){
	CommunicationThreadMutex.down();
	TWaitinglistEntry *Prev = NULL;
	TWaitinglistEntry *Entry = WaitinglistHead;
	while(Entry != NULL){
		if(StringEqCI(Entry->Name, Name)){
			break;
		}
		Prev = Entry;
		Entry = Entry->Next;
	}

	if(Entry == NULL){
		Entry = Waitinglist.getFreeItem();
		Entry->Next = NULL;
		strncpy(Entry->Name, Name, sizeof(Entry->Name) - 1);
		Entry->Name[sizeof(Entry->Name) - 1] = 0;
		Entry->Sleeping = false;
		if(Prev != NULL){
			Prev->Next = Entry;
		}else{
			WaitinglistHead = Entry;
		}
	}

	Entry->NextTry = NextTry;
	Entry->FreeAccount = FreeAccount;
	Entry->Newbie = Newbie;
	CommunicationThreadMutex.up();
}

void DeleteWaitinglistEntry(const char *Name){
	CommunicationThreadMutex.down();
	TWaitinglistEntry *Prev = NULL;
	TWaitinglistEntry *Entry = WaitinglistHead;
	while(Entry != NULL){
		if(StringEqCI(Entry->Name, Name)){
			if(Prev != NULL){
				Prev->Next = Entry->Next;
			}else{
				WaitinglistHead = Entry->Next;
			}
			Waitinglist.putFreeItem(Entry);
			break;
		}
		Prev = Entry;
		Entry = Entry->Next;
	}
	CommunicationThreadMutex.up();
}

int GetWaitinglistPosition(const char *Name, bool FreeAccount, bool Newbie){
	int FreeNewbies = 0;
	int FreeVeterans = 0;
	int PremiumNewbies = 0;
	int PremiumVeterans = 0;

	CommunicationThreadMutex.down();
	while(WaitinglistHead != NULL && RoundNr > (WaitinglistHead->NextTry + 60)){
		TWaitinglistEntry *Next = WaitinglistHead->Next;
		Waitinglist.putFreeItem(WaitinglistHead);
		WaitinglistHead = Next;
	}

	TWaitinglistEntry *Entry = WaitinglistHead;
	while(Entry != NULL){
		if(StringEqCI(Entry->Name, Name)){
			break;
		}

		if(!Entry->Sleeping){
			if(RoundNr > (Entry->NextTry + 5)){
				Entry->Sleeping = true;
			}else if(Entry->FreeAccount){
				if(Entry->Newbie){
					FreeNewbies += 1;
				}else{
					FreeVeterans += 1;
				}
			}else{
				if(Entry->Newbie){
					PremiumNewbies += 1;
				}else{
					PremiumVeterans += 1;
				}
			}
		}

		Entry = Entry->Next;
	}
	CommunicationThreadMutex.up();

	int Result = 1;
	if(FreeAccount){
		Result += PremiumVeterans + FreeVeterans;
		if(Newbie){
			Result += PremiumNewbies + FreeNewbies;
		}else if(GetNewbiesOnline() < (MaxNewbies - PremiumNewbieBuffer)){
			Result += FreeNewbies;
		}
	}else{
		Result += PremiumVeterans;
		if(Newbie || GetNewbiesOnline() < MaxNewbies){
			Result += PremiumNewbies;
		}
	}
	return Result;
}

int CheckWaitingTime(const char *Name, TConnection *Connection, bool FreeAccount, bool Newbie){
	int WaitingTime = 0;
	const char *Reason = NULL;
	int Position = GetWaitinglistPosition(Name, FreeAccount, Newbie);
	int PlayersOnline = GetPlayersOnline();
	int NewbiesOnline = GetNewbiesOnline();
	if((PlayersOnline + Position) > GetOrderBufferSpace()){
		Reason = "The server is overloaded.";
		WaitingTime = (Position / 2) + 10;
	}else if(FreeAccount){
		if(EarliestFreeAccountAdmissionRound > RoundNr){
			Reason = "The server is overloaded.\nOnly players with premium accounts\nare admitted at the moment.";
			WaitingTime = (int)(EarliestFreeAccountAdmissionRound - RoundNr) + Position / 2;
		}else if((PlayersOnline + Position) > (MaxPlayers - PremiumPlayerBuffer)){
			Reason = "Too many players online.\nOnly players with premium accounts\nare admitted at the moment.";
			WaitingTime = Position / 2 + 5;
		}else if(Newbie && (NewbiesOnline + Position) > (MaxNewbies - PremiumNewbieBuffer)){
			Reason = "There are too many players online\non the beginners' island, Rookgaard.\nOnly players with premium accounts\nare admitted at the moment.";
			WaitingTime = Position / 2 + 5;
		}
	}else{
		if((PlayersOnline + Position) > MaxPlayers){
			Reason = "There are too many players online.";
			WaitingTime = Position / 2 + 3;
		}else if(Newbie && (NewbiesOnline + Position) > MaxNewbies){
			Reason = "There are too many players online\non the beginners' island, Rookgaard.";
			WaitingTime = Position / 2 + 3;
		}
	}

	if(WaitingTime > 240){
		WaitingTime = 240;
	}

	if(WaitingTime > 0){
		char Message[250];
		snprintf(Message, sizeof(Message), "%s\n\nYou are at place %d on the waiting list.", Reason, Position);
		SendLoginMessage(Connection, LOGIN_MESSAGE_WAITINGLIST, Message, WaitingTime);
	}

	return WaitingTime;
}

// Connection Input
int ReadFromSocket(TConnection *Connection, uint8 *Buffer, int Size){
	int Attempts = 50;
	int BytesToRead = Size;
	uint8 *ReadPtr = Buffer;
	while(BytesToRead > 0){
		int BytesRead = (int)recv(Connection->GetSocket(), (char*)ReadPtr, BytesToRead, 0);
		if(BytesRead > 0){
			BytesToRead -= BytesRead;
			ReadPtr += BytesRead;
		}else if(BytesRead == 0){
			break;
		}else{
#if OS_WINDOWS
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){
				if(Attempts <= 0) return -ETIMEDOUT;
				if(BytesToRead == Size) return -EAGAIN;
				DelayThread(0, 10000);
				Attempts -= 1;
			} else {
				return -err;
			}
#else
			if(errno != EINTR){
				if(errno != EAGAIN){
					return -errno;
				}else if(Attempts <= 0){
					return -ETIMEDOUT;
				}else if(BytesToRead == Size){
					return -EAGAIN;
				}
				DelayThread(0, 10000);
				Attempts -= 1;
			}
#endif
		}
	}
	return Size - BytesToRead;
}

bool CallGameThread(TConnection *Connection){
	if(GameRunning()){
		Connection->WaitingForACK = true;
	}
	return true;
}

bool CheckConnection(TConnection *Connection){
	if(Connection == NULL || Connection->GetSocket() < 0){
		return false;
	}
	return Connection->ConnectionIsOk;
}

TPlayerData *PerformRegistration(TConnection *Connection, char *PlayerName,
		uint32 AccountID, const char *PlayerPassword, bool GamemasterClient){
	TQueryManagerPoolConnection QueryManagerConnection(&QueryManagerConnectionPool);
	if(!CheckConnection(Connection)){
		return NULL;
	}

	uint32 CharacterID = 0;
	int Sex = 0;
	char Guild[31] = "";
	char Rank[31] = "";
	char Title[31] = "";
	int NumberOfBuddies = 0;
	uint32 BuddyIDs[100] = {};
	char BuddyNames[100][30] = {};
	uint8 Rights[12] = {};
	bool PremiumAccountActivated = false;

	int LoginCode = QueryManagerConnection->loginGame(AccountID, PlayerName,
			PlayerPassword, Connection->GetIPAddress(), PrivateWorld, false,
			GamemasterClient, &CharacterID, &Sex, Guild, Rank, Title,
			&NumberOfBuddies, BuddyIDs, BuddyNames, Rights,
			&PremiumAccountActivated);

	if(LoginCode != 0){
		SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR, "Account number or password is not correct.", -1);
		return NULL;
	}

	if(PremiumAccountActivated){
		SendLoginMessage(Connection, LOGIN_MESSAGE_PREMIUM,
				"Your Premium Account is now activated.\nHave a lot of fun in Tibia.", -1);
	}

	Log("game", "Spieler %s loggt ein an Socket %d von %s.\n",
			PlayerName, Connection->GetSocket(), Connection->GetIPAddress());

	TPlayerData *PlayerData = AssignPlayerPoolSlot(CharacterID, true);
	if(PlayerData == NULL){
		QueryManagerConnection->decrementIsOnline(CharacterID);
		SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR,
				"There are too many players online.\nPlease try again later.", -1);
		return NULL;
	}

	PlayerData->AccountID = AccountID;
	PlayerData->Sex = Sex;
	strcpy(PlayerData->Name, PlayerName);
	memcpy(PlayerData->Rights, Rights, sizeof(Rights));
	strcpy(PlayerData->Guild, Guild);
	strcpy(PlayerData->Rank, Rank);
	strcpy(PlayerData->Title, Title);

	ReleasePlayerPoolSlot(PlayerData);

	return PlayerData;
}

bool HandleLogin(TConnection *Connection){
	TReadBuffer InputBuffer(Connection->InData, Connection->InDataSize);
	uint8 Command = InputBuffer.readByte();
	if(Command != CL_CMD_LOGIN_REQUEST){
		return false;
	}

	int TerminalType = 0;
	int TerminalVersion = 770;
	bool GamemasterClient = false;
	uint32 AccountID = 0;
	char PlayerName[30] = "";
	char PlayerPassword[30] = "";

	try{
#if TIBIA772
		TerminalType = (int)InputBuffer.readWord();
		TerminalVersion = (int)InputBuffer.readWord();
		TerminalType = (TerminalType & 0xFF);
		if(TerminalType == 0 || (TerminalType != 1 && TerminalType != 2)){
			TerminalType = 2;
		}
#endif
		uint8 AsymmetricData[128];
		InputBuffer.readBytes(AsymmetricData, 128);

		RSAMutex.down();
		bool decOk = PrivateKey.decrypt(AsymmetricData);
		RSAMutex.up();

		if(!decOk || AsymmetricData[0] != 0){
			SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR, "Login failed due to corrupt data.", -1);
			return false;
		}

		TReadBuffer ReadBuffer(AsymmetricData, 128);
		ReadBuffer.readByte(); // 0
		uint32 k0 = ReadBuffer.readQuad();
		uint32 k1 = ReadBuffer.readQuad();
		uint32 k2 = ReadBuffer.readQuad();
		uint32 k3 = ReadBuffer.readQuad();
		Connection->SymmetricKey.init(k0, k1, k2, k3);

#if !TIBIA772
		TerminalType = (int)ReadBuffer.readWord();
		TerminalVersion = (int)ReadBuffer.readWord();
#endif
		GamemasterClient = ReadBuffer.readByte() != 0;
		AccountID = ReadBuffer.readQuad();
		ReadBuffer.readString(PlayerName, sizeof(PlayerName));
		ReadBuffer.readString(PlayerPassword, sizeof(PlayerPassword));
	}catch(...){
		SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR, "Login failed due to corrupt data.", -1);
		return false;
	}

	if(PlayerName[0] == 0){
		SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR, "You must enter a character name.", -1);
		return false;
	}

	if(!GameRunning()){
		SendLoginMessage(Connection, LOGIN_MESSAGE_ERROR, "The server is not online.\nPlease try again later.", -1);
		return false;
	}

	print(1, "GameServer HandleLogin: Account=%u, Player=%s, GM=%d\n", AccountID, PlayerName, GamemasterClient ? 1 : 0);

	TPlayerData *Slot = PerformRegistration(Connection, PlayerName, AccountID, PlayerPassword, GamemasterClient);
	if(Slot == NULL){
		print(1, "GameServer PerformRegistration failed for Player=%s!\n", PlayerName);
		return false;
	}

	TWriteBuffer WriteBuffer(Connection->InData + 2, sizeof(Connection->InData) - 2);
	WriteBuffer.writeByte(CL_CMD_LOGIN);
	WriteBuffer.writeWord((int)TerminalType);
	WriteBuffer.writeWord((int)TerminalVersion);
	WriteBuffer.writeQuad(Slot->CharacterID);

	Connection->NextToSend = 0;
	Connection->NextToCommit = 0;
	Connection->InDataSize = WriteBuffer.Position;
	Connection->NextToWrite = 0;

	Connection->Login();
	return CallGameThread(Connection);
}

bool ReceiveCommand(TConnection *Connection){
	if(Connection == NULL) return false;

	while(!Connection->WaitingForACK){
		uint8 Help[2];
		int BytesRead = ReadFromSocket(Connection, Help, 2);
		if(BytesRead == 0){
			return false;
		}else if(BytesRead < 0){
			return (BytesRead == -EAGAIN);
		}

		if(BytesRead != 2){
			return false;
		}

		int Size = ((uint16)Help[0] | ((uint16)Help[1] << 8));
		if(Size == 0 || Size > (int)sizeof(Connection->InData)){
			return false;
		}

		BytesRead = ReadFromSocket(Connection, &Connection->InData[0], Size);
		if(BytesRead != Size){
			return false;
		}

		NetLoad(PACKET_AVERAGE_SIZE_OVERHEAD + BytesRead, false);

		if(Connection->State == CONNECTION_CONNECTED){
			Connection->StopLoginTimer();
			Connection->InDataSize = Size;
			if(!HandleLogin(Connection)){
				return false;
			}
		}else{
			if((Size % 8) != 0){
				return false;
			}

			for(int i = 0; i < Size; i += 8){
				Connection->SymmetricKey.decrypt(&Connection->InData[i]);
			}

			int PlainSize = ((uint16)Connection->InData[0]) | ((uint16)Connection->InData[1] << 8);
			if(PlainSize == 0 || (PlainSize + 2) > Size){
				return false;
			}

			Connection->InDataSize = PlainSize;
			if(!CallGameThread(Connection)){
				return false;
			}
		}
	}

	Connection->SigIOPending = true;
	return true;
}

void IncrementActiveConnections(void){
	CommunicationThreadMutex.down();
	ActiveConnections += 1;
	CommunicationThreadMutex.up();
}

void DecrementActiveConnections(void){
	CommunicationThreadMutex.down();
	ActiveConnections -= 1;
	CommunicationThreadMutex.up();
}

void CommunicationThread(socket_t Socket){
	TConnection *Connection = AssignFreeConnection();
	if(Connection == NULL){
		SocketClose(Socket);
		return;
	}

	Connection->ThreadID = gettid();
	Connection->Connect((int)Socket);
	Connection->WaitingForACK = false;

	SocketSetNonBlocking(Socket);

	int NoDelay = 1;
	setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&NoDelay, sizeof(NoDelay));

	if(!Connection->SetLoginTimer(5)){
		SocketClose(Socket);
		Connection->Free();
		return;
	}

	if(!ReceiveCommand(Connection)){
		Connection->Close(true);
	}

	while(GameRunning() && Connection->ConnectionIsOk){
		pollfd Pfd = {};
		Pfd.fd = Socket;
		Pfd.events = POLLIN;

		int Res = poll(&Pfd, 1, 20);
		if(Res > 0){
			if(Pfd.revents & (POLLERR | POLLHUP)){
				Connection->Close(false);
				break;
			}
			if((Pfd.revents & POLLIN) && !Connection->WaitingForACK){
				if(!ReceiveCommand(Connection)){
					Connection->Close(true);
					break;
				}
			}
		} else if(Res < 0){
			if(!GameRunning()) break;
		}
	}

	while(Connection->Live()){
		DelayThread(0, 50000);
	}

	if(Connection->ClosingIsDelayed){
		DelayThread(1, 0);
	}

	SocketClose(Socket);
	Connection->Free();
}

int HandleConnection(void *Data){
	socket_t Socket = (socket_t)((uintptr)Data);
	try{
		CommunicationThread(Socket);
	}catch(...){}
	DecrementActiveConnections();
	return 0;
}

bool OpenSocket(void){
	TCPSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(!SOCKET_IS_VALID(TCPSocket)){
		return false;
	}

	SocketSetReuseAddr(TCPSocket);

	sockaddr_in ServerAddress = {};
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons((uint16)GamePort);
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(TCPSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) != 0){
		error("LaunchServer: Failed to bind Game Server on port %d.\n", GamePort);
		return false;
	}

	if(listen(TCPSocket, 512) != 0){
		error("LaunchServer: Failed to listen on Game Server socket.\n");
		return false;
	}

	return true;
}

int AcceptorThreadLoop(void *Unused){
	AcceptorThreadID = gettid();
	print(1, "Game Server waiting for connections on port %d...\n", GamePort);
	while(GameRunning()){
		pollfd Pfd = {};
		Pfd.fd = TCPSocket;
		Pfd.events = POLLIN;

		int Res = poll(&Pfd, 1, 500);
		if(Res <= 0) continue;

		if(Pfd.revents & POLLIN){
			sockaddr_in ClientAddr = {};
			socklen_t AddrLen = sizeof(ClientAddr);
			socket_t ClientSock = accept(TCPSocket, (sockaddr*)&ClientAddr, &AddrLen);
			if(!SOCKET_IS_VALID(ClientSock)) continue;

			if(ActiveConnections >= MAX_COMMUNICATION_THREADS){
				SocketClose(ClientSock);
				continue;
			}

			IncrementActiveConnections();
			StartThread(HandleConnection, (void*)((uintptr)ClientSock), true);
		}
	}

	AcceptorThreadID = 0;
	return 0;
}

void InitCommunication(void){
	InitLoadHistory();
	WaitinglistHead = NULL;
	TCPSocket = SOCKET_INVALID;
	AcceptorThread = INVALID_THREAD_HANDLE;
	AcceptorThreadID = 0;
	ActiveConnections = 0;

	if(!PrivateKey.initFromFile("tibia.pem")){
		throw "cannot load RSA key";
	}

	if(!OpenSocket()){
		throw "cannot open socket";
	}

	AcceptorThread = StartThread(AcceptorThreadLoop, NULL, false);
}

void ExitCommunication(void){
	if(SOCKET_IS_VALID(TCPSocket)){
		SocketClose(TCPSocket);
	}

	if(AcceptorThread != INVALID_THREAD_HANDLE){
		JoinThread(AcceptorThread);
		AcceptorThread = INVALID_THREAD_HANDLE;
	}

	ExitLoadHistory();
}
