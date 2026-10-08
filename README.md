# Tibia CipSoft 7.72 Server (Windows & Linux Standalone)

[![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-brightgreen.svg)]()
[![Protocol](https://img.shields.io/badge/Protocol-7.72-orange.svg)]()
[![Database](https://img.shields.io/badge/Database-SQLite%203%20%7C%20MySQL%20%7C%20MariaDB-blue.svg)](https://www.sqlite.org/)
[![License](https://img.shields.io/badge/License-Public%20Domain-lightgrey.svg)](LICENSE.txt)

A modern, standalone, native Windows and Linux port of the decompiled **CipSoft Tibia 7.72 Server Engine**. 

This project unifies the **Game Server**, **Login Server**, and a **Flexible Dual-Database Engine (SQLite 3 & MySQL/MariaDB)** into a single, high-performance, lightweight executable (`tibia-server.exe`), eliminating the complex multi-process IPC and PostgreSQL requirements of the original architecture while allowing seamless integration with OTServ web AACs (Gesior, MyAAC, Znote).

---

## Key Highlights & Features

* **Native Windows & Cross-Platform Engine:**
  * Full Windows API / Winsock2 (`ws2_32.lib`) network compatibility with non-blocking sockets and `WSAPoll`.
  * Multi-threading standardized using modern **C++17** (`std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic`).
  * High-resolution timing subsystem using `std::chrono` (replaces legacy POSIX `gettimeofday`/`clock_gettime`).
  * Windows cryptographic math powered by **MPIR** (`mpir.dll`) and built-in RSA, XTEA, and SHA-256 engines.
  * Native build support for **MSVC (Visual Studio 2019/2022)**, **MinGW-w64**, and **CMake**.

* **All-in-One Standalone Architecture:**
  * **Embedded Login Server (Port 7171):** Handles Tibia 7.72 RSA authentication packets and character lists in-process.
  * **Game Server (Port 7172):** Complete world simulation and game loop.
  * **Dual Database Driver Support:**
    * **Embedded SQLite 3:** Uses `tibia.db` with zero external service configuration.
    * **MySQL / MariaDB:** Full support for external database servers with prepared statements (`MYSQL_STMT`), automatic reconnection, and web AAC compatibility.

* **Tibia 7.72 Protocol Support:**
  * Native 7.72 protocol parsing (`-DTIBIA772=1`), packet definitions, and client synchronization.

* **Extensive Bug Fixes & Refinements:**
  * **Stability:** Added missing exception handling (`try-catch`) across monster idle stimulation, sector refresh, and field creation loops; resolved crashes related to summon despawn attacks and unattributed damage calculations.
  * **Combat & Spells:** Corrected Ultimate Healing (UH) formula, Burst Arrow AoE propagation, wave spell trajectory line-of-sight checks, "Challenge" (`exeta res`), and GM door access spells (`aleta cogni`, `aleta grav`).
  * **Creatures & AI:** Fixed monster freezing bugs, fleeing creature attack logic, flight direction preferences, spawn field expansion, and NPC speech latency factors.
  * **World & Items:** Fixed container search radius (`CloseContainer` / `NotifyTrades`), cumulative throw-loot handling, level/quest door closing behaviors, and rope retrieval.
  * **Network Sync:** Fixed player state desynchronization on login and client assertions caused by VIP/Channel data.

---

## Repository Structure

```text
tibia-server/
├── CMakeLists.txt         # Modern CMake build configuration (MSVC & GCC/Clang)
├── Makefile               # GNU Makefile for MinGW / Linux
├── config.cfg             # Server configuration file (ports, database, paths, MOTD)
├── schema.sql             # Complete SQLite database schema and initial data
├── schema_mysql.sql       # Complete MySQL/MariaDB database schema and initial data
├── tibia.db               # Embedded SQLite database
├── tibia.pem              # 1024-bit RSA private key
├── mpir.dll               # MPIR library for Windows
├── libmariadb.dll         # MariaDB/MySQL client connector library
└── src/
    ├── main.cc            # Main entry point, unified initialization & game loop
    ├── compat/            # Windows/Linux portability layer (Sockets, Threads, Timers)
    ├── crypto/            # RSA, XTEA, SHA256, BigInt routines
    ├── database/          # Database abstraction layer (SQLite 3 & MySQL/MariaDB drivers)
    ├── login/             # Integrated Login Server (port 7171)
    └── game/              # CipSoft game server engine & simulation logic
```

---

## Building the Server

### Requirements
* **Windows:** Windows 10/11 x64, Visual Studio 2019/2022 (with "Desktop development with C++") or MinGW-w64, and CMake 3.15+.
* **Linux:** GCC 9+ or Clang 10+, CMake 3.15+, MariaDB client (`libmariadb-dev` / `libmysqlclient-dev`), and GMP library (`libgmp-dev`).

### Build with CMake (Recommended for Windows MSVC)
```bash
# Clone the repository
git clone https://github.com/rodrigopaixaorj/tibia-gameserver-unified.git
cd tibia-gameserver-unified

# Create build directory and generate project
mkdir build
cd build
cmake .. -A x64

# Compile Release binary
cmake --build . --config Release
```
The compiled `tibia-server.exe` will be located in `build/Release/`.

### Build on Linux
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## Running & Configuration

### 1. Game Data Prerequisite (Toor's Tarball)
The server binary requires the official virgin CipSoft 7.7 data files (`dat/`, `map/`, `mon/`, `npc/`, `save/`, etc.). 
* Download the original tarball released by Toor from the OtLand thread: **[7.7 RealOTs / CipSoft Files Virgin](https://otland.net/threads/7-7-realots-7-7-cipsoft-files-virgin.244562/)**.
* Extract the data files into a folder (e.g., `../tibia-game_data` or customize the path in `config.cfg`).

### 2. Server Setup & Launch
1. Make sure `tibia-server.exe`, `mpir.dll`, `libmariadb.dll`, `config.cfg`, `tibia.pem`, and `tibia.db` (if using SQLite) are placed in the server directory.
2. Edit `config.cfg` to configure network, paths, and database:
   ```ini
   login_port = 7171
   game_port = 7172
   bind_address = "0.0.0.0"
   world_address = "127.0.0.1"
   world_name = "Tibia"
   data_path = "../tibia-game_data"
   map_path = "../tibia-game_data/map"
   save_path = "../tibia-game_data/save"

   # Database Selection: "sqlite" or "mysql"
   db_type = "sqlite"

   # SQLite Settings (when db_type = "sqlite")
   database_file = "tibia.db"

   # MySQL / MariaDB Settings (when db_type = "mysql")
   mysql_host = "127.0.0.1"
   mysql_port = 3306
   mysql_user = "root"
   mysql_password = "password"
   mysql_database = "tibia"
   mysql_reconnect = true

   rsa_key_file = "tibia.pem"
   motd = "Welcome to CipSoft 7.72 Server!"
   ```
3. Run `tibia-server.exe`.
4. Connect using a **Tibia 7.72 Client** pointed to `127.0.0.1` (Port `7171`).

---

## Unified Pre-configured Test Accounts

The included database comes with pre-configured accounts (Password: `tibia`) populated with characters from the realots release:

| Account Number | Password | Vocation / Role | Total Characters |
| :--- | :--- | :--- | :--- |
| **`111111`** | `tibia` | Knights & Elite Knights | 12,485 |
| **`222222`** | `tibia` | Sorcerers & Master Sorcerers | 7,907 |
| **`333333`** | `tibia` | Druids & Elder Druids | 7,058 |
| **`444444`** | `tibia` | Paladins & Royal Paladins | 6,114 |
| **`555555`** | `tibia` | No Vocation / Rookgaard | 30,909 |
| **`666666`** | `tibia` | GameMasters (All GM rights enabled) | 38 |

---

## Credits & Acknowledgements

* **Toor** – For publicly releasing the virgin CipSoft 7.7 server tarball ([OtLand Thread](https://otland.net/threads/7-7-realots-7-7-cipsoft-files-virgin.244562/)).
* **Fusion32** – For the monumental reverse engineering and C++ decompilation project ([OtLand Thread](https://otland.net/threads/tibia-7-7-server-decompiled.296811/)).
* **CipSoft GmbH** – Creators of Tibia.
* **OpenTibia Community** – For the passion, research, and keeping classic Tibia alive.

---

## License & Disclaimer

This project is intended strictly for educational, research, and reverse engineering purposes. All code is released under the Public Domain (see `LICENSE.txt`). Tibia is a registered trademark of CipSoft GmbH.
