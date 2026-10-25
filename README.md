# Tibia CipSoft 7.72 Server (Windows & Linux Standalone)

[![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-brightgreen.svg)]()
[![Protocol](https://img.shields.io/badge/Protocol-7.72-orange.svg)]()
[![Database](https://img.shields.io/badge/Database-SQLite%203%20%7C%20MySQL%20%7C%20PostgreSQL-blue.svg)](https://www.sqlite.org/)
[![Tools](https://img.shields.io/badge/Tools-Tibia%20Data%20Studio-blueviolet.svg)]()
[![License](https://img.shields.io/badge/License-Public%20Domain-lightgrey.svg)](LICENSE.txt)

A modern, standalone, native Windows and Linux port of the decompiled **CipSoft Tibia 7.72 Server Engine**. 

This project unifies the **Game Server**, **Login Server**, and a **Flexible Tri-Database Engine (SQLite 3, MySQL/MariaDB, and PostgreSQL)** into a single, high-performance, lightweight executable (`tibia-server.exe`), eliminating the complex multi-process IPC and strict Linux PostgreSQL requirements of the original architecture. It features seamless integration with OTServ web AACs (Gesior, MyAAC, Znote), full **OTServList status protocol support (TSQP 1.0)**, a hybrid player persistence engine (`database` or legacy `files`), a comprehensive **TFS-style configuration system (`config.cfg`)**, and the **Tibia Data Studio** — an integrated web management suite with interactive 2D map visualization, entity editors, and quest wizards.

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
  * **Game Server (Port 7172):** Complete world simulation, AI tick loop, and map sector management.
  * **Tri-Database Driver Support:**
    * **Embedded SQLite 3:** Uses `tibia.db` with zero external service configuration.
    * **MySQL / MariaDB:** Full support for external database servers with prepared statements (`MYSQL_STMT`), automatic reconnection, and web AAC compatibility.
    * **PostgreSQL:** Native driver support with complete schema (`schema_pgsql.sql`).
  * **Hybrid Player Persistence (`player_storage_mode`):**
    * `"database"`: Stores skills, inventories, depots, and player states directly in SQL tables for instant AAC synchronization.
    * `"files"`: Stores player states in legacy text files (`usr/*.usr`) for complete vintage authenticity.

* **OTServList & Status Protocol (TSQP 1.0):**
  * Built-in support for XML and binary status queries on port `7171`.
  * Live telemetry reporting uptime, online count, peak players, monsters, NPCs, server location, URL, MOTD, and gameplay rates.
  * Compatible with Otservlist.org, otsv.me, and custom web AAC status widgets.

* **Tibia Data Studio (Web Management Suite):**
  * Integrated web application built with Python/Flask and a sleek, responsive dark mode interface.
  * **Interactive 2D Map Visualizer:** Real-time sector rendering (`.sec` & `.map`), floor navigation (Z: 0 to 15 / +7 to -7), minimap overview, zoom, and coordinate inspection.
  * **Full Entity Editors:** Visual editors for Monsters (`mon/`), NPCs (`npc/`), Items & Flags (`dat/`), and Spells (`magic.cc`).
  * **Quest Wizard:** Step-by-step visual creation of quest chests, unique keys, quest doors, and item rewards.
  * **Visual Diff Viewer:** Compare modified server files against original virgin backups before deploying.
  * **Bilingual Support:** Complete internationalization for English and Portuguese (PT-BR).

* **TFS-Style Gameplay & Server Customization (`config.cfg`):**
  * **Priority Configuration:** `config.cfg` has top loading priority, ensuring custom configurations override legacy `.tibia` files automatically.
  * **Rates & Multipliers:** Configurable experience (`rate_exp`), skill progress (`rate_skill`), magic level (`rate_magic`), loot drop rate (`rate_loot`), monster spawn timers (`rate_spawn`), and HP/Mana regeneration rates (`rate_hp_regen`, `rate_mana_regen`).
  * **Item Consumption & QoL:** Auto-learn / free spells toggle (`learn_spells`), infinite runes (`remove_charges_from_runes`), infinite potions/vials (`remove_charges_from_vials`), infinite ammunition & throwing weapons (`remove_weapon_ammunition`), unbreakable weapons, shields, amulets & infinite rings (`remove_weapon_charges`), unlimited carry capacity (`free_capacity`), full illumination (`always_light`), and configurable summons limit (`max_summons_per_player`).
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
* **GameMaster Tools & Extended Documentation:**
  * Interactive offline HTML reference guide available at `docs/gm_commands_guide.html`.
  * Step-based forward relative teleportation (`alani <steps>`).
  * Teleportation to named monsters and NPCs (`alani sio "<creature>"`).
  * Gamemaster invisibility toggle (`alana vid` / ghost mode).
  * Unquoted multi-word item spawning support for `alevo`.
  * Progression protection preventing GameMasters from accidentally gaining experience or advancing skill levels.

* **Performance & Exploit Mitigations:**
  * **Real-Time Boot Telemetry:** Asynchronous map sector loading with real-time percentage, throughput (sectors/sec), object count, and ETA display.
  * **Floor Separation:** Fixed exploit allowing creatures and players to be pushed across different Z-axis floors.
  * **Concurrency & Stability:** Patched RSA mutex deadlocks on login bursts, resolved divide-by-zero crashes, and protected against waiting list buffer overflows.

---

## Repository Structure

```text
tibia-server/
├── AAC Manager/                # Complete Web AAC Portal (Znote AAC tailored for CipSoft 7.72)
├── build.bat                   # 1-click build script for Windows (compiles & copies to root)
├── CMakeLists.txt              # Modern CMake build configuration (MSVC & GCC/Clang)
├── Makefile                    # GNU Makefile for MinGW / Linux
├── config.cfg                  # Server configuration file (network, databases, rates, gameplay)
├── schema.sql                  # Complete SQLite database schema and initial data
├── schema_mysql.sql            # Complete MySQL/MariaDB database schema and initial data
├── schema_pgsql.sql            # Complete PostgreSQL database schema and initial data
├── tibia.db                    # Embedded SQLite database (pre-populated with RealOTs characters)
├── tibia.pem                   # 1024-bit RSA private key
├── mpir.dll                    # MPIR cryptographic math library for Windows
├── libmariadb.dll              # MariaDB/MySQL client connector library
├── z.dll                       # Compression library
├── docs/
│   └── gm_commands_guide.html  # Interactive GameMaster commands reference guide
├── tools/
│   ├── Tibia Game Data Editor.bat  # 1-click launcher for Tibia Data Studio
│   ├── export_sqlite_to_mysql.py   # Automated SQLite to MySQL migration script
│   ├── export_sqlite_to_mysql.bat  # 1-click database migration launcher
│   └── tibia_data_studio/          # Full Web Management Suite (Flask, Map Visualizer, Editors)
└── src/
    ├── main.cc                 # Main entry point, unified initialization & telemetry
    ├── compat/                 # Windows/Linux portability layer (Sockets, Threads, Timers)
    ├── crypto/                 # RSA, XTEA, SHA256, BigInt routines
    ├── database/               # Database abstraction layer (SQLite 3, MySQL & PostgreSQL drivers)
    ├── login/                  # Integrated Login Server (port 7171) & TSQP 1.0 status responder
    └── game/                   # CipSoft game server engine & simulation logic
```

---

## Quick Start & Releases

### Pre-compiled Packages (Recommended for Players & Server Hosts)
If you do not want to compile the source code yourself, download the latest official pre-compiled package from GitHub Releases:
* **[Download Latest Release (ZIP)](https://github.com/rodrigopaixaorj/tibia-gameserver-unified/releases)**

Each release package contains the ready-to-run `tibia-server.exe`, runtime DLLs (`mpir.dll`, `libmariadb.dll`, `z.dll`), `config.cfg`, RSA key `tibia.pem`, and `tibia.db`.

---

## Building from Source

### Requirements
* **Windows:** Windows 10/11 x64, Visual Studio 2019/2022/2026 (with "Desktop development with C++") or MinGW-w64, and CMake 3.15+. Bundled dependencies are included under `deps/`.
* **Linux:** GCC 9+ or Clang 10+, CMake 3.15+, MariaDB client (`libmariadb-dev` / `libmysqlclient-dev`), PostgreSQL client (`libpq-dev`), and GMP library (`libgmp-dev`).

### 1-Click Build on Windows (Recommended)
Simply double-click or run the `build.bat` script from the project root:
```cmd
build.bat
```
* Automatically locates **CMake** (in system `PATH` or bundled with Visual Studio 2019/2022/2026).
* Configures and builds the **Release (x64)** binary using parallel cores.
* Automatically places the freshly built `tibia-server.exe` into the repository root.
* You can also run `build.bat clean` to clean the build directory, or `build.bat rebuild` for a fresh clean compile.

### Build with CMake CLI (Manual)
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
1. Ensure `tibia-server.exe`, `mpir.dll`, `libmariadb.dll`, `config.cfg`, `tibia.pem`, and `tibia.db` (if using SQLite) are present in the server root.
2. Edit `config.cfg` to adjust network, database, and rate preferences:
   ```ini
   login_port = 7171
   game_port = 7172
   bind_address = "0.0.0.0"
   world_address = "127.0.0.1"
   world_name = "Zanera"
   # Game Data Directory (TFS style: auto-detects dat, map, mon, npc, usr, save, log)
   data_path = "../tibia-game_data"

   # Database Selection: "sqlite", "mysql", or "postgres"
   db_type = "sqlite"

   # Player persistence: "database" (SQL tables) or "files" (legacy usr/*.usr)
   player_storage_mode = "database"

   # SQLite Settings (when db_type = "sqlite")
   database_file = "tibia.db"

   # Status Protocol (TSQP 1.0)
   server_name = "Tibia 7.72 Classic"
   owner_name = "Admin"
   owner_email = "admin@tibia.local"
   server_url = "https://otservlist.org"
   server_location = "Brazil"

   # Multipliers
   rate_exp = 1
   rate_skill = 3
   rate_magic = 3
   rate_loot = 1
   ```
3. Run `tibia-server.exe`.
4. Connect using a standard **Tibia 7.72 Client** pointed to `127.0.0.1` (Port `7171`).

---

## AAC Manager (Web Portal & Account Creator)

The repository includes a complete, pre-configured web portal based on **Znote AAC** located in the [`AAC Manager/`](AAC%20Manager/) directory. It is specially customized with a native **`CIPSOFT`** engine driver to communicate directly with the CipSoft 7.72 database tables without requiring TFS compatibility bridges.

### Key Highlights
* **Native CipSoft 7.72 Integration:** Directly manages accounts (`Accounts`), players (`Players`), equipment/inventory (`Player_Items`), skills (`Player_Skills`), depots (`Player_DepotItems`), storage values, and death records in real time.
* **Modern Player Experience:**
  * Clean, responsive interface optimized for both desktop and mobile web browsing.
  * Account registration, recovery, and character creation with authentic 7.72 vocations and towns.
  * Real-time online list, highscores (experience, magic level, skills), and server death logs.
  * Interactive guild system with rank management, guild war declarations, and private guild boards.
  * House browsing, direct purchase, and auction bidding.
  * In-game bug reporting and support ticket helpdesk.
* **Comprehensive Administration Suite:**
  * Web-based GameMaster admin panel for publishing news, managing changelogs, moderating forum threads, reviewing reports, and applying bans.
  * Built-in shop system supporting PayPal, PagSeguro, and PayGol payment gateways.
  * Two-Factor Authentication (2FA) support via Authenticator apps (e.g., Authy, Google Authenticator).

---

### Prerequisites
* **Web Server Stack:** Apache or Nginx with **PHP 5.6 to 8.x** (e.g., [UniServerZ](https://www.uniformserver.com/), [XAMPP](https://www.apachefriends.org/), or standard Linux LAMP/LEMP).
* **Required PHP Extensions:** `pdo_mysql`, `curl`, `gd`, `openssl`, and `mbstring`.
* **Database:** MySQL 5.7+ or MariaDB 10.3+ with the server database imported (`tibia_mysql_full.sql` or `schema_mysql.sql`).

---

### Installation & Setup Guide

#### 1. Deploy Files to your Web Server
Copy the contents of the [`AAC Manager/`](AAC%20Manager/) folder to your web server root or virtual host directory (e.g., `C:\UniServerZ\www\` or `/var/www/html/`).

#### 2. Import Database Schemas
The AAC Manager operates on the same MySQL database as the game server:
1. Import the main server schema and initial data (if you have not done so already):
   ```bash
   mysql -u root -p tibia < tibia_mysql_full.sql
   ```
2. Import the AAC-specific web tables schema located at [`AAC Manager/engine/database/znote_schema.sql`](AAC%20Manager/engine/database/znote_schema.sql):
   ```bash
   mysql -u root -p tibia < "AAC Manager/engine/database/znote_schema.sql"
   ```
   *(Alternatively, navigating to the website in your browser for the first time will automatically prompt and generate the schema for you).*

#### 3. Configure `config.php`
Open `config.php` in your AAC root folder and verify the following essential settings:

```php
// 1. Engine selection (MUST be set to 'CIPSOFT')
$config['ServerEngine'] = 'CIPSOFT';

// 2. Site Title and Public URL
$config['site_title'] = 'CipSoft 7.72 AAC';
$config['site_url']   = 'http://localhost'; // or your domain / public IP

// 3. Absolute path to your tibia-gameserver-unified folder
$config['server_path'] = 'D:\\Games\\OTServ\\tibia-gameserver-unified';

// 4. MySQL Connection Credentials (must match your config.cfg)
$config['sqlHost']     = '127.0.0.1';
$config['sqlPort']     = 3306;
$config['sqlUser']     = 'root';
$config['sqlPassword'] = 'your_mysql_password';
$config['sqlDatabase'] = 'tibia';

// 5. Web Administrator Access (Account numbers that have admin panel rights)
$config['page_admin_access'] = array('111111', '666666');
```

#### 4. Configure Game Server (`config.cfg`)
Ensure your `config.cfg` in the game server root is configured to use MySQL with direct database persistence:
```ini
# Database Driver Selection
db_type = "mysql"

# Direct player state synchronization with SQL
player_storage_mode = "database"

# MySQL Connection Details
mysql_host = "127.0.0.1"
mysql_port = 3306
mysql_user = "root"
mysql_password = "your_mysql_password"
mysql_database = "tibia"
mysql_reconnect = true
```

#### 5. Cache Directory Permissions
Ensure that the `engine/cache/` directory has write permissions so the portal can cache telemetry, highscores, and server status:
* **Linux:** `chmod -R 775 engine/cache`
* **Windows:** Ensure the IIS/Apache/UniServerZ process user has Modify/Write permissions on the `engine/cache/` folder.

#### 6. Access the Website
Open your browser and navigate to:
```text
http://localhost/
```
You can now create new accounts and characters, explore the highscores, manage guilds, and log in as administrator using your configured account number.

---

## Tibia Data Studio (Web Management Suite)

**Tibia Data Studio** is an all-in-one web application designed to manage, inspect, and edit virgin CipSoft data files without touching raw binaries or text files manually.

### How to Launch:
* **Windows (1-Click):** Double-click `tools/Tibia Game Data Editor.bat`.
* **Command Line:**
  ```bash
  cd tools/tibia_data_studio
  python run_editor.py
  ```
* Open your browser and navigate to: `http://localhost:5000`

### Included Tools:
1. **Interactive 2D Map Visualizer:**
   * Full world map exploration with real-time tile and object rendering.
   * Multi-floor navigation (Z: 0 to 15 / ground +7 to underground -7).
   * Minimap HUD with click-to-pan, coordinate tracker, and zoom controls.
2. **Entity Editors:**
   * **Monsters (`mon/`):** Visual editor for health, experience, attacks, defenses, immunities, and loot tables.
   * **NPCs (`npc/`):** View dialogue scripts, vendors, and interaction trees.
   * **Items (`dat/`):** Inspect item properties, flags (cumulative, container, blocking), and sprite IDs.
   * **Spells (`magic.cc`):** Inspect instant spells, runes, mana cost, and vocation requirements.
3. **Quest Wizard:**
   * Graphical setup for quest chests, quest keys, door action IDs, and multi-item reward bags.
4. **Visual Diff Viewer:**
   * Side-by-side comparison of local changes against virgin CipSoft reference files.
5. **Localization:**
   * Toggle dynamically between English and Portuguese (PT-BR).

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
| `data_path` | `"../tibia-game_data"` | Root data directory (TFS style). Automatically loads `dat`, `map`, `mon`, `npc`, `usr`, `save`, `log`. |
| `map_path` | _(optional)_ | Custom path to map sectors directory if stored outside data_path. |
| `monster_path` | _(optional)_ | Custom path to monsters directory if stored outside data_path. |
| `npc_path` | _(optional)_ | Custom path to NPCs directory if stored outside data_path. |
| `user_path` | _(optional)_ | Custom path to legacy character `.usr` files if stored outside data_path. |
| `save_path` | _(optional)_ | Custom path to game state save directory. Auto-created if omitted. |
| `log_path` | _(optional)_ | Custom path to server logs directory. Auto-created if omitted. |
| `rsa_key_file` | `"tibia.pem"` | 1024-bit RSA private key file for login decryption. |
| `motd` | `"Welcome to Tibia!"` | Message of the day shown upon character login. |

### 2. Database & Player Storage
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `db_type` | `"sqlite"` | Database driver engine: `"sqlite"`, `"mysql"`, or `"postgres"`. |
| `player_storage_mode` | `"database"` | Mode of saving player states: `"database"` (SQL tables) or `"files"` (legacy `usr/*.usr` text files). |
| `database_file` | `"tibia.db"` | SQLite database file path (when `db_type = "sqlite"`). |
| `mysql_host` | `"127.0.0.1"` | MySQL/MariaDB server hostname or IP (when `db_type = "mysql"`). |
| `mysql_port` | `3306` | MySQL/MariaDB server port. |
| `mysql_user` | `"root"` | MySQL/MariaDB username. |
| `mysql_password` | `""` | MySQL/MariaDB password. |
| `mysql_database` | `"tibia"` | MySQL/MariaDB database schema name. |
| `mysql_reconnect` | `true` | Automatically reconnect to MySQL on connection drops. |
| `pg_host` | `"127.0.0.1"` | PostgreSQL server hostname or IP (when `db_type = "postgres"`). |
| `pg_port` | `5432` | PostgreSQL server port. |
| `pg_user` | `"postgres"` | PostgreSQL username. |
| `pg_password` | `""` | PostgreSQL password. |
| `pg_database` | `"tibia"` | PostgreSQL database schema name. |

### 3. OTServList & Status Protocol (TSQP 1.0)
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `server_name` | `"Tibia 7.72 Classic"` | Server name broadcasted to status queriers and Otservlist. |
| `owner_name` | `"Admin"` | Server administrator name. |
| `owner_email` | `"admin@tibia.local"`| Contact email broadcasted in status protocol. |
| `server_url` | `"https://otservlist.org"` | Official server website URL. |
| `server_location` | `"Brazil"` | Geographical location tag. |
| `status_timeout` | `5000` | Status query socket timeout in milliseconds. |

### 4. Server Multipliers & Rates
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `rate_exp` | `1` | Experience gain multiplier from monster kills. |
| `rate_skill` | `3` | Skill advancement multiplier (Fist, Club, Sword, Axe, Distance, Shielding, Fishing). |
| `rate_magic` | `3` | Magic Level advancement multiplier (mana spent on spells). |
| `rate_loot` | `1` | Monster item drop rate multiplier. |
| `rate_spawn` | `1` | Monster spawn speed multiplier (higher values reduce respawn timer). |
| `rate_hp_regen` | `1` | Health point regeneration rate multiplier. |
| `rate_mana_regen` | `1` | Mana point regeneration rate multiplier. |

### 5. Gameplay & Item Consumption
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `learn_spells` | `false` | When `false`, spells do not need to be learned/bought from NPCs and are automatically available once Level/Magic Level requirements are met. When `true`, spells must be bought from NPCs. |
| `remove_charges_from_runes` | `false` | When `false`, magic runes have infinite charges and are not consumed upon use. |
| `remove_charges_from_vials` | `false` | When `false`, potions and fluids (mana/life fluids) are not emptied upon drinking. |
| `remove_weapon_ammunition` | `false` | When `false`, ammunition (arrows, bolts) and thrown weapons (spears, stars) are infinite. |
| `remove_weapon_charges` | `false` | When `false`, weapons, shields, amulets (unlimited charges) and rings (unlimited time/charges) do not wear out or expire. |
| `free_capacity` | `false` | When `true`, players have unlimited carry capacity (ignore item weight). |
| `always_light` | `false` | When `true`, ambient light is always at maximum (no night darkness). |
| `max_summons_per_player` | `2` | Maximum summoned/convinced creatures per player. |

### 6. Combat & Timers
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `attack_interval` | `2000` | Physical and wand attack interval in milliseconds (Fast Attack). |
| `defense_interval` | `2000` | Shield defense block interval in milliseconds. |
| `spell_exhaustion` | `1000` | Global cooldown for instant spells in milliseconds. |
| `rune_exhaustion` | `2000` | Cooldown for using magic runes in milliseconds. |
| `stair_jump_exhaustion` | `2000` | Delay after changing floors/stairs before performing actions (in ms). |

### 7. PvP, Skulls & Frags
| Parameter | Default | Description |
| :--- | :---: | :--- |
| `world_type` | `"pvp"` | Game world PvP mode: `"pvp"`, `"no-pvp"`, or `"pvp-enforced"`. |
| `protection_level` | `1` | Minimum character level required to participate in PvP combat. |
| `kills_to_red_skull` | `3` | Unjustified kills in 24 hours to acquire Red Skull (`kills_to_red_skull_week = 5`, `kills_to_red_skull_month = 10`). |
| `kills_to_ban` | `6` | Unjustified kills in 24 hours to trigger automatic banishment (`kills_to_ban_week = 10`, `kills_to_ban_month = 20`). |
| `white_skull_time` | `900` | White Skull duration in seconds (15 minutes). |
| `in_fight_time` | `60` | In-fight combat logout block duration in seconds. |
| `red_skull_duration` | `2592000` | Red Skull duration in seconds (30 days). |

### 8. Death Penalties, Soul Points & Houses
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

## GameMaster Commands & Documentation

A complete, interactive HTML documentation is provided in **[`docs/gm_commands_guide.html`](docs/gm_commands_guide.html)**.

Key commands available for GameMaster accounts (Access Level >= 3):
* **Teleportation:**
  * `alani <x>, <y>, <z>` – Teleport to exact coordinates.
  * `alani <steps>` – Teleport forward relative to current facing direction.
  * `alani "<player>"` – Teleport to target player.
  * `alani sio "<creature>"` – Teleport directly to a monster or NPC by name.
  * `alana vid` – Toggle Gamemaster invisibility mode (permanent until toggled off).
  * `aleta sio "<player>"` – Teleport target player to your position.
  * `aleta bar` – Teleport to temple of your hometown.
* **Items & Spawning:**
  * `alevo <item_id> [count]` / `alevo <item_name> [count]` – Spawn items directly into backpack or ground. Supports unquoted multi-word item names.
  * `alito tera "<monster>"` – Spawn a monster at current position.
* **Server Administration & Doors:**
  * `aleta grav` – Open and step through any locked door.
  * `aleta cogni` – Inspect door keys and access requirements.
  * `/b <message>` – Global red broadcast message.
  * `/kick "<player>"` – Instantly kick a player.
  * `/ban "<player>"` – Banish player account.

---

## Database Migration Tool

To easily migrate from the embedded SQLite database (`tibia.db`) to an external MySQL / MariaDB server:
1. Ensure MySQL is running and import `schema_mysql.sql`.
2. Double-click `tools/export_sqlite_to_mysql.bat` (or execute `python tools/export_sqlite_to_mysql.py`).
3. Update `db_type = "mysql"` in `config.cfg`.

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

* **Nottinghster** ([OtLand Profile](https://otland.net/members/nottinghster.3619/)) – Main author and lead developer responsible for the standalone porting, modernization, cross-platform architecture, and making this project publicly accessible to the OTServ community.
* **Toor** – For publicly releasing the virgin CipSoft 7.7 server tarball ([OtLand Thread](https://otland.net/threads/7-7-realots-7-7-cipsoft-files-virgin.244562/)).
* **Fusion32** – For the monumental reverse engineering and C++ decompilation project ([OtLand Thread](https://otland.net/threads/tibia-7-7-server-decompiled.296811/)).
* **CipSoft GmbH** – Creators of Tibia.
* **OpenTibia Community** – For the passion, research, and keeping classic Tibia alive.

---

## License & Disclaimer

This project is intended strictly for educational, research, and reverse engineering purposes. All code is released under the Public Domain (see `LICENSE.txt`). Tibia is a registered trademark of CipSoft GmbH.
