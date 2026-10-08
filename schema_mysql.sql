-- ==============================================================================
-- Tibia CipSoft 7.72 Server Schema for MariaDB / MySQL
-- ==============================================================================

SET FOREIGN_KEY_CHECKS=0;

-- 1. Patches
CREATE TABLE IF NOT EXISTS `Patches` (
	`FileName` VARCHAR(255) NOT NULL,
	`Timestamp` BIGINT NOT NULL,
	PRIMARY KEY (`FileName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 2. Worlds
CREATE TABLE IF NOT EXISTS `Worlds` (
	`WorldID` INT NOT NULL,
	`Name` VARCHAR(64) NOT NULL,
	`Type` INT NOT NULL DEFAULT 0,
	`RebootTime` INT NOT NULL DEFAULT 5,
	`Host` VARCHAR(128) NOT NULL DEFAULT '127.0.0.1',
	`Port` INT NOT NULL DEFAULT 7172,
	`MaxPlayers` INT NOT NULL DEFAULT 1000,
	`PremiumPlayerBuffer` INT NOT NULL DEFAULT 100,
	`MaxNewbies` INT NOT NULL DEFAULT 300,
	`PremiumNewbieBuffer` INT NOT NULL DEFAULT 100,
	`OnlinePeak` INT NOT NULL DEFAULT 0,
	`OnlinePeakTimestamp` BIGINT NOT NULL DEFAULT 0,
	`LastStartup` BIGINT NOT NULL DEFAULT 0,
	`LastShutdown` BIGINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`WorldID`),
	UNIQUE KEY `uk_world_name` (`Name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 3. Accounts
CREATE TABLE IF NOT EXISTS `Accounts` (
	`AccountID` INT UNSIGNED NOT NULL,
	`Email` VARCHAR(255) NOT NULL,
	`Auth` VARBINARY(128) NOT NULL,
	`PremiumEnd` BIGINT NOT NULL DEFAULT 0,
	`PendingPremiumDays` INT NOT NULL DEFAULT 0,
	`Deleted` TINYINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`AccountID`),
	UNIQUE KEY `uk_account_email` (`Email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 4. Characters
CREATE TABLE IF NOT EXISTS `Characters` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`CharacterID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`AccountID` INT UNSIGNED NOT NULL,
	`Name` VARCHAR(64) NOT NULL,
	`Sex` TINYINT NOT NULL DEFAULT 1,
	`Level` INT NOT NULL DEFAULT 1,
	`Profession` VARCHAR(32) NOT NULL DEFAULT '',
	`Residence` VARCHAR(64) NOT NULL DEFAULT 'Thais',
	`LastLoginTime` BIGINT NOT NULL DEFAULT 0,
	`TutorActivities` INT NOT NULL DEFAULT 0,
	`IsOnline` TINYINT NOT NULL DEFAULT 0,
	`Deleted` TINYINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`CharacterID`),
	UNIQUE KEY `uk_character_name` (`Name`),
	KEY `idx_characters_world` (`WorldID`, `IsOnline`),
	KEY `idx_characters_account` (`AccountID`, `IsOnline`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 5. Character Rights
CREATE TABLE IF NOT EXISTS `CharacterRights` (
	`CharacterID` INT UNSIGNED NOT NULL,
	`Name` VARCHAR(64) NOT NULL,
	PRIMARY KEY (`CharacterID`, `Name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 6. Character Deaths
CREATE TABLE IF NOT EXISTS `CharacterDeaths` (
	`DeathID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`CharacterID` INT UNSIGNED NOT NULL,
	`Level` INT NOT NULL,
	`OffenderID` INT UNSIGNED NOT NULL,
	`Remark` VARCHAR(255) NOT NULL DEFAULT '',
	`Unjustified` TINYINT NOT NULL DEFAULT 0,
	`Timestamp` BIGINT NOT NULL,
	PRIMARY KEY (`DeathID`),
	KEY `idx_deaths_character` (`CharacterID`, `Timestamp`),
	KEY `idx_deaths_offender` (`OffenderID`, `Timestamp`),
	KEY `idx_deaths_time` (`Timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 7. Buddies (VIP List)
CREATE TABLE IF NOT EXISTS `Buddies` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`AccountID` INT UNSIGNED NOT NULL,
	`BuddyID` INT UNSIGNED NOT NULL,
	PRIMARY KEY (`WorldID`, `AccountID`, `BuddyID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 8. World Invitations
CREATE TABLE IF NOT EXISTS `WorldInvitations` (
	`WorldID` INT NOT NULL,
	`CharacterID` INT UNSIGNED NOT NULL,
	PRIMARY KEY (`WorldID`, `CharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 9. Login Attempts
CREATE TABLE IF NOT EXISTS `LoginAttempts` (
	`AttemptID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`AccountID` INT UNSIGNED NOT NULL,
	`IPAddress` INT UNSIGNED NOT NULL,
	`Timestamp` BIGINT NOT NULL,
	`Failed` TINYINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`AttemptID`),
	KEY `idx_login_acc` (`AccountID`, `Timestamp`),
	KEY `idx_login_ip` (`IPAddress`, `Timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 10. Guilds
CREATE TABLE IF NOT EXISTS `Guilds` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`GuildID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`Name` VARCHAR(64) NOT NULL,
	`LeaderID` INT UNSIGNED NOT NULL,
	`Created` BIGINT NOT NULL,
	PRIMARY KEY (`GuildID`),
	UNIQUE KEY `uk_guild_name` (`Name`),
	UNIQUE KEY `uk_guild_leader` (`LeaderID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 11. Guild Ranks
CREATE TABLE IF NOT EXISTS `GuildRanks` (
	`GuildID` INT UNSIGNED NOT NULL,
	`Rank` INT NOT NULL,
	`Name` VARCHAR(64) NOT NULL,
	PRIMARY KEY (`GuildID`, `Rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 12. Guild Members
CREATE TABLE IF NOT EXISTS `GuildMembers` (
	`GuildID` INT UNSIGNED NOT NULL,
	`CharacterID` INT UNSIGNED NOT NULL,
	`Rank` INT NOT NULL,
	`Title` VARCHAR(64) NOT NULL DEFAULT '',
	`Joined` BIGINT NOT NULL,
	PRIMARY KEY (`CharacterID`),
	KEY `idx_members_guild` (`GuildID`, `Rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 13. Guild Invites
CREATE TABLE IF NOT EXISTS `GuildInvites` (
	`GuildID` INT UNSIGNED NOT NULL,
	`CharacterID` INT UNSIGNED NOT NULL,
	`RecruiterID` INT UNSIGNED NOT NULL,
	`Timestamp` BIGINT NOT NULL,
	PRIMARY KEY (`GuildID`, `CharacterID`),
	KEY `idx_invites_char` (`CharacterID`),
	KEY `idx_invites_recruiter` (`RecruiterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 14. Houses
CREATE TABLE IF NOT EXISTS `Houses` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`HouseID` SMALLINT UNSIGNED NOT NULL,
	`Name` VARCHAR(128) NOT NULL,
	`Rent` INT NOT NULL DEFAULT 0,
	`Description` TEXT NOT NULL,
	`Size` INT NOT NULL DEFAULT 0,
	`PositionX` INT NOT NULL DEFAULT 0,
	`PositionY` INT NOT NULL DEFAULT 0,
	`PositionZ` INT NOT NULL DEFAULT 0,
	`Town` VARCHAR(64) NOT NULL DEFAULT '',
	`GuildHouse` TINYINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`WorldID`, `HouseID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 15. House Owners
CREATE TABLE IF NOT EXISTS `HouseOwners` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`HouseID` SMALLINT UNSIGNED NOT NULL,
	`OwnerID` INT UNSIGNED NOT NULL,
	`PaidUntil` INT NOT NULL DEFAULT 0,
	PRIMARY KEY (`WorldID`, `HouseID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 16. House Auctions
CREATE TABLE IF NOT EXISTS `HouseAuctions` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`HouseID` SMALLINT UNSIGNED NOT NULL,
	`BidderID` INT UNSIGNED DEFAULT NULL,
	`BidAmount` INT DEFAULT NULL,
	`FinishTime` BIGINT DEFAULT NULL,
	PRIMARY KEY (`WorldID`, `HouseID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 17. House Transfers
CREATE TABLE IF NOT EXISTS `HouseTransfers` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`HouseID` SMALLINT UNSIGNED NOT NULL,
	`NewOwnerID` INT UNSIGNED NOT NULL,
	`Price` INT NOT NULL DEFAULT 0,
	PRIMARY KEY (`WorldID`, `HouseID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 18. House Auction Exclusions
CREATE TABLE IF NOT EXISTS `HouseAuctionExclusions` (
	`CharacterID` INT UNSIGNED NOT NULL,
	`Issued` BIGINT NOT NULL,
	`Until` BIGINT NOT NULL,
	`BanishmentID` INT NOT NULL,
	KEY `idx_auction_exclusions` (`CharacterID`, `Until`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 19. House Assignments
CREATE TABLE IF NOT EXISTS `HouseAssignments` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`HouseID` SMALLINT UNSIGNED NOT NULL,
	`OwnerID` INT UNSIGNED NOT NULL,
	`Price` INT NOT NULL DEFAULT 0,
	`Timestamp` BIGINT NOT NULL,
	KEY `idx_assignments_house` (`WorldID`, `HouseID`),
	KEY `idx_assignments_time` (`WorldID`, `Timestamp`),
	KEY `idx_assignments_owner` (`OwnerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 20. Banishments
CREATE TABLE IF NOT EXISTS `Banishments` (
	`BanishmentID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`AccountID` INT UNSIGNED NOT NULL,
	`IPAddress` INT UNSIGNED NOT NULL,
	`GamemasterID` INT UNSIGNED NOT NULL,
	`Reason` VARCHAR(255) NOT NULL,
	`Comment` TEXT NOT NULL,
	`FinalWarning` TINYINT NOT NULL DEFAULT 0,
	`Issued` BIGINT NOT NULL,
	`Until` BIGINT NOT NULL,
	PRIMARY KEY (`BanishmentID`),
	KEY `idx_banishments_acc` (`AccountID`, `Until`, `FinalWarning`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 21. IP Banishments
CREATE TABLE IF NOT EXISTS `IPBanishments` (
	`BanishmentID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`CharacterID` INT UNSIGNED NOT NULL,
	`IPAddress` INT UNSIGNED NOT NULL,
	`GamemasterID` INT UNSIGNED NOT NULL,
	`Reason` VARCHAR(255) NOT NULL,
	`Comment` TEXT NOT NULL,
	`Issued` BIGINT NOT NULL,
	`Until` BIGINT NOT NULL,
	PRIMARY KEY (`BanishmentID`),
	KEY `idx_ipban_address` (`IPAddress`),
	KEY `idx_ipban_char` (`CharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 22. Namelocks
CREATE TABLE IF NOT EXISTS `Namelocks` (
	`CharacterID` INT UNSIGNED NOT NULL,
	`IPAddress` INT UNSIGNED NOT NULL,
	`GamemasterID` INT UNSIGNED NOT NULL,
	`Reason` VARCHAR(255) NOT NULL,
	`Comment` TEXT NOT NULL,
	`Attempts` INT NOT NULL DEFAULT 0,
	`Approved` TINYINT NOT NULL DEFAULT 0,
	PRIMARY KEY (`CharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 23. Notations
CREATE TABLE IF NOT EXISTS `Notations` (
	`NotationID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`CharacterID` INT UNSIGNED NOT NULL,
	`IPAddress` INT UNSIGNED NOT NULL,
	`GamemasterID` INT UNSIGNED NOT NULL,
	`Reason` VARCHAR(255) NOT NULL,
	`Comment` TEXT NOT NULL,
	PRIMARY KEY (`NotationID`),
	KEY `idx_notations_char` (`CharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 24. Statements
CREATE TABLE IF NOT EXISTS `Statements` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`Timestamp` BIGINT NOT NULL,
	`StatementID` INT NOT NULL,
	`CharacterID` INT UNSIGNED NOT NULL,
	`Channel` VARCHAR(64) NOT NULL,
	`Text` TEXT NOT NULL,
	PRIMARY KEY (`WorldID`, `Timestamp`, `StatementID`),
	KEY `idx_statements_char` (`CharacterID`, `Timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 25. Reported Statements
CREATE TABLE IF NOT EXISTS `ReportedStatements` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`Timestamp` BIGINT NOT NULL,
	`StatementID` INT NOT NULL,
	`CharacterID` INT UNSIGNED NOT NULL,
	`BanishmentID` INT NOT NULL,
	`ReporterID` INT UNSIGNED NOT NULL,
	`Reason` VARCHAR(255) NOT NULL,
	`Comment` TEXT NOT NULL,
	PRIMARY KEY (`WorldID`, `Timestamp`, `StatementID`),
	KEY `idx_reported_char` (`CharacterID`, `Timestamp`),
	KEY `idx_reported_ban` (`BanishmentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 26. Kill Statistics
CREATE TABLE IF NOT EXISTS `KillStatistics` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`RaceName` VARCHAR(64) NOT NULL,
	`TimesKilled` INT NOT NULL DEFAULT 0,
	`PlayersKilled` INT NOT NULL DEFAULT 0,
	PRIMARY KEY (`WorldID`, `RaceName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 27. Online Characters
CREATE TABLE IF NOT EXISTS `OnlineCharacters` (
	`WorldID` INT NOT NULL DEFAULT 1,
	`Name` VARCHAR(64) NOT NULL,
	`Level` INT NOT NULL DEFAULT 1,
	`Profession` VARCHAR(32) NOT NULL DEFAULT '',
	PRIMARY KEY (`WorldID`, `Name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ==============================================================================
-- Initial Seed Data
-- ==============================================================================

REPLACE INTO `Worlds` (`WorldID`, `Name`, `Type`, `RebootTime`, `Host`, `Port`, `MaxPlayers`, `PremiumPlayerBuffer`, `MaxNewbies`, `PremiumNewbieBuffer`)
	VALUES (1, 'Zanera', 0, 5, 'localhost', 7172, 1000, 100, 300, 100);

-- Default account: 111111 / password: tibia
REPLACE INTO `Accounts` (`AccountID`, `Email`, `Auth`)
	VALUES (111111, '@tibia', UNHEX('206699cbc2fae1683118c873d746aa376049cb5923ef0980298bb7acbba527ec9e765668f7a338dffea34acf61a20efb654c1e9c62d35148dba2aeeef8dc7788'));

-- Default characters
REPLACE INTO `Characters` (`WorldID`, `CharacterID`, `AccountID`, `Name`, `Sex`)
	VALUES (1, 1, 111111, 'Gamemaster', 1), (1, 2, 111111, 'Player', 1);

-- Default Gamemaster rights
REPLACE INTO `CharacterRights` (`CharacterID`, `Name`)
	VALUES
		(1, 'NOTATION'),
		(1, 'NAMELOCK'),
		(1, 'STATEMENT_REPORT'),
		(1, 'BANISHMENT'),
		(1, 'FINAL_WARNING'),
		(1, 'IP_BANISHMENT'),
		(1, 'KICK'),
		(1, 'HOME_TELEPORT'),
		(1, 'GAMEMASTER_BROADCAST'),
		(1, 'ANONYMOUS_BROADCAST'),
		(1, 'NO_BANISHMENT'),
		(1, 'ALLOW_MULTICLIENT'),
		(1, 'LOG_COMMUNICATION'),
		(1, 'READ_GAMEMASTER_CHANNEL'),
		(1, 'READ_TUTOR_CHANNEL'),
		(1, 'HIGHLIGHT_HELP_CHANNEL'),
		(1, 'SEND_BUGREPORTS'),
		(1, 'NAME_INSULTING'),
		(1, 'NAME_SENTENCE'),
		(1, 'NAME_NONSENSICAL_LETTERS'),
		(1, 'NAME_BADLY_FORMATTED'),
		(1, 'NAME_NO_PERSON'),
		(1, 'NAME_CELEBRITY'),
		(1, 'NAME_COUNTRY'),
		(1, 'NAME_FAKE_IDENTITY'),
		(1, 'NAME_FAKE_POSITION'),
		(1, 'STATEMENT_INSULTING'),
		(1, 'STATEMENT_SPAMMING'),
		(1, 'STATEMENT_ADVERT_OFFTOPIC'),
		(1, 'STATEMENT_ADVERT_MONEY'),
		(1, 'STATEMENT_NON_ENGLISH'),
		(1, 'STATEMENT_CHANNEL_OFFTOPIC'),
		(1, 'STATEMENT_VIOLATION_INCITING'),
		(1, 'CHEATING_BUG_ABUSE'),
		(1, 'CHEATING_GAME_WEAKNESS'),
		(1, 'CHEATING_MACRO_USE'),
		(1, 'CHEATING_MODIFIED_CLIENT'),
		(1, 'CHEATING_HACKING'),
		(1, 'CHEATING_MULTI_CLIENT'),
		(1, 'CHEATING_ACCOUNT_TRADING'),
		(1, 'CHEATING_ACCOUNT_SHARING'),
		(1, 'GAMEMASTER_THREATENING'),
		(1, 'GAMEMASTER_PRETENDING'),
		(1, 'GAMEMASTER_INFLUENCE'),
		(1, 'GAMEMASTER_FALSE_REPORTS'),
		(1, 'KILLING_EXCESSIVE_UNJUSTIFIED'),
		(1, 'DESTRUCTIVE_BEHAVIOUR'),
		(1, 'SPOILING_AUCTION'),
		(1, 'INVALID_PAYMENT'),
		(1, 'TELEPORT_TO_CHARACTER'),
		(1, 'TELEPORT_TO_MARK'),
		(1, 'TELEPORT_VERTICAL'),
		(1, 'TELEPORT_TO_COORDINATE'),
		(1, 'LEVITATE'),
		(1, 'SPECIAL_MOVEUSE'),
		(1, 'MODIFY_GOSTRENGTH'),
		(1, 'SHOW_COORDINATE'),
		(1, 'RETRIEVE'),
		(1, 'ENTER_HOUSES'),
		(1, 'OPEN_NAMEDOORS'),
		(1, 'INVULNERABLE'),
		(1, 'UNLIMITED_MANA'),
		(1, 'KEEP_INVENTORY'),
		(1, 'ALL_SPELLS'),
		(1, 'UNLIMITED_CAPACITY'),
		(1, 'ATTACK_EVERYWHERE'),
		(1, 'NO_LOGOUT_BLOCK'),
		(1, 'GAMEMASTER_OUTFIT'),
		(1, 'ILLUMINATE'),
		(1, 'CHANGE_PROFESSION'),
		(1, 'IGNORED_BY_MONSTERS'),
		(1, 'SHOW_KEYHOLE_NUMBERS'),
		(1, 'CREATE_OBJECTS'),
		(1, 'CREATE_MONEY'),
		(1, 'CREATE_MONSTERS'),
		(1, 'CHANGE_SKILLS'),
		(1, 'CLEANUP_FIELDS'),
		(1, 'NO_STATISTICS');

SET FOREIGN_KEY_CHECKS=1;
