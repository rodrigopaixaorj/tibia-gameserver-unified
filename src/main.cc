#include "compat/compat.hh"
#include "crypto/crypto.hh"
#include "database/db.hh"
#include "login/login_server.hh"
#include "game/common.hh"
#include "game/communication.hh"
#include "game/config.hh"

#include <iostream>
#include <iomanip>
#include <csignal>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_MSC_VER)
    #define SERVER_COMPILER "MSVC " _CRT_STRINGIZE(_MSC_VER)
#elif defined(__clang__)
    #define SERVER_COMPILER "Clang " __clang_version__
#elif defined(__GNUC__)
    #define SERVER_COMPILER "GCC " __VERSION__
#else
    #define SERVER_COMPILER "Unknown Compiler"
#endif

#if defined(__amd64__) || defined(_M_X64)
    #define SERVER_PLATFORM "x64"
#elif defined(__i386__) || defined(_M_IX86)
    #define SERVER_PLATFORM "x86"
#elif defined(__arm__) || defined(_M_ARM) || defined(__aarch64__)
    #define SERVER_PLATFORM "ARM"
#else
    #define SERVER_PLATFORM "unknown"
#endif

static volatile bool g_ServerRunning = true;

static void SignalHandler(int signum){
	std::cout << "\n>> Shutdown signal received (" << signum << "). Closing server...\n";
	g_ServerRunning = false;
	CloseGame();
}

int main(int argc, char **argv){
#ifdef _WIN32
	SetConsoleTitleA("Tibia CipSoft 7.72 Server");
#endif

	std::cout << ":: ============================================================================\n";
	std::cout << ":: Tibia CipSoft Server Version 7.72\n";
	std::cout << ":: Compiled with " << SERVER_COMPILER << "\n";
	std::cout << ":: Compiled on " << __DATE__ << " " << __TIME__ << " for platform " << SERVER_PLATFORM << "\n";
	std::cout << "::\n";
	std::cout << ":: A server developed by CipSoft / OTServ Community\n";
	std::cout << ":: Server protocol: 7.72\n";
	std::cout << ":: ============================================================================\n::\n";

	// 1. Initialize network subsystem
	std::cout << ":: Initializing network subsystem... " << std::flush;
	if(!SocketSystemInit()){
		std::cout << "[failed]\n";
		std::cerr << "> ERROR: Failed to initialize network subsystem.\n";
		return 1;
	}
	std::cout << "[done]\n";

	// 2. Register signal handlers
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	// 3. Initialize embedded SQLite database
	std::cout << ":: Checking Database Connection... " << std::flush;
	const char *dbPath = "tibia.db";
	if(!DatabaseInit(dbPath)){
		std::cout << "[failed]\n";
		std::cerr << "> ERROR: Failed to connect to SQLite database.\n";
		SocketSystemExit();
		return 1;
	}
	std::cout << "SQLite 3 [done]\n";

	// 4. Start Login Server on port 7171
	int loginPort = 7171;
	std::cout << ":: Starting Login Server on port " << loginPort << "... " << std::flush;
	if(!LoginServerStart(loginPort, "0.0.0.0", "Welcome to Tibia 7.72!")){
		std::cout << "[failed]\n";
		std::cerr << "> ERROR: Failed to start Login Server on port " << loginPort << "\n";
	} else {
		std::cout << "[done]\n";
	}

	// 5. Initialize game data and subsystems
	try {
		InitAll();
		std::cout << ":: Initializing gamestate... " << std::flush;
		StartGame();
		std::cout << "[done]\n";
	} catch (const char *err) {
		std::cout << "[failed]\n";
		std::cerr << "\n> ERROR: Game initialization failed: " << err << "\n";
		LoginServerStop();
		DatabaseExit();
		SocketSystemExit();
		return 1;
	} catch (const std::exception &e) {
		std::cout << "[failed]\n";
		std::cerr << "\n> ERROR: Exception during initialization: " << e.what() << "\n";
		LoginServerStop();
		DatabaseExit();
		SocketSystemExit();
		return 1;
	}

	std::cout << ":: Checking world type... " << std::flush;
	const char *worldTypeName = "OPEN-PVP";
	if(WorldType == NON_PVP) worldTypeName = "OPTIONAL-PVP (NO-PVP)";
	else if(WorldType == PVP_ENFORCED) worldTypeName = "HARDCORE-PVP (PVP-ENFORCED)";
	std::cout << worldTypeName << " (World: " << WorldName << ")\n" << std::flush;

	std::cout << ":: Loaded all modules, server starting up... [done]\n";
	std::cout << ":: ============================================================================\n\n";

	std::cout << ">> " << WorldName << " (Tibia 7.72) Server Online!\n";
	std::cout << "   - Login Server: port " << loginPort << "\n";
	std::cout << "   - Game Server:  port " << GamePort << "\n";
	std::cout << "   - World Type:   " << worldTypeName << "\n";
	std::cout << "   - Max Players:  " << MaxPlayers << "\n\n";
	std::cout << ">> Press Ctrl+C to shutdown the server.\n\n" << std::flush;

	// 6. Main Game Server Loop
	auto lastTick = std::chrono::steady_clock::now();
	int beatMs = (Beat > 0 ? Beat : 50);

	while(g_ServerRunning && GameRunning()){
		auto now = std::chrono::steady_clock::now();
		int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();

		if(elapsed >= beatMs){
			lastTick = now;
			try {
				ReceiveData();
				AdvanceGame(elapsed);
			} catch(const char *err) {
				std::cerr << "\n> ERROR in game loop: " << err << "\n";
			} catch(const std::exception &e) {
				std::cerr << "\n> EXCEPTION in game loop: " << e.what() << "\n";
			} catch(...) {
				std::cerr << "\n> UNKNOWN EXCEPTION in game loop.\n";
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	std::cout << "\n:: Shutting down server...\n";
	std::cout << ":: Stopping Login Server... " << std::flush;
	LoginServerStop();
	std::cout << "[done]\n";

	std::cout << ":: Saving and closing game sessions... " << std::flush;
	LogoutAllPlayers();
	ExitAll();
	std::cout << "[done]\n";

	std::cout << ":: Closing database and sockets... " << std::flush;
	DatabaseExit();
	SocketSystemExit();
	std::cout << "[done]\n";

	std::cout << ":: Server shut down successfully.\n";
	return 0;
}
