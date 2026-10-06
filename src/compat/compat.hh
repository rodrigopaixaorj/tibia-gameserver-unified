#ifndef TIBIA_COMPAT_HH_
#define TIBIA_COMPAT_HH_ 1

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef uintptr_t uintptr;
typedef size_t usize;

typedef uint32 pid_t;
typedef int timer_t;

#define STATIC_ASSERT(expr) static_assert((expr), #expr)
#define NARRAY(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define ISPOW2(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)
#define KB(x) ((usize)(x) << 10)
#define MB(x) ((usize)(x) << 20)
#define GB(x) ((usize)(x) << 30)

#if defined(_WIN32) || defined(_WIN64)
#	define OS_WINDOWS 1
#	define OS_LINUX 0
#elif defined(__linux__) || defined(__gnu_linux__)
#	define OS_WINDOWS 0
#	define OS_LINUX 1
#else
#	define OS_WINDOWS 0
#	define OS_LINUX 0
#endif

#if defined(_MSC_VER)
#	define COMPILER_MSVC 1
#	define COMPILER_GCC 0
#	define COMPILER_CLANG 0
#elif defined(__clang__)
#	define COMPILER_MSVC 0
#	define COMPILER_GCC 0
#	define COMPILER_CLANG 1
#elif defined(__GNUC__)
#	define COMPILER_MSVC 0
#	define COMPILER_GCC 1
#	define COMPILER_CLANG 0
#endif

#if COMPILER_GCC || COMPILER_CLANG
#	define ATTR_FALLTHROUGH __attribute__((fallthrough))
#	define ATTR_PRINTF(x, y) __attribute__((format(printf, x, y)))
#else
#	define ATTR_FALLTHROUGH
#	define ATTR_PRINTF(x, y)
#endif

#if COMPILER_MSVC
#	define TRAP() __debugbreak()
#elif COMPILER_GCC || COMPILER_CLANG
#	define TRAP() __builtin_trap()
#else
#	define TRAP() abort()
#endif

#define ASSERT_ALWAYS(expr) if(!(expr)) { TRAP(); }
#if ENABLE_ASSERTIONS
#	define ASSERT(expr) ASSERT_ALWAYS(expr)
#else
#	define ASSERT(expr) ((void)(expr))
#endif

// Networking Sockets
#if OS_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <windows.h>
#	include <io.h>
#	include <direct.h>
#	include <process.h>
#	pragma comment(lib, "ws2_32.lib")

#	undef ERROR
#	undef NOERROR
#	undef IN
#	undef OUT
#	undef OPTIONAL
#	undef CONST
#	undef min
#	undef max
#	undef stricmp

	typedef SOCKET socket_t;
	typedef int socklen_t;
	#define SOCKET_INVALID INVALID_SOCKET
	#define SOCKET_IS_VALID(s) ((s) != INVALID_SOCKET)
	#define strerrordesc_np(e) "socket error"
	#define poll WSAPoll
	#define pollfd WSAPOLLFD
	#define PATH_SEP '\\'
	#define PATH_SEP_STR "\\"
	#define mkdir_portable(path, mode) _mkdir(path)

	inline uint32 gettid(void){
		return (uint32)GetCurrentThreadId();
	}
#else
#	include <errno.h>
#	include <fcntl.h>
#	include <netdb.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <poll.h>
#	include <signal.h>
#	include <sys/socket.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <sys/syscall.h>
#	include <unistd.h>
#	include <arpa/inet.h>

	typedef int socket_t;
	#define SOCKET_INVALID (-1)
	#define SOCKET_IS_VALID(s) ((s) >= 0)
	#define PATH_SEP '/'
	#define PATH_SEP_STR "/"
	#define mkdir_portable(path, mode) mkdir(path, mode)

	inline uint32 gettid(void){
		return (uint32)syscall(SYS_gettid);
	}
#endif

// Log helper prototypes
void LogAdd(const char *Prefix, const char *Format, ...);
void LogAddVerbose(const char *Prefix, const char *Function, const char *File, int Line, const char *Format, ...);

#define LOG(...)		LogAdd("INFO", __VA_ARGS__)
#define LOG_WARN(...)	LogAddVerbose("WARN", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERR(...)	LogAddVerbose("ERR", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define PANIC(...)																\
	do{																			\
		LogAddVerbose("PANIC", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);	\
		TRAP();																	\
	}while(0)

// Helper function prototypes
bool SocketSystemInit(void);
void SocketSystemExit(void);
void SocketClose(socket_t &Socket);
bool SocketSetNonBlocking(socket_t Socket);
bool SocketSetReuseAddr(socket_t Socket);

int64 GetClockMonotonicMS(void);
int GetMonotonicUptime(void);
void SleepMS(int Milliseconds);

struct tm GetLocalTime(time_t t);
struct tm GetGMTime(time_t t);

bool StringEmpty(const char *String);
bool StringEq(const char *A, const char *B);
bool StringEqCI(const char *A, const char *B);
bool StringCopy(char *Dest, int DestCapacity, const char *Src);
bool StringCopyN(char *Dest, int DestCapacity, const char *Src, int SrcLength);
bool StringFormat(char *Dest, int DestCapacity, const char *Format, ...);
bool StringFormatTime(char *Dest, int DestCapacity, const char *Format, int Timestamp);

#define StringBufFormat(Dest, Format, ...) \
	StringFormat((Dest), sizeof(Dest), (Format), ##__VA_ARGS__)

#define StringBufFormatTime(Dest, Format, Timestamp) \
	StringFormatTime((Dest), sizeof(Dest), (Format), (Timestamp))

uint32 HashString(const char *String);
bool DirectoryExists(const char *Path);
bool FileExists(const char *Path);
bool CreateDirectoryRecursive(const char *Path);

#endif // TIBIA_COMPAT_HH_
