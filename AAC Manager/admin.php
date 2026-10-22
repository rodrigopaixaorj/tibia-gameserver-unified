<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if (!isset($_SESSION['csrf_token'])) {
	$_SESSION['csrf_token'] = bin2hex(random_bytes(16));
}

protect_page();
admin_only($user_data);

$enc = 100;
$messages = array();

if (!empty($_POST)) {
	if (empty($_POST['csrf_token']) || !hash_equals($_POST['csrf_token'], $_SESSION['csrf_token'])) {
		http_response_code(400);
		die("Error: Invalid CSRF token!");
	}

	// Ban System
	if (!empty($_POST['ban_char']) && !empty($_POST['ban_reason']) && !empty($_POST['ban_time']) && !empty($_POST['ban_comment'])) {
		$charname = sanitize($_POST['ban_char']);
		if (user_character_exist($charname)) {
			$reasonid = (int)$_POST['ban_reason'] - $enc;
			$time = (int)$_POST['ban_time'] - $enc;
			$comment = sanitize($_POST['ban_comment']);

			if (set_rule_violation($charname, 1, 1, $reasonid, $time, $comment)) {
				$messages[] = 'Ban successfully applied to character ' . htmlspecialchars($charname) . '.';
			} else {
				$messages[] = 'Error recording ban for character.';
			}
		} else {
			$messages[] = 'Character ' . htmlspecialchars($charname) . ' does not exist.';
		}
	}

	// Permanently Delete Character
	if (!empty($_POST['del_name'])) {
		$del_name = sanitize($_POST['del_name']);
		$cid = user_character_id($del_name);
		if ($cid !== false) {
			user_delete_character($cid);
			$messages[] = 'Character ' . htmlspecialchars($del_name) . ' was permanently deleted.';
		} else {
			$messages[] = 'Character ' . htmlspecialchars($del_name) . ' was not found.';
		}
	}

	// Reset Account Password
	if (!empty($_POST['reset_pass']) && !empty($_POST['new_pass'])) {
		$char_name = sanitize($_POST['reset_pass']);
		if (user_character_exist($char_name)) {
			$acc_id = user_character_account_id($char_name);
			user_change_password($acc_id, $_POST['new_pass']);
			$messages[] = 'The password for account (' . $acc_id . ') of character ' . htmlspecialchars($char_name) . ' was changed to: ' . htmlspecialchars($_POST['new_pass']) . '.';
		} else {
			$messages[] = 'Character not found.';
		}
	}

	// Give Shop Points
	if (!empty($_POST['points_char']) && !empty($_POST['points_value'])) {
		$char = sanitize($_POST['points_char']);
		$points = (int)$_POST['points_value'];
		$account = mysql_select_single("SELECT `AccountID` FROM `Characters` WHERE `Name`='$char' LIMIT 1;");
		if ($account !== false) {
			$acc_id = $account['AccountID'];
			$znote_account = mysql_select_single("SELECT `points` FROM `znote_accounts` WHERE `account_id`='$acc_id' LIMIT 1;");
			$currPoints = ($znote_account !== false) ? (int)$znote_account['points'] : 0;
			$totalPoints = $currPoints + $points;
			mysql_update("UPDATE `znote_accounts` SET `points`='$totalPoints' WHERE `account_id`='$acc_id';");
			$messages[] = 'Added ' . $points . ' points to account ' . $acc_id . ' (Total: ' . $totalPoints . ').';
		} else {
			$messages[] = 'Character not found.';
		}
	}

	// Teleport Player (Updates PlayerState)
	if (!empty($_POST['teleport_char'])) {
		$tp_char = sanitize($_POST['teleport_char']);
		$tp_cid = user_character_id($tp_char);
		if ($tp_cid !== false) {
			$town_id = (int)$_POST['teleport_town'];
			$defPos = $config['default_pos'];
			mysql_update("UPDATE `PlayerState` SET `PosX`='{$defPos['x']}', `PosY`='{$defPos['y']}', `PosZ`='{$defPos['z']}' WHERE `CharacterID`='$tp_cid' LIMIT 1;");
			if (isset($config['towns'][$town_id])) {
				$townName = $config['towns'][$town_id];
				mysql_update("UPDATE `Characters` SET `Residence`='$townName' WHERE `CharacterID`='$tp_cid' LIMIT 1;");
			}
			$messages[] = 'Character ' . htmlspecialchars($tp_char) . ' was successfully teleported to temple default position.';
		} else {
			$messages[] = 'Character not found.';
		}
	}
}

?>

<h1>Administration Panel</h1>

<?php
if (!empty($messages)) {
	echo '<div class="alert alert-info"><ul>';
	foreach ($messages as $msg) {
		echo '<li>' . $msg . '</li>';
	}
	echo '</ul></div>';
}
?>

<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
	<!-- Bans -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Ban Player</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<label for="ban_char">Character Name:</label>
			<input type="text" name="ban_char" id="ban_char" class="form-control" required style="margin-bottom: 8px;">

			<label for="ban_reason">Reason:</label>
			<select name="ban_reason" id="ban_reason" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['ban_reason'] as $id => $reason): ?>
					<option value="<?php echo $id + $enc; ?>"><?php echo htmlspecialchars($reason); ?></option>
				<?php endforeach; ?>
			</select>

			<label for="ban_time">Duration:</label>
			<select name="ban_time" id="ban_time" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['ban_time'] as $sec => $desc): ?>
					<option value="<?php echo $sec + $enc; ?>"><?php echo htmlspecialchars($desc); ?></option>
				<?php endforeach; ?>
			</select>

			<label for="ban_comment">Comment:</label>
			<input type="text" name="ban_comment" id="ban_comment" class="form-control" placeholder="Detailed comment" required style="margin-bottom: 12px;">

			<input type="submit" value="Apply Ban" class="btn btn-danger">
		</form>
	</div>

	<!-- Password and Points Management -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Reset Account Password</h3>
		<form action="admin.php" method="post" style="margin-bottom: 20px;">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="reset_pass" placeholder="Character name" class="form-control" required style="margin-bottom: 8px;">
			<input type="text" name="new_pass" placeholder="New password" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Change Password" class="btn btn-warning">
		</form>

		<h3>Deliver Shop Points</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="points_char" placeholder="Character name" class="form-control" required style="margin-bottom: 8px;">
			<input type="number" name="points_value" placeholder="Points amount" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Deliver Points" class="btn btn-primary">
		</form>
	</div>

	<!-- Teleport and Delete -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Teleport Character to Temple</h3>
		<form action="admin.php" method="post" style="margin-bottom: 20px;">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="teleport_char" placeholder="Character name" class="form-control" required style="margin-bottom: 8px;">
			<label for="teleport_town">Destination Town:</label>
			<select name="teleport_town" id="teleport_town" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['towns'] as $tid => $tname): ?>
					<option value="<?php echo $tid; ?>"><?php echo htmlspecialchars($tname); ?></option>
				<?php endforeach; ?>
			</select>
			<input type="submit" value="Teleport" class="btn btn-secondary">
		</form>

		<h3>Delete Character</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="del_name" placeholder="Character name" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Permanently Delete" class="btn btn-danger" onclick="return confirm('WARNING: Are you sure you want to delete this character permanently?');">
		</form>
	</div>
</div>

<?php include 'layout/overall/footer.php'; ?>
