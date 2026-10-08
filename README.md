# Tibia CipSoft 7.72 Server (Windows & Linux Standalone)

[![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-brightgreen.svg)]()
[![Protocol](https://img.shields.io/badge/Protocol-7.72-orange.svg)]()
[![Database](https://img.shields.io/badge/Database-SQLite%203%20%7C%20MySQL%20%7C%20MariaDB-blue.svg)](https://www.sqlite.org/)
[![License](https://img.shields.io/badge/License-Public%20Domain-lightgrey.svg)](LICENSE.txt)

A modern, standalone, native Windows and Linux port of the decompiled **CipSoft Tibia 7.72 Server Engine**. 

This project unifies the **Game Server**, **Login Server**, and a **Flexible Dual-Database Engine (SQLite 3 & MySQL/MariaDB)** into a single, high-performance, lightweight executable (`tibia-server.exe`), eliminating the complex multi-process IPC and PostgreSQL requirements of the original architecture while allowing seamless integration with OTServ web AACs (Gesior, MyAAC, Znote) and providing a comprehensive **TFS-style configuration system (`config.cfg`)** for complete server and gameplay customization.

---

## Key Highlights & Features

* **Native Windows & Cross-Platform Engine:**
  * Full Windows API / Winsock2 (`ws2_32.lib`) network compatibility with non-blocking sockets and `WSAPoll`.
  * Multi-threading standardized using modern **C++17** (`std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic`).
  * High-resolution timing subsystem using `std::chrono` (replaces legacy POSIX `gettimeofday`/`clock_gettime`).
  * Windows cryptographic math powered by **MPIR** (`mpir.dll`) and built-in RSA, XTEA, and SHA-256 engines.
  * Native build support for **MSVC (Visual Studio 2019/2022/2026)**, **MinGW-w64**, and **CMake**.

* **All-in-One Standalone Architecture:**
  * **Embedded Login Server (Port 7171):** Handles Tibia 7.72 RSA authentication packets and character lists in-process.
  * **Game Server (Port 7172):** Complete world simulation and game loop.
  * **Dual Database Driver Support:**
    * **Embedded SQLite 3:** Uses `tibia.db` with zero external service configuration.
    * **MySQL / MariaDB:** Full support for external database servers with prepared statements (`MYSQL_STMT`), automatic reconnection, and web AAC compatibility.

* **Tibia 7.72 Protocol Support:**
  * Native 7.72 protocol parsing (`-DTIBIA772=1`), packet definitions, and client synchronization.

* **TFS-Style Gameplay & Server Customization (`config.cfg`):**
  * **Rates & Multipliers:** Configurable experience (`rate_exp`), skill progress (`rate_skill`), magic level (`rate_magic`), loot drop rate (`rate_loot`), monster spawn timers (`rate_spawn`), and HP/Mana regeneration rates (`rate_hp_regen`, `rate_mana_regen`).
  * **Item Consumption & QoL:** Infinite runes (`remove_charges_from_runes`), infinite potions/vials (`remove_charges_from_vials`), infinite ammunition & throwing weapons (`remove_weapon_ammunition`), unbreakable weapon charges & wearout (`remove_weapon_charges`), unlimited carry capacity (`free_capacity`), full illumination (`always_light`), and configurable summons limit (`max_summons_per_player`).
  * **Combat & Exhaustion:** Fast-attack interval (`attack_interval`), shield defense interval (`defense_interval`), spell cooldown (`spell_exhaustion`), rune cooldown (`rune_exhaustion`), and floor change delay (`stair_jump_exhaustion`).
  * **PvP, Skulls & Frags:** Configurable world type (`world_type = "pvp" | "no-pvp" | "pvp-enforced"`), minimum PvP level (`protection_level`), daily/weekly/monthly red skull limits (`kills_to_red_skull`), automatic banishment limits (`kills_to_ban`), white skull duration (`white_skull_time`), and in-fight combat lock (`in_fight_time`).
  * **Death & Losses:** Configurable base death penalty percentage (`death_lose_percent`), PvP kill experience (`experience_by_killing_players`), and PvP-Enforced exp percentage (`pvp_enforced_exp_percent`).
  * **Soul Points & Houses:** Soul point toggles (`enable_soul_points`), regeneration intervals (`soul_regen_interval`), house purchase level (`house_buy_level`), and VIP/Premium house restrictions (`house_only_premium`).

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
├── config.cfg             # Server configuration file (ports, database, paths, gameplay rates)
├── schema.sql             # Complete SQLite database schema and initial data
├── schema_mysql.sql       # Complete MySQL/MariaDB database schema and initial data
├── tibia.db               # Embedded SQLite database
├── tibia.pem              # 1024-bit RSA private key
├── mpir.dll               # MPIR library for Windows
├── libmariadb.dll         # MariaDB/MySQL client connector library
├── z.dll                  # Compression library
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
* **Windows:** Windows 10/11 x64, Visual Studio 2019/2022/2026 (with "Desktop development with C++") or MinGW-w64, and CMake 3.15+.
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
The compiled `tibia-server.exe` will be located in `build/Release/` and can be copied to the root directory.

### Build with MSBuild directly (Windows)
```bash
msbuild build\tibia-server.vcxproj -p:Configuration=Release -p:Platform=x64
```

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
2. Edit `config.cfg` to configure network, paths, database, and gameplay rates:
   ```ini
   login_port = 7171
   game_port = 7172
   bind_address = "0.0.0.0"
   world_address = "127.0.0.1"
   world_name = "Zanera"
   data_path = "../tibia-game_data/dat"
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
   mysql_password = ""
   mysql_database = "tibia"
   mysql_reconnect = true

   rsa_key_file = "tibia.pem"
   motd = "Welcome to CipSoft 7.72 Server!"
   ```
3. Run `tibia-server.exe`.
4. Connect using a **Tibia 7.72 Client** pointed to `127.0.0.1` (Port `7171`).

---

## Configuration Reference (`config.cfg`)

The server is configured via `config.cfg` in the root directory. All parameters support both `snake_case` and `camelCase` naming conventions.

### 1. Network, Capacity & System
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `login_port` | `7171` | Port for the embedded login server. |
| `game_port` | `7172` | Port for the game world server. |
| `bind_address` | `"0.0.0.0"` | Network interface IP address to bind to. |
| `world_address` | `"127.0.0.1"` | External/public IP address sent to client for game connection. |
| `world_name` | `"Zanera"` | Name of the game world. |
| `max_players` | `1000` | Maximum simultaneous players allowed online. |
| `max_connections` | `1100` | Maximum socket connections. |
| `connection_timeout`| `30` | Socket timeout in seconds. |
| `data_path` | `"../tibia-game_data/dat"` | Path to CipSoft `.dat` files directory. |
| `map_path` | `"../tibia-game_data/map"` | Path to CipSoft `.sec` and `.map` sector files directory. |
| `save_path` | `"../tibia-game_data/save"` | Path to game state save directory. |
| `log_path` | `"../tibia-game_data/log"` | Path to server logs directory. |
| `rsa_key_file` | `"tibia.pem"` | 1024-bit RSA private key file for login decryption. |
| `motd` | `"Welcome to Tibia!"` | Message of the day shown upon character login. |

### 2. Database (SQLite 3 & MySQL/MariaDB)
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `db_type` | `"sqlite"` | Database driver engine: `"sqlite"` or `"mysql"`. |
| `database_file` | `"tibia.db"` | SQLite database file path (when `db_type = "sqlite"`). |
| `mysql_host` | `"127.0.0.1"` | MySQL/MariaDB server hostname or IP (when `db_type = "mysql"`). |
| `mysql_port` | `3306` | MySQL/MariaDB server port. |
| `mysql_user` | `"root"` | MySQL/MariaDB username. |
| `mysql_password` | `""` | MySQL/MariaDB password. |
| `mysql_database` | `"tibia"` | MySQL/MariaDB database schema name. |
| `mysql_reconnect` | `true` | Automatically reconnect to MySQL on connection drops. |

### 3. Server Multipliers & Rates
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `rate_exp` | `1` | Experience gain multiplier from monster kills. |
| `rate_skill` | `3` | Skill advancement multiplier (Fist, Club, Sword, Axe, Distance, Shielding, Fishing). |
| `rate_magic` | `3` | Magic Level advancement multiplier (mana spent on spells). |
| `rate_loot` | `1` | Monster item drop rate multiplier. |
| `rate_spawn` | `1` | Monster spawn speed multiplier (higher values reduce respawn timer). |
| `rate_hp_regen` | `1` | Health point regeneration rate multiplier. |
| `rate_mana_regen` | `1` | Mana point regeneration rate multiplier. |

### 4. Gameplay & Item Consumption
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `remove_charges_from_runes` | `false` | When `false`, magic runes have infinite charges and are not consumed upon use. |
| `remove_charges_from_vials` | `false` | When `false`, potions and fluids (mana/life fluids) are not emptied upon drinking. |
| `remove_weapon_ammunition` | `false` | When `false`, ammunition (arrows, bolts) and thrown weapons (spears, stars) are infinite. |
| `remove_weapon_charges` | `false` | When `false`, weapons and shields with limited uses/charges do not wear out. |
| `free_capacity` | `false` | When `true`, players have unlimited carry capacity (ignore item weight). |
| `always_light` | `false` | When `true`, ambient light is always at maximum (no night darkness). |
| `max_summons_per_player` | `2` | Maximum summoned/convinced creatures per player. |

### 5. Combat & Timers
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `attack_interval` | `2000` | Physical and wand attack interval in milliseconds (Fast Attack). |
| `defense_interval` | `2000` | Shield defense block interval in milliseconds. |
| `spell_exhaustion` | `1000` | Global cooldown for instant spells in milliseconds. |
| `rune_exhaustion` | `2000` | Cooldown for using magic runes in milliseconds. |
| `stair_jump_exhaustion` | `2000` | Delay after changing floors/stairs before performing actions (in ms). |

### 6. PvP, Skulls & Frags
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `world_type` | `"pvp"` | Game world PvP mode: `"pvp"`, `"no-pvp"`, or `"pvp-enforced"`. |
| `protection_level` | `1` | Minimum character level required to participate in PvP combat. |
| `kills_to_red_skull` | `3` | Unjustified kills in 24 hours to acquire Red Skull (`kills_to_red_skull_week = 5`, `kills_to_red_skull_month = 10`). |
| `kills_to_ban` | `6` | Unjustified kills in 24 hours to trigger automatic banishment (`kills_to_ban_week = 10`, `kills_to_ban_month = 20`). |
| `white_skull_time` | `900` | White Skull duration in seconds (15 minutes). |
| `in_fight_time` | `60` | In-fight combat logout block duration in seconds. |
| `red_skull_duration` | `2592000` | Red Skull duration in seconds (30 days). |

### 7. Death Penalties, Soul Points & Houses
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `death_lose_percent` | `10` | Base percentage of experience/skills lost on death (7% promoted, -1% per blessing). |
| `experience_by_killing_players` | `false` | Enable experience gain from killing other players. |
| `pvp_enforced_exp_percent` | `5` | Percentage of victim's experience awarded to killer in PvP-Enforced mode. |
| `enable_soul_points` | `true` | Require and consume Soul Points for spells/runes. |
| `soul_regen_interval` | `120` | Seconds to regenerate 1 Soul Point (`soul_regen_interval_promoted = 15`). |
| `house_buy_level` | `1` | Minimum character level required to buy a house. |
| `house_only_premium` | `true` | Restrict house ownership to premium accounts. |

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
