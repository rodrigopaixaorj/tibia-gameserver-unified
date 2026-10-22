<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
protect_page();
admin_only($user_data);

$message = '';
$error = '';

// Atualizar Skills via POST
if (isset($_POST['pid']) && (int)$_POST['pid'] > 0) {
	$pid = (int)$_POST['pid'];
	$status = user_is_online($pid);

	if (!$status) {
		$level = (int)$_POST['level'];
		$vocation_id = (int)$_POST['vocation'];
		$vocation_name = vocation_id_to_name($vocation_id);

		$magic = (int)$_POST['magic'];
		$fist = (int)$_POST['fist'];
		$club = (int)$_POST['club'];
		$sword = (int)$_POST['sword'];
		$axe = (int)$_POST['axe'];
		$dist = (int)$_POST['dist'];
		$shield = (int)$_POST['shield'];
		$fish = (int)$_POST['fish'];

		$statgain = $config['vocations_gain'][$vocation_id];
		$playercnf = $config['player'];
		$LevelsFromBase = $level - $playercnf['base']['level'];
		$newhp = $playercnf['base']['health'] + ($statgain['hp'] * $LevelsFromBase);
		$newmp = $playercnf['base']['mana'] + ($statgain['mp'] * $LevelsFromBase);
		$newcap = $playercnf['base']['cap'] + ($statgain['cap'] * $LevelsFromBase);
		$newexp = level_to_experience($level);
		$speed = 220 + (2 * ($level - 1));

		// 1. Atualizar Characters
		mysql_update("UPDATE `Characters` SET `Level`='$level', `Profession`='$vocation_name' WHERE `CharacterID`='$pid' LIMIT 1;");

		// 2. Atualizar PlayerState
		mysql_update("UPDATE `PlayerState` SET `Profession`='$vocation_id' WHERE `CharacterID`='$pid' LIMIT 1;");

		// 3. Atualizar PlayerSkills (0=Exp, 1=Magic, 2=HP, 3=Mana, 4=Speed, 5=Cap, 6=Shield, 7=Dist, 8=Sword, 9=Club, 10=Axe, 11=Fist, 13=Fish)
		$skillUpdates = array(
			0 => array('act' => $level, 'max' => $level, 'exp' => $newexp),
			1 => array('act' => $magic, 'max' => $magic, 'exp' => 0),
			2 => array('act' => $newhp, 'max' => $newhp, 'exp' => 0),
			3 => array('act' => $newmp, 'max' => $newmp, 'exp' => 0),
			4 => array('act' => $speed, 'max' => $speed, 'exp' => 0),
			5 => array('act' => $newcap, 'max' => $newcap, 'exp' => 0),
			6 => array('act' => $shield, 'max' => $shield, 'exp' => 0),
			7 => array('act' => $dist, 'max' => $dist, 'exp' => 0),
			8 => array('act' => $sword, 'max' => $sword, 'exp' => 0),
			9 => array('act' => $club, 'max' => $club, 'exp' => 0),
			10 => array('act' => $axe, 'max' => $axe, 'exp' => 0),
			11 => array('act' => $fist, 'max' => $fist, 'exp' => 0),
			13 => array('act' => $fish, 'max' => $fish, 'exp' => 0),
		);

		foreach ($skillUpdates as $skId => $vals) {
			$exists = mysql_select_single("SELECT `CharacterID` FROM `PlayerSkills` WHERE `CharacterID`='$pid' AND `SkillID`='$skId' LIMIT 1;");
			if ($exists) {
				mysql_update("UPDATE `PlayerSkills` SET `Actual`='{$vals['act']}', `Maximum`='{$vals['max']}', `Experience`='{$vals['exp']}' WHERE `CharacterID`='$pid' AND `SkillID`='$skId' LIMIT 1;");
			} else {
				mysql_insert("INSERT INTO `PlayerSkills` (`CharacterID`, `SkillID`, `Actual`, `Maximum`, `Experience`, `TargetCount`, `TargetActual`) VALUES ('$pid', '$skId', '{$vals['act']}', '{$vals['max']}', '{$vals['exp']}', 0, 0);");
			}
		}

		$message = 'Character skills and attributes updated successfully!';
	} else {
		$error = 'The player must be OFFLINE in-game to edit their skills.';
	}
}

$name = isset($_GET['name']) ? sanitize($_GET['name']) : '';
$charData = false;
$charSkills = array();

if (!empty($name)) {
	$cid = user_character_exist($name);
	if ($cid !== false) {
		$charData = mysql_select_single("SELECT `c`.`CharacterID`, `c`.`Name`, `c`.`Level`, `c`.`Profession`, `ps`.`Profession` AS `voc_id` 
			FROM `Characters` AS `c`
			LEFT JOIN `PlayerState` AS `ps` ON `c`.`CharacterID` = `ps`.`CharacterID`
			WHERE `c`.`CharacterID` = '$cid' LIMIT 1;");

		$rawSkills = mysql_select_multi("SELECT `SkillID`, `Actual` FROM `PlayerSkills` WHERE `CharacterID` = '$cid';");
		if ($rawSkills) {
			foreach ($rawSkills as $rs) {
				$charSkills[(int)$rs['SkillID']] = (int)$rs['Actual'];
			}
		}
	} else {
		$error = 'Character not found.';
	}
}

?>

<h1>Character Skill Editor</h1>

<?php if (!empty($message)): ?>
	<div class="alert alert-success"><?php echo $message; ?></div>
<?php endif; ?>
<?php if (!empty($error)): ?>
	<div class="alert alert-danger"><?php echo $error; ?></div>
<?php endif; ?>

<form action="admin_skills.php" method="get" class="form-inline" style="margin-bottom: 25px;">
	<label for="name"><strong>Search Character:</strong></label>
	<input type="text" name="name" id="name" class="form-control" placeholder="Character name" value="<?php echo htmlspecialchars($name); ?>" required style="margin: 0 10px;">
	<input type="submit" value="Load Skills" class="btn btn-primary">
</form>

<?php if ($charData !== false): ?>
	<h2>Editing: <?php echo htmlspecialchars($charData['Name']); ?></h2>
	<form action="admin_skills.php" method="post">
		<input type="hidden" name="pid" value="<?php echo $charData['CharacterID']; ?>">
		<input type="hidden" name="name" value="<?php echo htmlspecialchars($charData['Name']); ?>">

		<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
			<div style="flex: 1; min-width: 250px;">
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Level:</label>
					<input type="number" name="level" value="<?php echo $charData['Level']; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Vocation:</label>
					<select name="vocation" class="form-control">
						<?php foreach ($config['vocations'] as $vId => $vData): ?>
							<option value="<?php echo $vId; ?>" <?php if ((int)$charData['voc_id'] === $vId) echo 'selected'; ?>>
								<?php echo htmlspecialchars($vData['name']); ?>
							</option>
						<?php endforeach; ?>
					</select>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Magic Level:</label>
					<input type="number" name="magic" value="<?php echo isset($charSkills[1]) ? $charSkills[1] : 0; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Shielding:</label>
					<input type="number" name="shield" value="<?php echo isset($charSkills[6]) ? $charSkills[6] : 10; ?>" class="form-control" required>
				</div>
			</div>

			<div style="flex: 1; min-width: 250px;">
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Sword Fighting:</label>
					<input type="number" name="sword" value="<?php echo isset($charSkills[8]) ? $charSkills[8] : 10; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Axe Fighting:</label>
					<input type="number" name="axe" value="<?php echo isset($charSkills[10]) ? $charSkills[10] : 10; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Club Fighting:</label>
					<input type="number" name="club" value="<?php echo isset($charSkills[9]) ? $charSkills[9] : 10; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Distance Fighting:</label>
					<input type="number" name="dist" value="<?php echo isset($charSkills[7]) ? $charSkills[7] : 10; ?>" class="form-control" required>
				</div>
			</div>

			<div style="flex: 1; min-width: 250px;">
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Fist Fighting:</label>
					<input type="number" name="fist" value="<?php echo isset($charSkills[11]) ? $charSkills[11] : 10; ?>" class="form-control" required>
				</div>
				<div class="form-group" style="margin-bottom: 12px;">
					<label>Fishing:</label>
					<input type="number" name="fish" value="<?php echo isset($charSkills[13]) ? $charSkills[13] : 10; ?>" class="form-control" required>
				</div>
			</div>
		</div>

		<div style="margin-top: 15px;">
			<input type="submit" value="Save Skill Changes" class="btn btn-success">
		</div>
	</form>
<?php endif; ?>

<?php include 'layout/overall/footer.php'; ?>
