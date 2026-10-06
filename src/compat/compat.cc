#include "compat.hh"

static int64 g_StartTimeMS = 0;

void LogAdd(const char *Prefix, const char *Format, ...){
	char Entry[4096];
	va_list ap;
	va_start(ap, Format);
	vsnprintf(Entry, sizeof(Entry), Format, ap);
	va_end(ap);

	int Length = (int)strlen(Entry);
	while(Length > 0 && isspace(Entry[Length - 1])){
		Entry[Length - 1] = 0;
		Length -= 1;
	}

	if(Length > 0){
		char TimeString[128];
		StringBufFormatTime(TimeString, "%Y-%m-%d %H:%M:%S", (int)time(NULL));
		fprintf(stdout, "%s [%s] %s\n", TimeString, Prefix, Entry);
		fflush(stdout);
	}
}

void LogAddVerbose(const char *Prefix, const char *Function, const char *File, int Line, const char *Format, ...){
	char Entry[4096];
	va_list ap;
	va_start(ap, Format);
	vsnprintf(Entry, sizeof(Entry), Format, ap);
	va_end(ap);

	int Length = (int)strlen(Entry);
	while(Length > 0 && isspace(Entry[Length - 1])){
		Entry[Length - 1] = 0;
		Length -= 1;
	}

	if(Length > 0){
		(void)File;
		(void)Line;
		char TimeString[128];
		StringBufFormatTime(TimeString, "%Y-%m-%d %H:%M:%S", (int)time(NULL));
		fprintf(stdout, "%s [%s] %s: %s\n", TimeString, Prefix, Function, Entry);
		fflush(stdout);
	}
}

bool SocketSystemInit(void){
#if OS_WINDOWS
	WSADATA WsaData;
	int Res = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if(Res != 0){
		LOG_ERR("WSAStartup failed with error %d", Res);
		return false;
	}
#endif
	g_StartTimeMS = GetClockMonotonicMS();
	return true;
}

void SocketSystemExit(void){
#if OS_WINDOWS
	WSACleanup();
#endif
}

void SocketClose(socket_t &Socket){
	if(SOCKET_IS_VALID(Socket)){
#if OS_WINDOWS
		closesocket(Socket);
#else
		close(Socket);
#endif
		Socket = SOCKET_INVALID;
	}
}

bool SocketSetNonBlocking(socket_t Socket){
#if OS_WINDOWS
	u_long Mode = 1;
	return ioctlsocket(Socket, FIONBIO, &Mode) == 0;
#else
	int Flags = fcntl(Socket, F_GETFL);
	if(Flags == -1) return false;
	return fcntl(Socket, F_SETFL, Flags | O_NONBLOCK) != -1;
#endif
}

bool SocketSetReuseAddr(socket_t Socket){
	int Reuse = 1;
	return setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&Reuse, sizeof(Reuse)) == 0;
}

int64 GetClockMonotonicMS(void){
	auto Now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()).count();
}

int GetMonotonicUptime(void){
	if(g_StartTimeMS == 0){
		g_StartTimeMS = GetClockMonotonicMS();
	}
	return (int)((GetClockMonotonicMS() - g_StartTimeMS) / 1000);
}

void SleepMS(int Milliseconds){
	std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
}

struct tm GetLocalTime(time_t t){
	struct tm result;
#if COMPILER_MSVC
	localtime_s(&result, &t);
#else
	localtime_r(&t, &result);
#endif
	return result;
}

struct tm GetGMTime(time_t t){
	struct tm result;
#if COMPILER_MSVC
	gmtime_s(&result, &t);
#else
	gmtime_r(&t, &result);
#endif
	return result;
}

bool StringEmpty(const char *String){
	return String == NULL || String[0] == 0;
}

bool StringEq(const char *A, const char *B){
	if(A == NULL || B == NULL) return A == B;
	return strcmp(A, B) == 0;
}

bool StringEqCI(const char *A, const char *B){
	if(A == NULL || B == NULL) return A == B;
#if OS_WINDOWS
	return _stricmp(A, B) == 0;
#else
	return strcasecmp(A, B) == 0;
#endif
}

bool StringCopy(char *Dest, int DestCapacity, const char *Src){
	int SrcLength = (Src != NULL ? (int)strlen(Src) : 0);
	return StringCopyN(Dest, DestCapacity, Src, SrcLength);
}

bool StringCopyN(char *Dest, int DestCapacity, const char *Src, int SrcLength){
	ASSERT(DestCapacity > 0);
	bool Result = (SrcLength < DestCapacity);
	if(Result && SrcLength > 0){
		memcpy(Dest, Src, SrcLength);
		Dest[SrcLength] = 0;
	}else{
		Dest[0] = 0;
	}
	return Result;
}

bool StringFormat(char *Dest, int DestCapacity, const char *Format, ...){
	va_list ap;
	va_start(ap, Format);
	int Written = vsnprintf(Dest, DestCapacity, Format, ap);
	va_end(ap);
	return Written >= 0 && Written < DestCapacity;
}

bool StringFormatTime(char *Dest, int DestCapacity, const char *Format, int Timestamp){
	struct tm tm = GetLocalTime((time_t)Timestamp);
	int Result = (int)strftime(Dest, DestCapacity, Format, &tm);
	return Result >= 0 && Result < DestCapacity;
}

uint32 HashString(const char *String){
	uint32 Hash = 5381;
	if(String){
		while(int c = *String++){
			Hash = ((Hash << 5) + Hash) + c;
		}
	}
	return Hash;
}

bool DirectoryExists(const char *Path){
#if OS_WINDOWS
	DWORD dwAttrib = GetFileAttributesA(Path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	return (stat(Path, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

bool CreateDirectoryRecursive(const char *Path){
	if(DirectoryExists(Path)) return true;
	char Temp[1024];
	StringCopy(Temp, sizeof(Temp), Path);
	for(char *p = Temp + 1; *p; p++){
		if(*p == '/' || *p == '\\'){
			char ch = *p;
			*p = 0;
			if(!DirectoryExists(Temp)){
				mkdir_portable(Temp, 0755);
			}
			*p = ch;
		}
	}
	if(!DirectoryExists(Temp)){
		mkdir_portable(Temp, 0755);
	}
	return DirectoryExists(Path);
}
