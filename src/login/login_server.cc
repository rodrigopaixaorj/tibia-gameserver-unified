#include "login_server.hh"
#include "../crypto/crypto.hh"
#include "../database/db.hh"
#include "../database/db_common.hh"
#include "../game/config.hh"

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>

static std::atomic<bool> g_LoginRunning(false);
static std::thread g_LoginThread;
static socket_t g_LoginSocket = SOCKET_INVALID;
static std::string g_MotdText = "Welcome to CipSoft 7.72 Unified Server!";
static RSAKey *g_LoginRSA = nullptr;

struct TBufferWriter {
	uint8 *Buffer;
	int Capacity;
	int Position;

	TBufferWriter(uint8 *Buf, int Cap) : Buffer(Buf), Capacity(Cap), Position(0) {}

	void Write8(uint8 v){
		if(Position + 1 <= Capacity){
			Buffer[Position++] = v;
		}
	}
	void Write16(uint16 v){
		if(Position + 2 <= Capacity){
			BufferWrite16LE(Buffer + Position, v);
			Position += 2;
		}
	}
	void Write32LE(uint32 v){
		if(Position + 4 <= Capacity){
			BufferWrite32LE(Buffer + Position, v);
			Position += 4;
		}
	}
	void Write64LE(uint64 v){
		Write32LE((uint32)(v & 0xFFFFFFFF));
		Write32LE((uint32)(v >> 32));
	}
	void Write32BE(uint32 v){
		if(Position + 4 <= Capacity){
			BufferWrite32BE(Buffer + Position, v);
			Position += 4;
		}
	}
	void WriteString(const char *str){
		int len = str ? (int)strlen(str) : 0;
		Write16((uint16)len);
		if(len > 0 && Position + len <= Capacity){
			memcpy(Buffer + Position, str, len);
			Position += len;
		}
	}
};

static void SendLoginXTEAResponse(socket_t ClientSock, const uint32 *XTEAKey, const uint8 *Payload, int PayloadSize){
	int DataSize = PayloadSize;
	int EncryptedSize = DataSize + 2; // +2 for DataSize U16
	int Padding = 0;
	if((EncryptedSize % 8) != 0){
		Padding = 8 - (EncryptedSize % 8);
		EncryptedSize += Padding;
	}

	std::vector<uint8> OutBuffer(EncryptedSize + 2);
	BufferWrite16LE(OutBuffer.data(), (uint16)EncryptedSize);
	BufferWrite16LE(OutBuffer.data() + 2, (uint16)DataSize);
	memcpy(OutBuffer.data() + 4, Payload, PayloadSize);
	if(Padding > 0){
		memset(OutBuffer.data() + 4 + PayloadSize, 0x33, Padding);
	}

	XTEAEncrypt(XTEAKey, OutBuffer.data() + 2, EncryptedSize);
	int TotalToSend = EncryptedSize + 2;
	send(ClientSock, (const char*)OutBuffer.data(), TotalToSend, 0);
}

static void HandleLoginRequest(socket_t ClientSock, const uint8 *Packet, int PacketSize, const char *IPString){
	if(PacketSize < 145){
		return;
	}

	uint8 Opcode = Packet[0];
	if(Opcode != 1){
		return;
	}

	uint16 TerminalType = BufferRead16LE(Packet + 1);
	uint16 TerminalVersion = BufferRead16LE(Packet + 3);
	(void)TerminalType;
	(void)TerminalVersion;

	// Decrypt 128-byte RSA block (starting at offset 17)
	uint8 RSABlock[128];
	memcpy(RSABlock, Packet + 17, 128);

	bool ok = RSADecrypt(g_LoginRSA, RSABlock, 128);
	LOG("HandleLoginRequest: RSADecrypt return=%d, byte[0]=0x%02X byte[1]=0x%02X byte[17]=0x%02X", (int)ok, RSABlock[0], RSABlock[1], RSABlock[17]);
	if(!ok || RSABlock[0] != 0){
		LOG_ERR("Failed to decrypt RSA login data from %s", IPString);
		return;
	}

	uint32 XTEAKey[4];
	XTEAKey[0] = BufferRead32LE(RSABlock + 1);
	XTEAKey[1] = BufferRead32LE(RSABlock + 5);
	XTEAKey[2] = BufferRead32LE(RSABlock + 9);
	XTEAKey[3] = BufferRead32LE(RSABlock + 13);

	uint32 AccountID = BufferRead32LE(RSABlock + 17);
	uint16 PwdLen = BufferRead16LE(RSABlock + 21);
	char Password[64];
	if(PwdLen >= sizeof(Password)) PwdLen = sizeof(Password) - 1;
	memcpy(Password, RSABlock + 23, PwdLen);
	Password[PwdLen] = 0;

	LOG("Login request from %s: Account=%u", IPString, AccountID);

	TCharacterLoginData Characters[128];
	int NumCharacters = 0;
	int PremiumDays = 0;
	int LoginResult = DB_LoginAccount(AccountID, Password, IPString, 128, &NumCharacters, Characters, &PremiumDays);

	uint8 ResponsePayload[8192];
	TBufferWriter Writer(ResponsePayload, sizeof(ResponsePayload));

	if(LoginResult == 0){
		if(FreePremium && PremiumDays < 365){
			PremiumDays = 365;
		}

		// MOTD (Opcode 20 = 0x14)
		if(!g_MotdText.empty()){
			Writer.Write8(20);
			Writer.WriteString(g_MotdText.c_str());
		}

		// Character List (Opcode 100 = 0x64)
		Writer.Write8(100);
		Writer.Write8((uint8)NumCharacters);
		for(int i = 0; i < NumCharacters; i++){
			Writer.WriteString(Characters[i].Name);
			Writer.WriteString(Characters[i].WorldName);
			uint32 Addr = Characters[i].WorldAddress;
			if(WorldAddress[0] != 0 && strcmp(WorldAddress, "0.0.0.0") != 0 && strcmp(WorldAddress, "127.0.0.1") != 0){
				Addr = DB_ParseIPv4(WorldAddress);
			}
			Writer.Write32BE(Addr);
			Writer.Write16((uint16)Characters[i].WorldPort);
		}
		Writer.Write16((uint16)PremiumDays);

		SendLoginXTEAResponse(ClientSock, XTEAKey, ResponsePayload, Writer.Position);
	} else {
		// Error (Opcode 10 = 0x0A)
		Writer.Write8(10);
		if(LoginResult == 2){
			Writer.WriteString("Password is not correct.");
		} else {
			Writer.WriteString("Account number is not correct.");
		}
		SendLoginXTEAResponse(ClientSock, XTEAKey, ResponsePayload, Writer.Position);
	}
}

enum RequestedInfo_t : uint16 {
	REQUEST_BASIC_SERVER_INFO = 1 << 0,
	REQUEST_OWNER_SERVER_INFO = 1 << 1,
	REQUEST_MISC_SERVER_INFO = 1 << 2,
	REQUEST_PLAYERS_INFO = 1 << 3,
	REQUEST_MAP_INFO = 1 << 4,
	REQUEST_EXT_PLAYERS_INFO = 1 << 5,
	REQUEST_PLAYER_STATUS_INFO = 1 << 6,
	REQUEST_SERVER_SOFTWARE_INFO = 1 << 7,
};

static std::unordered_map<uint32_t, int64> g_StatusIpMap;
static std::mutex g_StatusIpMutex;
static int g_PeakPlayersOnline = 0;

static bool CheckStatusFlood(uint32_t ClientIP){
	if(ClientIP == 0x0100007F){ // 127.0.0.1 always allowed without rate limit
		return true;
	}
	int timeoutMs = (StatusTimeout > 0 ? StatusTimeout : 5000);
	int64 nowMs = GetClockMonotonicMS();
	std::lock_guard<std::mutex> lock(g_StatusIpMutex);
	auto it = g_StatusIpMap.find(ClientIP);
	if(it != g_StatusIpMap.end() && (nowMs < (it->second + timeoutMs))){
		return false;
	}
	g_StatusIpMap[ClientIP] = nowMs;

	// Prevent memory leak under traffic from thousands of unique scanners
	if(g_StatusIpMap.size() > 4096){
		for(auto mapIt = g_StatusIpMap.begin(); mapIt != g_StatusIpMap.end(); ){
			if(nowMs > (mapIt->second + timeoutMs * 2)){
				mapIt = g_StatusIpMap.erase(mapIt);
			} else {
				++mapIt;
			}
		}
	}
	return true;
}

static void HandleStatusRequest(socket_t ClientSock, uint32_t ClientIP){
	if(!CheckStatusFlood(ClientIP)){
		return;
	}

	int online = GetPlayersOnline();
	if(online > g_PeakPlayersOnline){
		g_PeakPlayersOnline = online;
	}

	int maxP = (MaxPlayers > 0 ? MaxPlayers : 1000);
	const char *srvName = (ServerName[0] ? ServerName : (WorldName[0] ? WorldName : "Tibia"));
	const char *srvLoc = (ServerLocation[0] ? ServerLocation : "Brazil");
	const char *srvUrl = (ServerURL[0] ? ServerURL : "");
	const char *ownName = (OwnerName[0] ? OwnerName : "Admin");
	const char *ownMail = (OwnerEmail[0] ? OwnerEmail : "");
	const char *motd = (MotdText[0] ? MotdText : g_MotdText.c_str());
	const char *ipAddr = (WorldAddress[0] ? WorldAddress : "127.0.0.1");
	int port = (LoginPort > 0 ? LoginPort : 7171);

	char Response[2048];
	int len = snprintf(Response, sizeof(Response),
		"<?xml version=\"1.0\"?>\n"
		"<tsqp version=\"1.0\">\n"
		"  <serverinfo uptime=\"%d\" ip=\"%s\" servername=\"%s\" port=\"%d\" location=\"%s\" url=\"%s\" server=\"CipSoft 7.72 Unified\" version=\"7.72\" client=\"7.72\"/>\n"
		"  <owner name=\"%s\" email=\"%s\"/>\n"
		"  <players online=\"%d\" max=\"%d\" peak=\"%d\"/>\n"
		"  <monsters total=\"1000\"/>\n"
		"  <npcs total=\"150\"/>\n"
		"  <rates experience=\"%d\" skill=\"%d\" loot=\"%d\" magic=\"%d\" spawn=\"%d\"/>\n"
		"  <map name=\"%s\" author=\"CipSoft\" width=\"2048\" height=\"2048\"/>\n"
		"  <motd>%s</motd>\n"
		"</tsqp>",
		GetMonotonicUptime(),
		ipAddr,
		srvName,
		port,
		srvLoc,
		srvUrl,
		ownName,
		ownMail,
		online,
		maxP,
		g_PeakPlayersOnline,
		RateExp > 0 ? RateExp : 1,
		RateSkill > 0 ? RateSkill : 1,
		RateLoot > 0 ? RateLoot : 1,
		RateMagic > 0 ? RateMagic : 1,
		RateSpawn > 0 ? RateSpawn : 1,
		WorldName[0] ? WorldName : "Tibia",
		motd
	);

	if(len > 0){
		send(ClientSock, Response, len, 0);
	}
}

static void HandleBinaryStatusRequest(socket_t ClientSock, uint32_t ClientIP, uint16 requestedInfo){
	if(!CheckStatusFlood(ClientIP)){
		return;
	}

	int online = GetPlayersOnline();
	if(online > g_PeakPlayersOnline){
		g_PeakPlayersOnline = online;
	}

	uint8 buffer[2048];
	TBufferWriter writer(buffer + 2, sizeof(buffer) - 2);

	int maxP = (MaxPlayers > 0 ? MaxPlayers : 1000);
	const char *srvName = (ServerName[0] ? ServerName : (WorldName[0] ? WorldName : "Tibia"));
	const char *srvLoc = (ServerLocation[0] ? ServerLocation : "Brazil");
	const char *srvUrl = (ServerURL[0] ? ServerURL : "");
	const char *ownName = (OwnerName[0] ? OwnerName : "Admin");
	const char *ownMail = (OwnerEmail[0] ? OwnerEmail : "");
	const char *motd = (MotdText[0] ? MotdText : g_MotdText.c_str());
	const char *ipAddr = (WorldAddress[0] ? WorldAddress : "127.0.0.1");
	int port = (LoginPort > 0 ? LoginPort : 7171);

	if(requestedInfo & REQUEST_BASIC_SERVER_INFO){
		writer.Write8(0x10);
		writer.WriteString(srvName);
		writer.WriteString(ipAddr);
		char portStr[16];
		snprintf(portStr, sizeof(portStr), "%d", port);
		writer.WriteString(portStr);
	}

	if(requestedInfo & REQUEST_OWNER_SERVER_INFO){
		writer.Write8(0x11);
		writer.WriteString(ownName);
		writer.WriteString(ownMail);
	}

	if(requestedInfo & REQUEST_MISC_SERVER_INFO){
		writer.Write8(0x12);
		writer.WriteString(motd);
		writer.WriteString(srvLoc);
		writer.WriteString(srvUrl);
		writer.Write64LE((uint64)GetMonotonicUptime());
	}

	if(requestedInfo & REQUEST_PLAYERS_INFO){
		writer.Write8(0x20);
		writer.Write32LE((uint32)online);
		writer.Write32LE((uint32)maxP);
		writer.Write32LE((uint32)g_PeakPlayersOnline);
	}

	if(requestedInfo & REQUEST_MAP_INFO){
		writer.Write8(0x30);
		writer.WriteString(WorldName[0] ? WorldName : "world");
		writer.WriteString("CipSoft");
		writer.Write16(2048);
		writer.Write16(2048);
	}

	if(requestedInfo & REQUEST_SERVER_SOFTWARE_INFO){
		writer.Write8(0x23);
		writer.WriteString("CipSoft 7.72 Unified");
		writer.WriteString("7.72");
		writer.WriteString("7.72");
	}

	uint16 payloadSize = (uint16)writer.Position;
	BufferWrite16LE(buffer, payloadSize);
	send(ClientSock, (const char*)buffer, payloadSize + 2, 0);
}

static void LoginServerWorker(int Port, std::string BindIP){
	g_LoginSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(!SOCKET_IS_VALID(g_LoginSocket)){
		LOG_ERR("Failed to create Login Server listening socket");
		return;
	}

	SocketSetReuseAddr(g_LoginSocket);

	sockaddr_in Addr = {};
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons((uint16)Port);
	Addr.sin_addr.s_addr = (BindIP == "0.0.0.0" || BindIP.empty()) ? htonl(INADDR_ANY) : inet_addr(BindIP.c_str());

	if(bind(g_LoginSocket, (sockaddr*)&Addr, sizeof(Addr)) != 0){
#if OS_WINDOWS
		int err = WSAGetLastError();
#else
		int err = errno;
#endif
		LOG_ERR("Failed to bind Login Server on port %d (error %d)", Port, err);
		SocketClose(g_LoginSocket);
		return;
	}

	if(listen(g_LoginSocket, 64) != 0){
		LOG_ERR("Failed to listen on Login Server on port %d", Port);
		SocketClose(g_LoginSocket);
		return;
	}

	while(g_LoginRunning.load()){
		pollfd Pfd = {};
		Pfd.fd = g_LoginSocket;
		Pfd.events = POLLIN;

		int PollRes = poll(&Pfd, 1, 500);
		if(PollRes <= 0) continue;

		if(Pfd.revents & POLLIN){
			sockaddr_in ClientAddr = {};
			socklen_t AddrLen = sizeof(ClientAddr);
			socket_t ClientSock = accept(g_LoginSocket, (sockaddr*)&ClientAddr, &AddrLen);
			if(!SOCKET_IS_VALID(ClientSock)) continue;

			// 3-second read/write timeout to mitigate DoS / hanging connections
#if defined(_WIN32)
			DWORD timeoutMs = 3000;
			setsockopt(ClientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
			setsockopt(ClientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
#else
			struct timeval tv;
			tv.tv_sec = 3;
			tv.tv_usec = 0;
			setsockopt(ClientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
			setsockopt(ClientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
#endif

			char IPString[64];
			snprintf(IPString, sizeof(IPString), "%s:%d", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
			uint32 clientIP = (uint32)ClientAddr.sin_addr.s_addr;

			// Read initial packet (up to 1024 bytes)
			uint8 InitialBuffer[1024];
			int r = recv(ClientSock, (char*)InitialBuffer, sizeof(InitialBuffer), 0);
			if(r >= 2){
				// Case 1: Status requests in RAW format (without 2-byte length header)
				// Used by otservlist crawlers, PHP scripts, and fast pingers:
				// \xFF\xFFinfo, \xFFinfo, "info"
				if((r >= 6 && InitialBuffer[0] == 0xFF && InitialBuffer[1] == 0xFF && memcmp(InitialBuffer + 2, "info", 4) == 0) ||
				   (r >= 5 && InitialBuffer[0] == 0xFF && memcmp(InitialBuffer + 1, "info", 4) == 0) ||
				   (r >= 4 && memcmp(InitialBuffer, "info", 4) == 0) ||
				   (InitialBuffer[0] == 0xFF && InitialBuffer[1] == 0xFF && r <= 6)){
					HandleStatusRequest(ClientSock, clientIP);
				} else {
					// Case 2: Standard Tibia packet with 2-byte length header (LE)
					uint16 PacketSize = BufferRead16LE(InitialBuffer);
					if(PacketSize > 0 && PacketSize <= 2048){
						std::vector<uint8> PacketData(PacketSize);
						int have = r - 2;
						if(have > PacketSize) have = PacketSize;
						if(have > 0){
							memcpy(PacketData.data(), InitialBuffer + 2, have);
						}
						int totalRead = have;
						while(totalRead < PacketSize){
							int chunk = recv(ClientSock, (char*)PacketData.data() + totalRead, PacketSize - totalRead, 0);
							if(chunk <= 0) break;
							totalRead += chunk;
						}

						if(totalRead == PacketSize){
							if(PacketData[0] == 0x01 && PacketSize > 10){
								// Tibia 7.72 client login request
								HandleLoginRequest(ClientSock, PacketData.data(), PacketSize, IPString);
							} else if(PacketData[0] == 0xFF){
								// Status protocol with length header
								if(PacketSize >= 4 && PacketData[1] == 0x01){
									// Binary status protocol (0x01)
									uint16 requestedInfo = BufferRead16LE(PacketData.data() + 2);
									HandleBinaryStatusRequest(ClientSock, clientIP, requestedInfo);
								} else {
									// XML status protocol (0xFF)
									HandleStatusRequest(ClientSock, clientIP);
								}
							}
						}
					}
				}
			}
			SocketClose(ClientSock);
		}
	}

	SocketClose(g_LoginSocket);
}

bool LoginServerStart(int Port, const char *BindIP, const char *Motd){
	if(g_LoginRunning.load()) return true;

	if(Motd) g_MotdText = Motd;

	if(g_LoginRSA == nullptr){
		g_LoginRSA = RSALoadPEM("tibia.pem");
	}

	g_LoginRunning.store(true);
	g_LoginThread = std::thread(LoginServerWorker, Port, BindIP ? BindIP : "0.0.0.0");
	return true;
}

void LoginServerStop(void){
	if(!g_LoginRunning.load()) return;
	g_LoginRunning.store(false);
	if(SOCKET_IS_VALID(g_LoginSocket)){
		SocketClose(g_LoginSocket);
	}
	if(g_LoginThread.joinable()){
		g_LoginThread.join();
	}
	if(g_LoginRSA != nullptr){
		RSAFree(g_LoginRSA);
		g_LoginRSA = nullptr;
	}
}

bool LoginServerIsRunning(void){
	return g_LoginRunning.load();
}
