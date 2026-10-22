<?php
require_once 'engine/init.php';
if ($config['require_login']['guilds']) protect_page();

include 'layout/overall/header.php';

function cipsoft_guild_list() {
	$cache = new Cache('engine/cache/guildlist');
	$cache->setExpiration(60);
	if ($cache->hasExpired()) {
		$guilds = mysql_select_multi("SELECT `g`.`GuildID` AS `id`, `g`.`Name` AS `name`, `g`.`Created` AS `creationdata`, '' AS `motd`, 
			(SELECT COUNT(`CharacterID`) FROM `GuildMembers` WHERE `GuildID` = `g`.`GuildID`) AS `total` 
			FROM `Guilds` AS `g` 
			ORDER BY `g`.`Name` ASC;");

		if ($guilds !== false) {
			for ($i = 0; $i < count($guilds); $i++) {
				$guilds[$i]['level'] = get_guild_level_data($guilds[$i]['id']);
			}
		}
		$cache->setContent($guilds);
		$cache->save();
	} else {
		$guilds = $cache->load();
	}
	return $guilds;
}

$user_chars = array();
$user_char_ids = array();
if (user_logged_in()) {
	$char_array = user_character_list($session_user_id);
	if ($char_array) {
		foreach ($char_array as $c) {
			$user_chars[] = $c['name'];
			$user_char_ids[] = (int)$c['id'];
		}
	}
}

// Single guild view
if (!empty($_GET['name'])) {
	$guild_name = sanitize($_GET['name']);
	$guild = get_guild_data($guild_name);

	if (!$guild) {
		echo '<div class="alert alert-danger">Guild not found.</div>';
		echo '<p><a href="guilds.php" class="btn btn-primary">Back to guild list</a></p>';
		include 'layout/overall/footer.php';
		exit();
	}

	$gid = (int)$guild['id'];
	$players = get_guild_players($gid);
	$ranks = get_guild_rank_data($gid);
	$inv_data = guild_invite_list($gid);

	// Highest access of current visitor (3=Leader, 2=Vice-Leader, 1=Member, 0=None)
	$highest_access = 0;
	if (user_logged_in() && $players) {
		foreach ($players as $p) {
			if (in_array((int)$p['id'], $user_char_ids)) {
				$rank_lvl = (int)$p['rank_level'];
				if ($rank_lvl > $highest_access) {
					$highest_access = $rank_lvl;
				}
			}
		}
	}

	// Handle POST
	if (user_logged_in() && !empty($_POST)) {
		// Accept invitation
		if (!empty($_POST['joinguild'])) {
			$join_id = (int)$_POST['joinguild'];
			if (in_array($join_id, $user_char_ids)) {
				if (!user_is_online($join_id)) {
					if (get_character_guild_rank($join_id) === false) {
						guild_player_join($join_id, $gid);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">This character already belongs to a guild.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Character must be offline in-game to join the guild.</div>';
				}
			}
		}

		// Reject invitation
		if (!empty($_POST['reject_invite'])) {
			$uninv_id = (int)$_POST['reject_invite'];
			if (in_array($uninv_id, $user_char_ids) || $highest_access >= 2) {
				guild_remove_invitation($uninv_id, $gid);
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}
		}

		// Leave guild
		if (!empty($_POST['leave_guild'])) {
			$leave_id = (int)$_POST['leave_guild'];
			if (in_array($leave_id, $user_char_ids)) {
				if (!user_is_online($leave_id)) {
					$p_rank = get_character_guild_rank($leave_id);
					if ($p_rank != 3) {
						guild_player_leave($leave_id);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">Guild leader cannot leave without transferring leadership or disbanding the guild.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Character must be offline in-game to leave the guild.</div>';
				}
			}
		}

		// Vice-Leader & Leader Actions (Access >= 2)
		if ($highest_access >= 2) {
			// Invite player
			if (!empty($_POST['invite_player'])) {
				$inv_name = sanitize($_POST['invite_player']);
				$inv_cid = user_character_id($inv_name);
				if ($inv_cid !== false) {
					if (get_character_guild_rank($inv_cid) === false) {
						guild_invite_player($inv_cid, $gid, $session_user_id);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">That player is already in a guild.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Character not found.</div>';
				}
			}

			// Update Guild Nick/Title
			if (!empty($_POST['member_nick_char'])) {
				$nick_cid = user_character_id($_POST['member_nick_char']);
				$new_nick = sanitize($_POST['member_nick']);
				update_player_guildnick($nick_cid, $new_nick);
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}

			// Promote / Demote member
			if (!empty($_POST['promote_char']) && !empty($_POST['promote_rank'])) {
				$prom_cid = user_character_id($_POST['promote_char']);
				$prom_rid = (int)$_POST['promote_rank'];
				if ($prom_rid < 3 || $highest_access == 3) {
					update_player_guild_position($prom_cid, $prom_rid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				}
			}

			// Kick member
			if (!empty($_POST['kick_member'])) {
				$kick_cid = user_character_id($_POST['kick_member']);
				$kick_rank = get_character_guild_rank($kick_cid);
				if ($kick_rank < $highest_access) {
					guild_remove_member($kick_cid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				}
			}
		}

		// Leader Exclusive Actions (Access == 3)
		if ($highest_access == 3) {
			// Edit rank names
			if (isset($_POST['change_rank_titles'])) {
				foreach ($ranks as $r) {
					$fName = 'rank_name_' . $r['id'];
					if (!empty($_POST[$fName])) {
						guild_change_rank($gid, $r['id'], $_POST[$fName]);
					}
				}
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}

			// Transfer leadership
			if (!empty($_POST['new_guild_leader'])) {
				$new_lid = (int)$_POST['new_guild_leader'];
				$old_lid = guild_leader($gid);
				if (!user_is_online($new_lid) && !user_is_online($old_lid)) {
					guild_change_leader($new_lid, $old_lid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				} else {
					echo '<div class="alert alert-danger">Both characters must be offline to transfer leadership.</div>';
				}
			}

			// Disband guild
			if (isset($_POST['disband_guild'])) {
				guild_delete($gid);
				header('Location: guilds.php');
				exit();
			}
		}
	}
	?>

	<h1>Guild: <?php echo htmlspecialchars($guild['name']); ?></h1>

	<h2>Guild Members (<?php echo count($players); ?>)</h2>
	<div class="table-responsive">
		<table class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Rank</th>
					<th>Name</th>
					<th>Level</th>
					<th>Vocation</th>
					<th>Status</th>
				</tr>
			</thead>
			<tbody>
				<?php
				if ($players) {
					foreach ($players as $p) {
						$online_status = ($p['online'] == 1) ? '<span style="color:green; font-weight:bold;">Online</span>' : '<span style="color:gray;">Offline</span>';
						$nick = !empty($p['guildnick']) ? ' (' . htmlspecialchars($p['guildnick']) . ')' : '';
						?>
						<tr>
							<td><strong><?php echo htmlspecialchars($p['rank_name']); ?></strong></td>
							<td><a href="characterprofile.php?name=<?php echo urlencode($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?></a><?php echo $nick; ?></td>
							<td><?php echo $p['level']; ?></td>
							<td><?php echo htmlspecialchars($p['vocation']); ?></td>
							<td><?php echo $online_status; ?></td>
						</tr>
						<?php
					}
				}
				?>
			</tbody>
		</table>
	</div>

	<?php if (!empty($inv_data)): ?>
		<h3>Invited Characters</h3>
		<div class="table-responsive">
			<table class="table table-bordered">
				<thead>
					<tr>
						<th>Name</th>
						<th>Actions</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($inv_data as $inv): ?>
						<tr>
							<td><?php echo htmlspecialchars($inv['name']); ?></td>
							<td>
								<?php if (in_array((int)$inv['player_id'], $user_char_ids)): ?>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="joinguild" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Accept Invitation" class="btn btn-success btn-sm">
									</form>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="reject_invite" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Reject" class="btn btn-secondary btn-sm">
									</form>
								<?php elseif ($highest_access >= 2): ?>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="reject_invite" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Cancel Invitation" class="btn btn-danger btn-sm">
									</form>
								<?php else: ?>
									Pending player response
								<?php endif; ?>
							</td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		</div>
	<?php endif; ?>

	<?php
	// Option for members to leave
	if (user_logged_in() && $players) {
		$my_guild_chars = array();
		foreach ($players as $p) {
			if (in_array((int)$p['id'], $user_char_ids) && $p['rank_level'] != 3) {
				$my_guild_chars[] = $p;
			}
		}
		if (!empty($my_guild_chars)) {
			?>
			<h3>Leave Guild</h3>
			<form action="" method="post" class="form-inline">
				<select name="leave_guild" class="form-control" required>
					<?php foreach ($my_guild_chars as $mgc): ?>
						<option value="<?php echo $mgc['id']; ?>"><?php echo htmlspecialchars($mgc['name']); ?></option>
					<?php endforeach; ?>
				</select>
				<input type="submit" value="Leave Guild" class="btn btn-warning" onclick="return confirm('Are you sure you want to leave this guild?');">
			</form>
			<?php
		}
	}
	?>

	<?php if ($highest_access >= 2): ?>
		<hr>
		<h2>Guild Management Panel</h2>

		<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
			<div style="flex: 1; min-width: 280px;">
				<h3>Invite Player</h3>
				<form action="" method="post">
					<input type="text" name="invite_player" placeholder="Character name" class="form-control" required style="margin-bottom: 8px;">
					<input type="submit" value="Send Invitation" class="btn btn-primary">
				</form>
			</div>

			<div style="flex: 1; min-width: 280px;">
				<h3>Change Member Title</h3>
				<form action="" method="post">
					<select name="member_nick_char" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($players as $p): ?>
							<option value="<?php echo htmlspecialchars($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?></option>
						<?php endforeach; ?>
					</select>
					<input type="text" name="member_nick" placeholder="New title (or empty to clear)" class="form-control" style="margin-bottom: 8px;">
					<input type="submit" value="Update Title" class="btn btn-secondary">
				</form>

				<h3 style="margin-top: 15px;">Promote / Demote Member</h3>
				<form action="" method="post">
					<select name="promote_char" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($players as $p): if ($p['rank_level'] != 3): ?>
							<option value="<?php echo htmlspecialchars($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?> (<?php echo htmlspecialchars($p['rank_name']); ?>)</option>
						<?php endif; endforeach; ?>
					</select>
					<select name="promote_rank" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($ranks as $r): if ($r['level'] != 3 || $highest_access == 3): ?>
							<option value="<?php echo $r['id']; ?>"><?php echo htmlspecialchars($r['name']); ?></option>
						<?php endif; endforeach; ?>
					</select>
					<input type="submit" value="Change Rank" class="btn btn-secondary">
				</form>
			</div>
		</div>

		<?php if ($highest_access == 3): ?>
			<hr>
			<h3>Leader Exclusive Actions</h3>
			<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
				<div style="flex: 1; min-width: 280px;">
					<h4>Transfer Leadership</h4>
					<form action="" method="post">
						<select name="new_guild_leader" class="form-control" required style="margin-bottom: 8px;">
							<?php foreach ($players as $p): if ($p['rank_level'] != 3): ?>
								<option value="<?php echo $p['id']; ?>"><?php echo htmlspecialchars($p['name']); ?></option>
							<?php endif; endforeach; ?>
						</select>
						<input type="submit" value="Transfer Leadership" class="btn btn-warning" onclick="return confirm('Are you sure you want to transfer guild leadership?');">
					</form>
				</div>
				<div style="flex: 1; min-width: 280px;">
					<h4>Disband Guild</h4>
					<form action="" method="post">
						<input type="hidden" name="disband_guild" value="1">
						<input type="submit" value="Disband Guild Permanently" class="btn btn-danger" onclick="return confirm('WARNING: Disbanding the guild is irreversible. Are you sure?');">
					</form>
				</div>
			</div>
		<?php endif; ?>
	<?php endif; ?>

	<p style="margin-top: 25px;"><a href="guilds.php">&laquo; Back to all guilds</a></p>

	<?php
} else {
	// ALL GUILDS LIST
	$guilds = cipsoft_guild_list();
	?>
	<h1>Guilds</h1>

	<?php if (!empty($guilds)): ?>
		<div class="table-responsive">
			<table id="guildsTable" class="table table-striped table-hover">
				<thead>
					<tr class="yellow">
						<th>Guild Name</th>
						<th>Members</th>
						<th>Average Level</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($guilds as $g): ?>
						<tr onclick="window.location.href='guilds.php?name=<?php echo urlencode($g['name']); ?>';" style="cursor: pointer;">
							<td><a href="guilds.php?name=<?php echo urlencode($g['name']); ?>"><strong><?php echo htmlspecialchars($g['name']); ?></strong></a></td>
							<td><?php echo isset($g['level']['players']) ? $g['level']['players'] : 0; ?></td>
							<td><?php echo isset($g['level']['avg']) ? $g['level']['avg'] : 0; ?></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		</div>
	<?php else: ?>
		<div class="alert alert-info">No guilds have been created yet.</div>
	<?php endif; ?>

	<?php if (user_logged_in()): ?>
		<hr>
		<h2>Found a New Guild</h2>
		<?php
		if (!empty($_POST['selected_char']) && !empty($_POST['guild_name'])) {
			$founder_name = sanitize($_POST['selected_char']);
			$guild_create_name = sanitize($_POST['guild_name']);
			$founder_id = user_character_id($founder_name);

			if (in_array($founder_id, $user_char_ids)) {
				$char_info = user_character_data($founder_id, 'level');
				if ($char_info['level'] >= $config['create_guild_level']) {
					if (!user_is_online($founder_id)) {
						if (get_character_guild_rank($founder_id) === false) {
							if (preg_match("/^[a-zA-Z ]+$/", $guild_create_name) && strlen($guild_create_name) <= 30) {
								if (get_guild_id($guild_create_name) === false) {
									create_guild($founder_id, $guild_create_name);
									echo '<div class="alert alert-success">Guild <strong>' . htmlspecialchars($guild_create_name) . '</strong> created successfully! <a href="guilds.php?name=' . urlencode($guild_create_name) . '">Click here to manage it</a>.</div>';
								} else {
									echo '<div class="alert alert-danger">A guild with this name already exists.</div>';
								}
							} else {
								echo '<div class="alert alert-danger">Guild name can only contain letters and spaces (maximum 30 characters).</div>';
							}
						} else {
							echo '<div class="alert alert-danger">This character already belongs to a guild.</div>';
						}
					} else {
						echo '<div class="alert alert-danger">The founding character must be offline in-game.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Your character must be level ' . $config['create_guild_level'] . ' or higher to found a guild.</div>';
				}
			}
		}
		?>
		<form action="guilds.php" method="post" class="form-inline">
			<ul class="form-list">
				<li>
					<label for="selected_char">Founding Character:</label>
					<select name="selected_char" id="selected_char" required>
						<?php foreach ($user_chars as $uc): ?>
							<option value="<?php echo htmlspecialchars($uc); ?>"><?php echo htmlspecialchars($uc); ?></option>
						<?php endforeach; ?>
					</select>
				</li>
				<li>
					<label for="guild_name">Guild Name:</label>
					<input type="text" name="guild_name" id="guild_name" placeholder="e.g. Knights of Honor" required maxlength="30">
				</li>
				<li>
					<input type="submit" value="Found Guild" class="btn btn-primary">
				</li>
			</ul>
		</form>
	<?php else: ?>
		<p><a href="login.php">Login</a> to found a guild or manage invitations.</p>
	<?php endif; ?>

	<?php
}

include 'layout/overall/footer.php';
?>
