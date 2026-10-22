<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if ($config['log_ip']) {
	znote_visitor_insert_detailed_data(4);
}

if (!empty($_GET['name'])) {
	$name = sanitize($_GET['name']);
	$user_id = user_character_exist($name);

	if ($user_id !== false) {
		$char = mysql_select_single("SELECT `c`.`CharacterID`, `c`.`AccountID`, `c`.`Name`, `c`.`Sex`, `c`.`Level`, `c`.`Profession`, `c`.`Residence`, `c`.`LastLoginTime`, `c`.`IsOnline`
			FROM `Characters` AS `c`
			WHERE `c`.`CharacterID` = '$user_id' AND `c`.`Deleted` = 0 LIMIT 1;");

		if ($char === false) {
			echo '<div class="alert alert-warning">Character not found.</div>';
			include 'layout/overall/footer.php';
			exit();
		}

		$znote_char = user_znote_character_data($user_id, 'created', 'hide_char', 'comment');
		$account_flag = user_znote_account_data($char['AccountID'], 'flag');

		// Guild data
		$guild_data = mysql_select_single("SELECT `g`.`GuildID`, `g`.`Name` AS `guild_name`, `gr`.`Name` AS `rank_name` 
			FROM `GuildMembers` AS `gm`
			INNER JOIN `Guilds` AS `g` ON `gm`.`GuildID` = `g`.`GuildID`
			LEFT JOIN `GuildRanks` AS `gr` ON `gm`.`GuildID` = `gr`.`GuildID` AND `gm`.`Rank` = `gr`.`Rank`
			WHERE `gm`.`CharacterID` = '$user_id' LIMIT 1;");

		// House data
		$houses = mysql_select_multi("SELECT `h`.`Name`, `h`.`Town` AS `TownID` 
			FROM `Houses` AS `h`
			INNER JOIN `HouseOwners` AS `ho` ON `h`.`HouseID` = `ho`.`HouseID`
			WHERE `ho`.`OwnerID` = '$user_id';");

		// Deletion flag
		$deletion_time = mysql_select_single("SELECT `time` FROM `znote_deleted_characters` WHERE `character_name`='$name' AND `done` = '0' LIMIT 1;");
		?>

		<div class="profile-container">
			<h1>Character Profile: <?php echo htmlspecialchars($char['Name']); ?></h1>

			<?php if ($deletion_time !== false): ?>
				<div class="alert alert-danger">
					<strong>Attention:</strong> This character is scheduled for deletion at: <?php echo $deletion_time['time']; ?>.
				</div>
			<?php endif; ?>

			<table class="table table-striped table-hover">
				<tbody>
					<tr>
						<td style="width: 180px;"><strong>Name:</strong></td>
						<td>
							<strong><?php echo htmlspecialchars($char['Name']); ?></strong>
							<?php if ($config['country_flags']['enabled'] && !empty($account_flag['flag'])): ?>
								<img src="flags/<?php echo htmlspecialchars($account_flag['flag']); ?>.png" alt="flag" style="vertical-align: middle; margin-left: 5px;">
							<?php endif; ?>
						</td>
					</tr>
					<tr>
						<td><strong>Sex:</strong></td>
						<td><?php echo ($char['Sex'] == 1) ? 'Male' : 'Female'; ?></td>
					</tr>
					<tr>
						<td><strong>Level:</strong></td>
						<td><?php echo $char['Level']; ?></td>
					</tr>
					<tr>
						<td><strong>Vocation:</strong></td>
						<td><?php echo htmlspecialchars($char['Profession']); ?></td>
					</tr>
					<tr>
						<td><strong>Residence:</strong></td>
						<td><?php echo htmlspecialchars($char['Residence']); ?></td>
					</tr>
					<?php if ($guild_data !== false): ?>
						<tr>
							<td><strong>Guild:</strong></td>
							<td>
								<strong><?php echo htmlspecialchars($guild_data['rank_name']); ?></strong> of the 
								<a href="guilds.php?name=<?php echo urlencode($guild_data['guild_name']); ?>">
									<?php echo htmlspecialchars($guild_data['guild_name']); ?>
								</a>
							</td>
						</tr>
					<?php endif; ?>
					<?php if ($houses !== false): foreach ($houses as $h): ?>
						<tr>
							<td><strong>House:</strong></td>
							<td><?php echo htmlspecialchars($h['Name']); ?> (<?php echo town_id_to_name($h['TownID']); ?>)</td>
						</tr>
					<?php endforeach; endif; ?>
					<tr>
						<td><strong>Last Login:</strong></td>
						<td><?php echo ($char['LastLoginTime'] > 0) ? getClock($char['LastLoginTime'], true, true) : 'Never logged in.'; ?></td>
					</tr>
					<tr>
						<td><strong>Status:</strong></td>
						<td>
							<?php if ($char['IsOnline'] == 1): ?>
								<span style="color: green; font-weight: bold;">Online</span>
							<?php else: ?>
								<span style="color: gray;">Offline</span>
							<?php endif; ?>
						</td>
					</tr>
					<?php if (!empty($znote_char['created'])): ?>
						<tr>
							<td><strong>Created:</strong></td>
							<td><?php echo getClock($znote_char['created'], true); ?></td>
						</tr>
					<?php endif; ?>
				</tbody>
			</table>

			<?php if (!empty($znote_char['comment'])): ?>
				<h3>Character Comment</h3>
				<div class="character-comment" style="padding: 10px; background: rgba(0,0,0,0.05); border-left: 3px solid #b89b4b; border-radius: 4px; margin-bottom: 20px;">
					<?php echo nl2br(htmlspecialchars($znote_char['comment'])); ?>
				</div>
			<?php endif; ?>

			<h3>Death History</h3>
			<table class="table table-striped table-hover">
				<thead>
					<tr class="yellow">
						<th style="width: 180px;">Date / Time</th>
						<th>Death Description</th>
					</tr>
				</thead>
				<tbody>
					<?php
					$deaths = user_fetch_deathlist($user_id);
					if ($deaths) {
						foreach ($deaths as $d) {
							$killer_link = ($d['is_player'] == 1) 
								? '<a href="characterprofile.php?name=' . urlencode($d['killed_by']) . '"><strong>' . htmlspecialchars($d['killed_by']) . '</strong></a>' 
								: htmlspecialchars($d['killed_by']);
							?>
							<tr>
								<td><?php echo getClock($d['time'], true, true); ?></td>
								<td>Killed at level <?php echo $d['level']; ?> by <?php echo $killer_link; ?>.</td>
							</tr>
							<?php
						}
					} else {
						?>
						<tr>
							<td colspan="2">This character has never died.</td>
						</tr>
						<?php
					}
					?>
				</tbody>
			</table>

			<?php
			// Other visible characters on the same account
			$otherChars = mysql_select_multi("SELECT `c`.`CharacterID`, `c`.`Name`, `c`.`Level`, `c`.`Profession`, `c`.`LastLoginTime`, `c`.`IsOnline` 
				FROM `Characters` AS `c`
				LEFT JOIN `znote_players` AS `zp` ON `c`.`CharacterID` = `zp`.`player_id`
				WHERE `c`.`AccountID` = '{$char['AccountID']}' AND `c`.`CharacterID` != '$user_id' AND `c`.`Deleted` = 0 AND (`zp`.`hide_char` IS NULL OR `zp`.`hide_char` = 0)
				ORDER BY `c`.`Level` DESC LIMIT 30;");

			if ($otherChars):
				?>
				<h3>Other Characters on this Account</h3>
				<div class="table-responsive">
					<table class="table table-striped table-hover">
						<thead>
							<tr class="yellow">
								<th>Name</th>
								<th>Level</th>
								<th>Vocation</th>
								<th>Last Login</th>
								<th>Status</th>
							</tr>
						</thead>
						<tbody>
							<?php foreach ($otherChars as $oc): ?>
								<tr>
									<td><a href="characterprofile.php?name=<?php echo urlencode($oc['Name']); ?>"><strong><?php echo htmlspecialchars($oc['Name']); ?></strong></a></td>
									<td><?php echo $oc['Level']; ?></td>
									<td><?php echo htmlspecialchars($oc['Profession']); ?></td>
									<td><?php echo ($oc['LastLoginTime'] > 0) ? getClock($oc['LastLoginTime'], true, true) : 'Never'; ?></td>
									<td><?php echo ($oc['IsOnline'] == 1) ? '<span style="color:green;">Online</span>' : '<span style="color:gray;">Offline</span>'; ?></td>
								</tr>
							<?php endforeach; ?>
						</tbody>
					</table>
				</div>
			<?php endif; ?>
		</div>
		<?php
	} else {
		echo '<div class="alert alert-danger">Character <strong>' . htmlspecialchars($name) . '</strong> not found.</div>';
	}
} else {
	header('Location: index.php');
	exit();
}

include 'layout/overall/footer.php';
?>
