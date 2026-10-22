<?php
/**
 * CipSoft Server Unified - Users Functions
 * Exclusively compatible with native CipSoft 7.72 schema (MySQL)
 */

// Fetch Images
function fetchImages($status) {
	$status = (int)$status;
	return mysql_select_multi("SELECT `id`, `title`, `desc`, `date`, `status`, `image`, `delhash`, `account_id` FROM `znote_images` WHERE `status`='$status' ORDER BY `date` DESC;");
}

// Insert image data
function insertImage($account_id, $title, $desc, $image, $image_delete) {
	$title = sanitize($title);
	$desc = sanitize($desc);
	$image = sanitize($image);
	$image_delete = sanitize($image_delete);
	$account_id = (int)$account_id;
	$time = time();

	$exist = mysql_select_single("SELECT `id` FROM `znote_images` WHERE `image`='$image' LIMIT 1;");
	if ($exist === false) {
		mysql_insert("INSERT INTO `znote_images` (`title`, `desc`, `date`, `status`, `image`, `delhash`, `account_id`) VALUES ('$title', '$desc', '$time', '1', '$image', '$image_delete', '$account_id');");
		return true;
	}
	return false;
}

function updateImage($id, $status) {
	$id = (int)$id;
	$status = (int)$status;
	mysql_update("UPDATE `znote_images` SET `status`='$status' WHERE `id`='$id';");
}

// Fetch killers score (Top Fraggers)
function fetchMurders() {
	return mysql_select_multi("SELECT `c`.`Name` AS `killed_by`, COUNT(`d`.`DeathID`) AS `kills` 
		FROM `CharacterDeaths` AS `d` 
		INNER JOIN `Characters` AS `c` ON `d`.`OffenderID` = `c`.`CharacterID` 
		WHERE `d`.`OffenderID` > 0 
		GROUP BY `d`.`OffenderID`, `c`.`Name` 
		ORDER BY `kills` DESC LIMIT 0, 10;");
}

// Fetch deaths score (Top Victims)
function fetchLoosers() {
	return mysql_select_multi("SELECT `c`.`Name` AS `name`, COUNT(`d`.`DeathID`) AS `Deaths` 
		FROM `CharacterDeaths` AS `d` 
		INNER JOIN `Characters` AS `c` ON `d`.`CharacterID` = `c`.`CharacterID` 
		GROUP BY `d`.`CharacterID`, `c`.`Name` 
		ORDER BY `Deaths` DESC LIMIT 0, 10;");
}

// Fetch latest deaths
function fetchLatestDeaths($from = 0, $to = 30) {
	$from = (int)$from;
	$to = (int)$to;
	$deaths = mysql_select_multi("SELECT `d`.`DeathID`, `d`.`CharacterID`, `d`.`Level` AS `level`, `d`.`OffenderID`, `d`.`Remark`, `d`.`Timestamp` AS `time`, `c`.`Name` AS `victim` 
		FROM `CharacterDeaths` AS `d` 
		INNER JOIN `Characters` AS `c` ON `d`.`CharacterID` = `c`.`CharacterID` 
		ORDER BY `d`.`Timestamp` DESC LIMIT $from, $to;");

	if ($deaths !== false) {
		for ($i = 0; $i < count($deaths); $i++) {
			$offender_id = (int)$deaths[$i]['OffenderID'];
			if ($offender_id > 0) {
				$killer_char = mysql_select_single("SELECT `Name` FROM `Characters` WHERE `CharacterID` = '$offender_id' LIMIT 1;");
				$deaths[$i]['killed_by'] = ($killer_char !== false) ? $killer_char['Name'] : 'Unknown player';
				$deaths[$i]['is_player'] = 1;
			} else {
				$deaths[$i]['killed_by'] = !empty($deaths[$i]['Remark']) ? $deaths[$i]['Remark'] : 'a creature';
				$deaths[$i]['is_player'] = 0;
			}
		}
	}
	return $deaths;
}

// Support list (Staff)
function support_list() {
	$staffs = mysql_select_multi("SELECT `c`.`CharacterID` AS `id`, `c`.`Name` AS `name`, `c`.`IsOnline` AS `online`, `c`.`AccountID` AS `account_id`, `cr`.`RightID` AS `group_id` 
		FROM `Characters` AS `c` 
		INNER JOIN `CharacterRights` AS `cr` ON `c`.`CharacterID` = `cr`.`CharacterID` 
		WHERE `c`.`Deleted` = 0 
		GROUP BY `c`.`CharacterID` 
		ORDER BY `cr`.`RightID` DESC, `c`.`Level` DESC;");

	if ($staffs === false) {
		// Fallback para God account (666666) se CharacterRights estiver vazio
		$staffs = mysql_select_multi("SELECT `CharacterID` AS `id`, `Name` AS `name`, `IsOnline` AS `online`, `AccountID` AS `account_id`, 3 AS `group_id` 
			FROM `Characters` 
			WHERE `AccountID` = 666666 AND `Deleted` = 0;");
	}
	return $staffs;
}

// NEWS
function fetchAllNews() {
	return mysql_select_multi("SELECT `n`.`id`, `n`.`title`, `n`.`text`, `n`.`date`, `c`.`Name` AS `name` 
		FROM `znote_news` AS `n` 
		LEFT JOIN `Characters` AS `c` ON `n`.`pid` = `c`.`CharacterID` 
		ORDER BY `n`.`id` DESC;");
}

// HOUSES
function fetchAllHouses() {
	return mysql_select_multi("SELECT `h`.`HouseID` AS `id`, `h`.`WorldID` AS `world_id`, `h`.`Town` AS `town_id`, `h`.`Name` AS `name`, `h`.`Rent` AS `rent`, `h`.`Size` AS `size`, `ho`.`OwnerID` AS `owner`, `c`.`Name` AS `owner_name`, `ho`.`PaidUntil` AS `paid` 
		FROM `Houses` AS `h` 
		LEFT JOIN `HouseOwners` AS `ho` ON `h`.`HouseID` = `ho`.`HouseID` 
		LEFT JOIN `Characters` AS `c` ON `ho`.`OwnerID` = `c`.`CharacterID` 
		ORDER BY `h`.`Town` ASC, `h`.`Name` ASC;");
}

function fetchTownHouses($town_id) {
	global $config;
	$town_filter = sanitize($town_id);
	if (is_numeric($town_id) && isset($config['towns'][$town_id])) {
		$town_name = sanitize($config['towns'][$town_id]);
		$where = "(`h`.`Town` = '$town_filter' OR `h`.`Town` = '$town_name')";
	} else {
		$where = "`h`.`Town` = '$town_filter'";
	}

	return mysql_select_multi("SELECT `h`.`HouseID` AS `id`, `h`.`WorldID` AS `world_id`, `h`.`Town` AS `town_id`, `h`.`Name` AS `name`, `h`.`Rent` AS `rent`, `h`.`Size` AS `size`, `ho`.`OwnerID` AS `owner`, `c`.`Name` AS `owner_name`, `ho`.`PaidUntil` AS `paid` 
		FROM `Houses` AS `h` 
		LEFT JOIN `HouseOwners` AS `ho` ON `h`.`HouseID` = `ho`.`HouseID` 
		LEFT JOIN `Characters` AS `c` ON `ho`.`OwnerID` = `c`.`CharacterID` 
		WHERE $where 
		ORDER BY `h`.`Name` ASC;");
}

function fetchHouse($house_id) {
	$house_id = (int)$house_id;
	return mysql_select_single("SELECT `h`.`HouseID` AS `id`, `h`.`WorldID` AS `world_id`, `h`.`Town` AS `town_id`, `h`.`Name` AS `name`, `h`.`Rent` AS `rent`, `h`.`Size` AS `size`, `ho`.`OwnerID` AS `owner`, `c`.`Name` AS `owner_name`, `ho`.`PaidUntil` AS `paid` 
		FROM `Houses` AS `h` 
		LEFT JOIN `HouseOwners` AS `ho` ON `h`.`HouseID` = `ho`.`HouseID` 
		LEFT JOIN `Characters` AS `c` ON `ho`.`OwnerID` = `c`.`CharacterID` 
		WHERE `h`.`HouseID` = '$house_id' LIMIT 1;");
}

// Is player online
function user_is_online($player_id) {
	$player_id = (int)$player_id;
	$data = mysql_select_single("SELECT `IsOnline` FROM `Characters` WHERE `CharacterID` = '$player_id' LIMIT 1;");
	return ($data !== false && $data['IsOnline'] == 1);
}

// Shop
function shop_delete_row_order($rowid) {
	$rowid = (int)$rowid;
	mysql_delete("DELETE FROM `znote_shop_orders` WHERE `id`='$rowid';");
}

function shop_update_row_count($rowid, $count) {
	$rowid = (int)$rowid;
	$count = (int)$count;
	mysql_update("UPDATE `znote_shop_orders` SET `count`='$count' WHERE `id`='$rowid'");
}

function shop_account_gender_tickets($accid) {
	$accid = (int)$accid;
	return mysql_select_multi("SELECT `id`, `count` FROM `znote_shop_orders` WHERE `account_id`='$accid' AND `type`='3';");
}

// GUILDS
function guild_remove_member($cid) {
	$cid = (int)$cid;
	mysql_delete("DELETE FROM `GuildMembers` WHERE `CharacterID` = '$cid';");
}

function guild_change_rank($gid, $rid, $name) {
	$gid = (int)$gid;
	$rid = (int)$rid;
	$name = sanitize($name);
	mysql_update("UPDATE `GuildRanks` SET `Name`='$name' WHERE `GuildID`='$gid' AND `Rank`='$rid';");
}

function guild_change_leader($nCid, $oCid) {
	$nCid = (int)$nCid;
	$oCid = (int)$oCid;
	$gid = guild_leader_gid($oCid);
	if ($gid === false) return false;

	// Leader is Rank 3, Vice-leader is Rank 2
	mysql_update("UPDATE `GuildMembers` SET `Rank`='3' WHERE `CharacterID`='$nCid';");
	mysql_update("UPDATE `GuildMembers` SET `Rank`='2' WHERE `CharacterID`='$oCid';");
	guild_new_leader($nCid, $gid);
	return true;
}

function guild_new_leader($new_leader, $gid) {
	$new_leader = (int)$new_leader;
	$gid = (int)$gid;
	mysql_update("UPDATE `Guilds` SET `LeaderID`='$new_leader' WHERE `GuildID`='$gid';");
}

function guild_leader_gid($leader) {
	$leader = (int)$leader;
	$data = mysql_select_single("SELECT `GuildID` FROM `Guilds` WHERE `LeaderID`='$leader' LIMIT 1;");
	return ($data !== false) ? $data['GuildID'] : false;
}

function guild_leader($gid) {
	$gid = (int)$gid;
	$data = mysql_select_single("SELECT `LeaderID` FROM `Guilds` WHERE `GuildID`='$gid' LIMIT 1;");
	return ($data !== false) ? $data['LeaderID'] : false;
}

function guild_remove_invites($gid) {
	$gid = (int)$gid;
	mysql_delete("DELETE FROM `GuildInvites` WHERE `GuildID`='$gid';");
}

function guild_delete($gid) {
	$gid = (int)$gid;
	mysql_delete("DELETE FROM `GuildMembers` WHERE `GuildID`='$gid';");
	mysql_delete("DELETE FROM `GuildRanks` WHERE `GuildID`='$gid';");
	mysql_delete("DELETE FROM `GuildInvites` WHERE `GuildID`='$gid';");
	mysql_delete("DELETE FROM `Guilds` WHERE `GuildID`='$gid';");
}

function guild_player_leave($cid) {
	$cid = (int)$cid;
	mysql_delete("DELETE FROM `GuildMembers` WHERE `CharacterID`='$cid';");
}

function guild_player_join($cid, $gid) {
	$cid = (int)$cid;
	$gid = (int)$gid;
	$time = time();

	guild_remove_all_invitations($cid);
	// Regular member rank = 1
	mysql_insert("INSERT INTO `GuildMembers` (`CharacterID`, `GuildID`, `Rank`, `Title`, `Joined`) VALUES ('$cid', '$gid', '1', '', '$time');");
	return true;
}

function guild_remove_invitation($cid, $gid) {
	$cid = (int)$cid;
	$gid = (int)$gid;
	mysql_delete("DELETE FROM `GuildInvites` WHERE `CharacterID`='$cid' AND `GuildID`='$gid';");
}

function guild_remove_all_invitations($cid) {
	$cid = (int)$cid;
	mysql_delete("DELETE FROM `GuildInvites` WHERE `CharacterID`='$cid';");
}

function guild_invite_player($cid, $gid, $inviter = 0) {
	$cid = (int)$cid;
	$gid = (int)$gid;
	$inviter = (int)$inviter;
	$time = time();
	mysql_insert("INSERT INTO `GuildInvites` (`CharacterID`, `GuildID`, `RecruiterID`, `Timestamp`) VALUES ('$cid', '$gid', '$inviter', '$time');");
}

function guild_invite_list($gid) {
	$gid = (int)$gid;
	return mysql_select_multi("SELECT `gi`.`CharacterID` AS `player_id`, `gi`.`GuildID` AS `guild_id`, `c`.`Name` AS `name` 
		FROM `GuildInvites` AS `gi` 
		INNER JOIN `Characters` AS `c` ON `gi`.`CharacterID` = `c`.`CharacterID` 
		WHERE `gi`.`GuildID`='$gid';");
}

function update_player_guild_position($cid, $rid) {
	$cid = (int)$cid;
	$rid = (int)$rid;
	mysql_update("UPDATE `GuildMembers` SET `Rank`='$rid' WHERE `CharacterID`='$cid';");
}

function update_player_guildnick($cid, $nick) {
	$cid = (int)$cid;
	$nick = sanitize($nick);
	mysql_update("UPDATE `GuildMembers` SET `Title`='$nick' WHERE `CharacterID`='$cid';");
}

function get_guild_rank_data($gid) {
	$gid = (int)$gid;
	return mysql_select_multi("SELECT `Rank` AS `id`, `GuildID` AS `guild_id`, `Name` AS `name`, `Rank` AS `level` 
		FROM `GuildRanks` 
		WHERE `GuildID`='$gid' 
		ORDER BY `Rank` DESC;");
}

function create_guild($cid, $name) {
	$cid = (int)$cid;
	$name = trim(preg_replace('/\s\s+/', ' ', str_replace("\n", " ", sanitize($name))));
	$time = time();

	// Insert Guild into native CipSoft schema (WorldID, Name, LeaderID, Created)
	mysql_insert("INSERT INTO `Guilds` (`WorldID`, `Name`, `LeaderID`, `Created`) VALUES ('1', '$name', '$cid', '$time');");
	$gid = get_guild_id($name);

	if ($gid !== false) {
		// Create default ranks: 1 Member, 2 Vice-Leader, 3 Leader
		mysql_insert("INSERT INTO `GuildRanks` (`GuildID`, `Rank`, `Name`) VALUES 
			('$gid', '1', 'Member'),
			('$gid', '2', 'Vice-Leader'),
			('$gid', '3', 'Leader');");

		// Associate creator as Leader
		mysql_insert("INSERT INTO `GuildMembers` (`CharacterID`, `GuildID`, `Rank`, `Title`, `Joined`) VALUES ('$cid', '$gid', '3', 'Leader', '$time');");
	}
}

function get_character_guild_rank($cid) {
	$cid = (int)$cid;
	$data = mysql_select_single("SELECT `Rank` FROM `GuildMembers` WHERE `CharacterID`='$cid' LIMIT 1;");
	return ($data !== false && $data['Rank'] > 0) ? $data['Rank'] : false;
}

function get_player_guild_rank($rank_id, $gid = 0) {
	$rank_id = (int)$rank_id;
	$gid = (int)$gid;
	$where = ($gid > 0) ? "WHERE `GuildID`='$gid' AND `Rank`='$rank_id'" : "WHERE `Rank`='$rank_id'";
	$data = mysql_select_single("SELECT `Name` FROM `GuildRanks` $where LIMIT 1;");
	return ($data !== false) ? $data['Name'] : false;
}

function get_guild_position($rid) {
	return (int)$rid;
}

function get_player_guild_data($cid) {
	$cid = (int)$cid;
	$data = mysql_select_single("SELECT `gm`.`GuildID` AS `guild_id`, `gm`.`Rank` AS `rank_id`, `gm`.`Rank` AS `rank_level`, `gr`.`Name` AS `rank_name`, `g`.`Name` AS `guild_name` 
		FROM `GuildMembers` AS `gm` 
		LEFT JOIN `GuildRanks` AS `gr` ON `gm`.`GuildID` = `gr`.`GuildID` AND `gm`.`Rank` = `gr`.`Rank` 
		LEFT JOIN `Guilds` AS `g` ON `gm`.`GuildID` = `g`.`GuildID` 
		WHERE `gm`.`CharacterID`='$cid' LIMIT 1;");
	return $data;
}

function get_guild_name($gid) {
	$gid = (int)$gid;
	$guild = mysql_select_single("SELECT `Name` FROM `Guilds` WHERE `GuildID`='$gid' LIMIT 1;");
	return ($guild !== false) ? $guild['Name'] : false;
}

function get_guild_id($name) {
	$name = sanitize($name);
	$data = mysql_select_single("SELECT `GuildID` FROM `Guilds` WHERE `Name`='$name' LIMIT 1;");
	return ($data !== false) ? $data['GuildID'] : false;
}

function get_guild_data($name) {
	$name = sanitize($name);
	return mysql_select_single("SELECT `GuildID` AS `id`, `Name` AS `name`, `LeaderID` AS `ownerid`, `Created` AS `creationdata` 
		FROM `Guilds` WHERE `Name`='$name' LIMIT 1;");
}

function get_guilds_list() {
	return mysql_select_multi("SELECT `GuildID` AS `id`, `Name` AS `name`, `Created` AS `creationdata` FROM `Guilds` ORDER BY `Name`;");
}

function get_guild_players($gid) {
	$gid = (int)$gid;
	return mysql_select_multi("SELECT `c`.`CharacterID` AS `id`, `c`.`Name` AS `name`, `c`.`Level` AS `level`, `c`.`Profession` AS `vocation`, `c`.`IsOnline` AS `online`, `gm`.`Rank` AS `rank_id`, `gm`.`Title` AS `guildnick`, `gr`.`Name` AS `rank_name`, `gr`.`Rank` AS `rank_level` 
		FROM `GuildMembers` AS `gm` 
		INNER JOIN `Characters` AS `c` ON `gm`.`CharacterID` = `c`.`CharacterID` 
		LEFT JOIN `GuildRanks` AS `gr` ON `gm`.`GuildID` = `gr`.`GuildID` AND `gm`.`Rank` = `gr`.`Rank` 
		WHERE `gm`.`GuildID` = '$gid' 
		ORDER BY `gm`.`Rank` DESC, `c`.`Name` ASC;");
}

function get_guild_level_data($gid) {
	$gid = (int)$gid;
	$data = mysql_select_multi("SELECT `c`.`Level` 
		FROM `GuildMembers` AS `gm` 
		INNER JOIN `Characters` AS `c` ON `gm`.`CharacterID` = `c`.`CharacterID` 
		WHERE `gm`.`GuildID` = '$gid';");
	$members = 0;
	$totallevels = 0;
	if ($data !== false && !empty($data)) {
		foreach ($data as $player) {
			$members++;
			$totallevels += (int)$player['Level'];
		}
		return array('avg' => (int)($totallevels / $members), 'total' => $totallevels, 'players' => $members);
	}
	return false;
}

function count_guild_members($gid) {
	$gid = (int)$gid;
	$data = mysql_select_single("SELECT COUNT(`CharacterID`) AS `total` FROM `GuildMembers` WHERE `GuildID`='$gid';");
	return ($data !== false) ? $data['total'] : 0;
}

// BANS & VIOLATIONS
function set_rule_violation($charname, $typeid, $actionid, $reasonid, $time, $comment) {
	$charid = user_character_id($charname);
	if (!$charid) return false;

	$typeid = (int)$typeid;
	$actionid = (int)$actionid;
	$reasonid = (int)$reasonid;
	$expires = (int)($time + time());
	$comment = sanitize($comment);

	$acc_id = user_character_account_id($charname);
	$now = time();

	// Tabela Banishments nativa CipSoft:
	// AccountID, CharacterID, BanishedBy, BanishedDate, ExpiresDate, Reason, Comment
	mysql_insert("INSERT INTO `Banishments` (`AccountID`, `CharacterID`, `BanishedBy`, `BanishedDate`, `ExpiresDate`, `Reason`, `Comment`) 
		VALUES ('$acc_id', '$charid', '1', '$now', '$expires', '$reasonid', '$comment');");
	return true;
}

// DEATHS & KILLERS
function user_fetch_deathlist($char_id) {
	$char_id = (int)$char_id;
	$deaths = mysql_select_multi("SELECT `DeathID` AS `id`, `Level` AS `level`, `OffenderID`, `Remark`, `Timestamp` AS `time` 
		FROM `CharacterDeaths` 
		WHERE `CharacterID`='$char_id' 
		ORDER BY `Timestamp` DESC LIMIT 0, 10;");

	if ($deaths !== false) {
		for ($i = 0; $i < count($deaths); $i++) {
			$off_id = (int)$deaths[$i]['OffenderID'];
			if ($off_id > 0) {
				$kchar = mysql_select_single("SELECT `Name` FROM `Characters` WHERE `CharacterID` = '$off_id' LIMIT 1;");
				$deaths[$i]['killed_by'] = ($kchar !== false) ? $kchar['Name'] : 'Unknown player';
				$deaths[$i]['is_player'] = 1;
			} else {
				$deaths[$i]['killed_by'] = !empty($deaths[$i]['Remark']) ? $deaths[$i]['Remark'] : 'a creature';
				$deaths[$i]['is_player'] = 0;
			}
		}
	}
	return $deaths;
}

function user_count_deathlist($char_id) {
	$char_id = (int)$char_id;
	$data = mysql_select_single("SELECT COUNT(`DeathID`) AS `id` FROM `CharacterDeaths` WHERE `CharacterID`='$char_id';");
	return ($data !== false) ? $data['id'] : 0;
}

// MY ACCOUNT RELATED
function user_update_comment($char_id, $comment) {
	$char_id = (int)$char_id;
	$comment = sanitize($comment);
	mysql_update("UPDATE `znote_players` SET `comment`='$comment' WHERE `player_id`='$char_id'");
}

function user_delete_character($char_id) {
	$char_id = (int)$char_id;
	mysql_update("UPDATE `Characters` SET `Deleted` = 1 WHERE `CharacterID`='$char_id';");
	mysql_delete("DELETE FROM `znote_players` WHERE `player_id`='$char_id';");
}

function user_delete_character_soft($char_id) {
	$char_id = (int)$char_id;
	$char_name = user_character_name($char_id);
	$original_acc_id = user_character_account_id($char_name);
	if (!user_character_pending_delete($char_name)) {
		mysql_insert('INSERT INTO `znote_deleted_characters`(`original_account_id`, `character_name`, `time`, `done`) VALUES(' . $original_acc_id . ', "' . $char_name . '", (NOW() + INTERVAL ' . config('delete_character_interval') . '), 0)');
		return true;
	}
	return false;
}

function user_character_pending_delete($char_name) {
	$char_name = sanitize($char_name);
	$result = mysql_select_single('SELECT `done` FROM `znote_deleted_characters` WHERE `character_name` = "' . $char_name . '"');
	return ($result === false) ? false : !$result['done'];
}

function user_pending_deletes($acc_id) {
	$acc_id = (int)$acc_id;
	return mysql_select_multi('SELECT `id`, `character_name`, `time` FROM `znote_deleted_characters` WHERE `original_account_id` = ' . $acc_id . ' AND `done` = 0');
}

function user_character_list($account_id, $limit = 100) {
	$account_id = (int)$account_id;
	$limit_sql = ($limit > 0) ? "LIMIT " . (int)$limit : "";
	$characters = mysql_select_multi("SELECT `c`.`CharacterID` AS `id`, `c`.`Name` AS `name`, `c`.`Level` AS `level`, `c`.`Profession` AS `vocation`, `c`.`Residence` AS `town_id`, `c`.`LastLoginTime` AS `lastlogin`, `c`.`IsOnline` AS `online`, COALESCE(`zp`.`hide_char`, 0) AS `hide_char` 
		FROM `Characters` `c` 
		LEFT JOIN `znote_players` `zp` ON `c`.`CharacterID` = `zp`.`player_id` 
		WHERE `c`.`AccountID`='$account_id' AND `c`.`Deleted` = 0 
		ORDER BY `c`.`Level` DESC 
		$limit_sql;");

	if ($characters !== false) {
		$count = count($characters);
		for ($i = 0; $i < $count; $i++) {
			$characters[$i]['vocation'] = $characters[$i]['vocation'];
			$characters[$i]['town_id'] = $characters[$i]['town_id'];

			if ($characters[$i]['lastlogin'] != 0) {
				$characters[$i]['lastlogin'] = getClock($characters[$i]['lastlogin'], true, false);
			} else {
				$characters[$i]['lastlogin'] = 'Never.';
			}

			$characters[$i]['online'] = online_id_to_name($characters[$i]['online']);
		}
	}
	return $characters;
}

function user_character_list_player_id($account_id) {
	$account_id = (int)$account_id;
	return mysql_select_multi("SELECT `CharacterID` AS `id` FROM `Characters` WHERE `AccountID`='$account_id' AND `Deleted` = 0 ORDER BY `Level` DESC LIMIT 0, 30");
}

function user_character_list_count($account_id) {
	$account_id = (int)$account_id;
	$data = mysql_select_single("SELECT COUNT(`CharacterID`) AS `id` FROM `Characters` WHERE `AccountID`='$account_id' AND `Deleted` = 0;");
	return ($data !== false) ? $data['id'] : 0;
}

// HIGHSCORES
function fetchAllScores($rows, $tfs, $g, $vlist, $v = -1, $flags = false, $outfits = false) {
	$rows = (int)$rows;
	$data = array();

	$vocGroups = array('all' => array());
	foreach ($vlist as $vid => $vdata) {
		if ($vdata['fromVoc'] === false) {
			$vocGroups[(string)$vid] = array();
		} else {
			$sharedGroup = (string)$vdata['fromVoc'] . ", " . (string)$vid;
			$vocGroups[$sharedGroup] = array();
			$vocGroups[(string)$vdata['fromVoc']] = $sharedGroup;
			$vocGroups[(string)$vid] = $sharedGroup;
		}
	}

	foreach ($vocGroups as $voc_id => $key_or_arr) {
		$vGrp = is_array($key_or_arr) ? $voc_id : $key_or_arr;

		if (empty($vocGroups[$vGrp])) {
			$vClause = '';
			if ($vGrp !== 'all') {
				$vocParts = explode(',', $vGrp);
				$profClauses = array();
				foreach ($vocParts as $vp) {
					$vp = trim($vp);
					$vName = vocation_id_to_name((int)$vp);
					if ($vName) {
						$profClauses[] = "'$vName'";
						$profClauses[] = "'" . (int)$vp . "'";
					}
				}
				if (!empty($profClauses)) {
					$vClause = "AND `c`.`Profession` IN (" . implode(',', $profClauses) . ")";
				}
			}

			// Experience / Level (subType 7) based on Characters
			$levelScores = mysql_select_multi("SELECT `c`.`Name` AS `name`, `c`.`Profession` AS `vocation`, `c`.`Level` AS `value`, 128 AS `type`, 0 AS `head`, 0 AS `body`, 0 AS `legs`, 0 AS `feet` 
				FROM `Characters` AS `c` 
				WHERE `c`.`Deleted` = 0 $vClause 
				ORDER BY `c`.`Level` DESC LIMIT 0, $rows;");

			for ($st = 1; $st <= 9; $st++) {
				$vocGroups[$vGrp][$st] = ($levelScores !== false) ? $levelScores : array();
			}
		}
	}
	return $vocGroups;
}

// LOST ACCOUNT & RECOVERY
function user_recover($mode, $edom, $email, $character, $ip) {
	$email = sanitize($email);
	$character = sanitize($character);
	$edom = sanitize($edom);

	$char_data = mysql_select_single("SELECT `AccountID` FROM `Characters` WHERE `Name` = '$character' AND `Deleted` = 0 LIMIT 1;");
	if ($char_data === false) {
		echo '<p>Character not found.</p>';
		return;
	}

	$account_id = (int)$char_data['AccountID'];
	$acc_data = mysql_select_single("SELECT `AccountID`, `Email`, `Auth` FROM `Accounts` WHERE `AccountID` = '$account_id' AND `Deleted` = 0 LIMIT 1;");
	if ($acc_data === false || strtolower($acc_data['Email']) !== strtolower($email)) {
		echo '<p>Account records do not match.</p>';
		return;
	}

	if ($mode === 'username') {
		if (!cipsoft_verify_password($edom, $acc_data['Auth'])) {
			echo '<p>Incorrect password.</p>';
			return;
		}
		echo '<br><p>Your account number is:</p> <h3>'. $acc_data['AccountID'] .'</h3>';
	} else {
		// Password recovery
		if ((int)$edom !== $account_id) {
			echo '<p>Account number does not match.</p>';
			return;
		}
		$newpass = substr(sha1(rand(1000000, 99999999)), 0, 10);
		user_change_password($account_id, $newpass);
		echo '<br><p>Your new password is:</p> <h3>'. $newpass .'</h3><p>Please keep it safe and change it upon logging in!</p>';
	}
}

function user_account_id_from_name($id) {
	$id = (int)$id;
	$result = mysql_select_single("SELECT `AccountID` FROM `Accounts` WHERE `AccountID` = '$id' LIMIT 1;");
	return ($result !== false) ? $result['AccountID'] : false;
}

// PREMIUM DAYS
function user_account_add_premdays($accid, $days) {
	$accid = (int)$accid;
	$days = (int)$days;
	$now = time();

	$data = mysql_select_single("SELECT `PremiumEnd` FROM `Accounts` WHERE `AccountID`='$accid' LIMIT 1;");
	if ($data !== false) {
		$currentEnd = (int)$data['PremiumEnd'];
		$base = ($currentEnd > $now) ? $currentEnd : $now;
		$newEnd = $base + ($days * 86400);
		mysql_update("UPDATE `Accounts` SET `PremiumEnd`='$newEnd' WHERE `AccountID`='$accid';");
	}
}

// GENDER
function user_character_change_gender($name) {
	$user_id = user_character_id($name);
	if (!$user_id) return;
	$data = mysql_select_single("SELECT `Sex` FROM `Characters` WHERE `CharacterID`='$user_id';");
	if ($data !== false) {
		$newSex = ($data['Sex'] == 1) ? 0 : 1;
		mysql_update("UPDATE `Characters` SET `Sex`='$newSex' WHERE `CharacterID`='$user_id';");
		// Also update PlayerState
		$lookType = ($newSex == 1) ? 128 : 136;
		mysql_update("UPDATE `PlayerState` SET `OrigLookType`='$lookType', `CurrLookType`='$lookType' WHERE `CharacterID`='$user_id';");
	}
}

function user_character_account_id($character) {
	$character = sanitize($character);
	$data = mysql_select_single("SELECT `AccountID` FROM `Characters` WHERE `Name`='$character' AND `Deleted` = 0 LIMIT 1;");
	return ($data !== false) ? $data['AccountID'] : false;
}

function user_update_account($update_data) {
	$update = array();
	array_walk($update_data, 'array_sanitize');
	foreach ($update_data as $field => $data) {
		$update[] = '`' . $field . '` = \'' . $data . '\'';
	}
	$user_id = (int)getSession('user_id');
	mysql_update("UPDATE `Accounts` SET " . implode(', ', $update) . " WHERE `AccountID`=" . $user_id . ";");
}

function user_update_znote_account($update_data) {
	$update = array();
	array_walk($update_data, 'array_sanitize');
	foreach ($update_data as $field => $data) {
		$update[] = '`' . $field . '` = \'' . $data . '\'';
	}
	$user_id = (int)getSession('user_id');
	mysql_update("UPDATE `znote_accounts` SET " . implode(', ', $update) . " WHERE `account_id`=" . $user_id . ";");
}

// CHANGE PASSWORD
function user_change_password($user_id, $password) {
	$user_id = (int)$user_id;
	$auth = cipsoft_hash_password($password);
	$auth_hex = bin2hex($auth);
	mysql_update("UPDATE `Accounts` SET `Auth` = UNHEX('$auth_hex') WHERE `AccountID` = '$user_id';");
}

function user_character_set_hide($char_id, $value) {
	$char_id = (int)$char_id;
	$value = (int)$value;
	mysql_update("UPDATE `znote_players` SET `hide_char`='$value' WHERE `player_id`='$char_id';");
}

// CREATE ACCOUNT
function user_create_account($register_data, $maildata) {
	$acc_id = isset($register_data['id']) ? (int)$register_data['id'] : (isset($register_data['name']) ? (int)$register_data['name'] : rand(100000, 999999));
	
	// Garantir unicidade do AccountID
	while (user_exist($acc_id)) {
		$acc_id = rand(100000, 999999);
	}

	$email = sanitize($register_data['email']);
	$rawPassword = $register_data['password'];
	$auth = cipsoft_hash_password($rawPassword);
	$auth_hex = bin2hex($auth);

	$ip = isset($register_data['ip']) ? $register_data['ip'] : getIP();
	$created = time();
	$flag = isset($register_data['flag']) ? sanitize($register_data['flag']) : 'unknown';

	// Inserir na tabela nativa CipSoft Accounts
	mysql_insert("INSERT INTO `Accounts` (`AccountID`, `Email`, `Auth`, `PremiumEnd`, `PendingPremiumDays`, `Deleted`) 
		VALUES ('$acc_id', '$email', UNHEX('$auth_hex'), '0', '0', '0');");

	// Inserir metadados Znote AAC
	$activeKey = rand(100000000, 999999999);
	$active = ($maildata['register']) ? 0 : 1;
	mysql_insert("INSERT INTO `znote_accounts` (`account_id`, `ip`, `created`, `active`, `active_email`, `activekey`, `flag`) 
		VALUES ('$acc_id', '$ip', '$created', '$active', '0', '$activeKey', '$flag');");

	if ($maildata['register']) {
		$thisurl = config('site_url') . "$_SERVER[REQUEST_URI]?authenticate&u=" . $acc_id . "&k=" . $activeKey;
		$mailer = new Mail($maildata);
		$title = "Account Activation - " . $maildata['fromName'];
		$body = "<h1>Account Activation</h1>";
		$body .= "<p>Click the following link to activate your account:</p>";
		$body .= "<p><a href='$thisurl'>$thisurl</a></p>";
		$mailer->sendMail($email, $title, $body, (string)$acc_id);
	}

	return $acc_id;
}

// CREATE CHARACTER
function user_create_character($character_data) {
	array_walk($character_data, 'array_sanitize');
	$cnf = fullConfig();

	$account_id = (int)$character_data['account_id'];
	$name = sanitize($character_data['name']);
	$sex = (int)$character_data['sex']; // 0 = Female, 1 = Male
	$vocation_id = (int)$character_data['vocation'];
	$town_id = (int)$character_data['town_id'];

	$playercnf = $cnf['player'];
	$base = $playercnf['base'];
	$create = $playercnf['create'];
	$skills = $create['skills'][$vocation_id];
	$gains = $cnf['vocations_gain'][$vocation_id];

	$level = $create['level'];
	$residence = town_id_to_name($town_id);
	$prof_name = vocation_id_to_name($vocation_id);

	// No vocation adjustments
	if ($vocation_id === 0) {
		$level = 1;
		$town_id = 10;
		$residence = 'Rookgaard';
		$prof_name = 'None';
		$health = 150;
		$mana = 0;
		$cap = 400;
		$exp = 0;
		$speed = 220;
		$nextExp = 100;

		$posX = 32097;
		$posY = 32205;
		$posZ = 7;
	} else {
		$leveldiff = $level - $base['level'];
		$health = $base['health'] + ($gains['hp'] * $leveldiff);
		$mana = $base['mana'] + ($gains['mp'] * $leveldiff);
		$cap = $base['cap'] + ($gains['cap'] * $leveldiff);
		$exp = level_to_experience($level);
		$speed = 220 + (2 * ($level - 1));
		$nextExp = level_to_experience($level + 1);

		if (isset($cnf['town_spawns'][$town_id])) {
			$posX = $cnf['town_spawns'][$town_id]['x'];
			$posY = $cnf['town_spawns'][$town_id]['y'];
			$posZ = $cnf['town_spawns'][$town_id]['z'];
		} else {
			$posX = $cnf['default_pos']['x'];
			$posY = $cnf['default_pos']['y'];
			$posZ = $cnf['default_pos']['z'];
		}
	}

	$outfit = ($sex == 1) ? $create['male_outfit'] : $create['female_outfit'];

	$world_id = isset($cnf['world_id']) ? (int)$cnf['world_id'] : 1;

	// 1. Insert into Characters
	mysql_insert("INSERT INTO `Characters` (`WorldID`, `AccountID`, `Name`, `Sex`, `Level`, `Profession`, `Residence`, `LastLoginTime`, `TutorActivities`, `IsOnline`, `Deleted`) 
		VALUES ('$world_id', '$account_id', '$name', '$sex', '$level', '$prof_name', '$residence', '0', '0', '0', '0');");

	$char_id = user_character_id($name);
	if (!$char_id) return false;

	// 2. Insert into PlayerState (Required for CipSoft game server)
	mysql_insert("INSERT INTO `PlayerState` (`CharacterID`, `Race`, `Profession`, `OrigLookType`, `OrigLookHead`, `OrigLookBody`, `OrigLookLegs`, `OrigLookFeet`, `CurrLookType`, `CurrLookHead`, `CurrLookBody`, `CurrLookLegs`, `CurrLookFeet`, `StartX`, `StartY`, `StartZ`, `PosX`, `PosY`, `PosZ`, `PlayerkillerEnd`) 
		VALUES ('$char_id', '1', '$vocation_id', '{$outfit['id']}', '{$outfit['head']}', '{$outfit['body']}', '{$outfit['legs']}', '{$outfit['feet']}', '{$outfit['id']}', '{$outfit['head']}', '{$outfit['body']}', '{$outfit['legs']}', '{$outfit['feet']}', '$posX', '$posY', '$posZ', '$posX', '$posY', '$posZ', '0');");

	// 3. Insert into PlayerSkills (16 columns: SkillID 0=Level, 1=Magic, 2=HP, 3=Mana, 4=Speed, 5=Cap, 6=Shield, 7=Dist, 8=Sword, 9=Club, 10=Axe, 11=Fist, 13=Fish, 22=Soul)
	$speed = 220 + (2 * ($level - 1));
	$nextExp = level_to_experience($level + 1);

	$skillsToInsert = array(
		array(0, $level, $level, 0, 0, 0, 0, 0, 0, 0, 0, $exp, 0, $nextExp, 0),
		array(1, $skills['magic'], $skills['magic'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(2, $health, $health, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(3, $mana, $mana, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(4, $speed, $speed, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(5, $cap, $cap, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(6, $skills['shield'], $skills['shield'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(7, $skills['dist'], $skills['dist'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(8, $skills['sword'], $skills['sword'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(9, $skills['club'], $skills['club'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(10, $skills['axe'], $skills['axe'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(11, $skills['fist'], $skills['fist'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(13, $skills['fishing'], $skills['fishing'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
		array(22, 100, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	);

	foreach ($skillsToInsert as $s) {
		mysql_insert("REPLACE INTO `PlayerSkills` (`CharacterID`, `SkillID`, `Actual`, `Maximum`, `Minimum`, `DeltaAct`, `MagicDeltaAct`, `Cycle`, `MaxCycle`, `Count`, `MaxCount`, `AddLevel`, `Experience`, `FactorPercent`, `NextLevel`, `Delta`) 
			VALUES ('$char_id', '{$s[0]}', '{$s[1]}', '{$s[2]}', '{$s[3]}', '{$s[4]}', '{$s[5]}', '{$s[6]}', '{$s[7]}', '{$s[8]}', '{$s[9]}', '{$s[10]}', '{$s[11]}', '{$s[12]}', '{$s[13]}', '{$s[14]}');");
	}

	// 4. Insert into znote_players
	$created_time = time();
	mysql_insert("INSERT INTO `znote_players` (`player_id`, `created`, `hide_char`, `comment`) VALUES ('$char_id', '$created_time', '0', '');");

	return $char_id;
}

// Online counter
function user_count_online() {
	$data = mysql_select_single("SELECT COUNT(`CharacterID`) AS `count` FROM `Characters` WHERE `IsOnline` = 1 AND `Deleted` = 0;");
	return ($data !== false) ? $data['count'] : 0;
}

// Account counter
function user_count_accounts() {
	$result = mysql_select_single("SELECT COUNT(`AccountID`) AS `id` FROM `Accounts` WHERE `Deleted` = 0;");
	return ($result !== false) ? $result['id'] : 0;
}

// Fetch character data
function user_character_data($user_id) {
	$user_id = (int)$user_id;
	$func_num_args = func_num_args();
	$func_get_args = func_get_args();

	if ($func_num_args > 1) {
		unset($func_get_args[0]);

		// Mapear campos comuns para CipSoft Characters / PlayerState / PlayerSkills
		$aliases = array(
			'id' => '`CharacterID` AS `id`',
			'name' => '`Name` AS `name`',
			'account_id' => '`AccountID` AS `account_id`',
			'level' => '`Level` AS `level`',
			'vocation' => '`Profession` AS `vocation`',
			'town_id' => '`Residence` AS `town_id`',
			'sex' => '`Sex` AS `sex`',
			'lastlogin' => '`LastLoginTime` AS `lastlogin`',
			'online' => '`IsOnline` AS `online`',
			'deleted' => '`Deleted` AS `deleted`'
		);

		$fields = array();
		foreach ($func_get_args as $arg) {
			$arg_lower = strtolower($arg);
			if (isset($aliases[$arg_lower])) {
				$fields[] = $aliases[$arg_lower];
			} else {
				$fields[] = '`' . sanitize($arg) . '`';
			}
		}

		$sql = "SELECT " . implode(', ', $fields) . " FROM `Characters` WHERE `CharacterID` = $user_id LIMIT 1;";
		return mysql_select_single($sql);
	}
	return false;
}

function user_znote_character_data($character_id) {
	$charid = (int)$character_id;
	$func_num_args = func_num_args();
	$func_get_args = func_get_args();

	if ($func_num_args > 1) {
		unset($func_get_args[0]);
		$fields = '`' . implode('`, `', $func_get_args) . '`';
		return mysql_select_single("SELECT $fields FROM `znote_players` WHERE `player_id` = $charid LIMIT 1;");
	}
	return false;
}

function user_znote_data() {
	$func_num_args = func_num_args();
	$func_get_args = func_get_args();
	if ($func_num_args > 0) {
		$fields = '`' . implode('`, `', $func_get_args) . '`';
		return mysql_select_single("SELECT $fields FROM `znote`;");
	}
	return false;
}

function user_znote_account_data($account_id) {
	$accid = (int)$account_id;
	$func_num_args = func_num_args();
	$func_get_args = func_get_args();
	if ($func_num_args > 1) {
		unset($func_get_args[0]);
		$fields = '`' . implode('`, `', $func_get_args) . '`';
		return mysql_select_single("SELECT $fields FROM `znote_accounts` WHERE `account_id` = $accid LIMIT 1;");
	}
	return false;
}

function user_data($user_id) {
	$user_id = (int)$user_id;
	$func_num_args = func_num_args();
	$func_get_args = func_get_args();

	if ($func_num_args > 1) {
		unset($func_get_args[0]);
		$aliases = array(
			'id' => '`AccountID` AS `id`',
			'name' => '`AccountID` AS `name`',
			'email' => '`Email` AS `email`',
			'premdays' => '`PremiumEnd` AS `premdays`',
			'auth' => '`Auth` AS `auth`',
			'deleted' => '`Deleted` AS `deleted`'
		);

		$fields = array();
		foreach ($func_get_args as $arg) {
			$arg_lower = strtolower($arg);
			if (isset($aliases[$arg_lower])) {
				$fields[] = $aliases[$arg_lower];
			} else {
				$fields[] = '`' . sanitize($arg) . '`';
			}
		}

		$data = mysql_select_single("SELECT " . implode(', ', $fields) . " FROM `Accounts` WHERE `AccountID` = $user_id LIMIT 1;");
		if ($data !== false && isset($data['premdays'])) {
			$now = time();
			$diff = (int)$data['premdays'] - $now;
			$data['premdays'] = ($diff > 0) ? (int)ceil($diff / 86400) : 0;
		}
		return $data;
	}
	return false;
}

function user_exist($username) {
	$username = (int)$username;
	$data = mysql_select_single("SELECT `AccountID` FROM `Accounts` WHERE `AccountID`='$username' AND `Deleted` = 0 LIMIT 1;");
	return ($data !== false);
}

function user_name($id) {
	$id = (int)$id;
	$name = mysql_select_single("SELECT `Name` FROM `Characters` WHERE `CharacterID`='$id' LIMIT 1;");
	return ($name !== false) ? $name['Name'] : false;
}

function user_character_exist($username) {
	$username = sanitize($username);
	$player = mysql_select_single("SELECT `CharacterID` FROM `Characters` WHERE `Name`='$username' AND `Deleted` = 0 LIMIT 1;");
	return ($player !== false) ? $player['CharacterID'] : false;
}

function user_email_exist($email) {
	$email = sanitize($email);
	$data = mysql_select_single("SELECT `AccountID` FROM `Accounts` WHERE `Email`='$email' AND `Deleted` = 0 LIMIT 1;");
	return ($data !== false);
}

function user_id_from_email($email) {
	$email = sanitize($email);
	$data = mysql_select_single("SELECT `AccountID` FROM `Accounts` WHERE `Email`='$email' AND `Deleted` = 0 LIMIT 1;");
	return ($data !== false) ? $data['AccountID'] : false;
}

function user_id($username) {
	$username = (int)$username;
	$data = mysql_select_single("SELECT `AccountID` FROM `Accounts` WHERE `AccountID`='$username' AND `Deleted` = 0 LIMIT 1;");
	return ($data !== false) ? $data['AccountID'] : false;
}

function user_login_id($username, $password) {
	return user_login($username, $password);
}

function user_character_id($charname) {
	$charname = sanitize($charname);
	$char = mysql_select_single("SELECT `CharacterID` FROM `Characters` WHERE `Name`='$charname' AND `Deleted` = 0 LIMIT 1;");
	return ($char !== false) ? $char['CharacterID'] : false;
}

function user_character_name($charID) {
	$charID = (int)$charID;
	$char = mysql_select_single("SELECT `Name` FROM `Characters` WHERE `CharacterID` = '$charID' LIMIT 1;");
	return ($char !== false) ? $char['Name'] : false;
}

function user_character_hide($username) {
	$cid = user_character_id($username);
	if (!$cid) return 0;
	$char = mysql_select_single("SELECT `hide_char` FROM `znote_players` WHERE `player_id`='$cid' LIMIT 1;");
	return ($char !== false) ? $char['hide_char'] : 0;
}

// LOGIN CIPSOFT
function user_login($username, $password) {
	$acc_id = (int)$username;
	if ($acc_id <= 0) return false;

	$acc = mysql_select_single("SELECT `AccountID`, `Auth` FROM `Accounts` WHERE `AccountID`='$acc_id' AND `Deleted` = 0 LIMIT 1;");
	if ($acc === false) return false;

	if (cipsoft_verify_password($password, $acc['Auth'])) {
		return $acc['AccountID'];
	}
	return false;
}

function user_logged_in() {
	return (getSession('user_id') !== false);
}
?>
