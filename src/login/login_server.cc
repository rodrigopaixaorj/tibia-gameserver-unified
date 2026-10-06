#include "login_server.hh"
#include "../crypto/crypto.hh"
#include "../database/db.hh"

#include <string>
#include <vector>
#include <thread>
#include <atomic>

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
	int EncryptedSize = DataSize + 2; // +2 para DataSize U16
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
			Writer.Write32BE(Characters[i].WorldAddress);
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

static void HandleStatusRequest(socket_t ClientSock){
	char Response[512];
	int PlayersOnline = 0;
	int MaxPlayers = 1000;
	int len = snprintf(Response, sizeof(Response),
		"<?xml version=\"1.0\"?>\n"
		"<tsqp version=\"1.0\">\n"
		"  <serverinfo uptime=\"%d\" ip=\"127.0.0.1\" port=\"7171\" location=\"Local\" url=\"\" server=\"CipSoft 7.72 Unified\" version=\"7.72\"/>\n"
		"  <owner name=\"Admin\" email=\"admin@tibia.local\"/>\n"
		"  <players online=\"%d\" max=\"%d\" peak=\"%d\"/>\n"
		"  <monsters total=\"1000\"/>\n"
		"</tsqp>",
		GetMonotonicUptime(), PlayersOnline, MaxPlayers, PlayersOnline);

	send(ClientSock, Response, len, 0);
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
		LOG_ERR("Failed to bind Login Server on port %d", Port);
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

			// Set 3-second receive timeout to prevent DoS from slow/hanging connections
#if defined(_WIN32)
			DWORD timeoutMs = 3000;
			setsockopt(ClientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
#else
			struct timeval tv;
			tv.tv_sec = 3;
			tv.tv_usec = 0;
			setsockopt(ClientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif

			char IPString[64];
			snprintf(IPString, sizeof(IPString), "%s:%d", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));

			// Read packet size (2 bytes)
			uint8 Header[2];
			int r = recv(ClientSock, (char*)Header, 2, 0);
			if(r == 2){
				uint16 PacketSize = BufferRead16LE(Header);
				if(PacketSize > 0 && PacketSize <= 2048){
					uint8 *PacketData = (uint8*)malloc(PacketSize);
					if(PacketData){
						int TotalRead = 0;
						while(TotalRead < PacketSize){
							int chunk = recv(ClientSock, (char*)PacketData + TotalRead, PacketSize - TotalRead, 0);
							if(chunk <= 0) break;
							TotalRead += chunk;
						}

						if(TotalRead == PacketSize){
							if(PacketData[0] == 0x01){
								HandleLoginRequest(ClientSock, PacketData, PacketSize, IPString);
							} else if(PacketData[0] == 0xFF){
								HandleStatusRequest(ClientSock);
							}
						}
						free(PacketData);
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
