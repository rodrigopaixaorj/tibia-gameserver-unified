<?php
require_once 'engine/init.php';
protect_page();

$errors = array();

if (!empty($_POST)) {
	$required_fields = array('name', 'selected_town');
	foreach ($required_fields as $field) {
		if (empty($_POST[$field])) {
			$errors[] = 'You need to fill in all required fields.';
			break;
		}
	}

	if (empty($errors)) {
		if (!Token::isValid($_POST['token'])) {
			$errors[] = 'Invalid session token. Please try again.';
		}

		$name = validate_name($_POST['name']);
		if ($name === false) {
			$errors[] = 'Character name cannot have more than 2 words.';
		} else {
			if (user_character_exist($name) !== false) {
				$errors[] = 'Sorry, a character with this name already exists.';
			}
			if (!preg_match("/^[a-zA-Z ]+$/", $name)) {
				$errors[] = 'Character name can only contain letters a-z and spaces.';
			}
			if (strlen($name) < $config['minL'] || strlen($name) > $config['maxL']) {
				$errors[] = 'Character name must be between ' . $config['minL'] . ' and ' . $config['maxL'] . ' characters.';
			}

			$resname = explode(" ", $name);
			foreach ($resname as $res) {
				if (in_array(strtolower($res), $config['invalidNameTags'])) {
					$errors[] = 'The chosen name contains a disallowed word.';
				}
				if (strlen($res) == 1) {
					$errors[] = 'Single-letter words are not allowed in character names.';
				}
			}

			if (in_array(strtolower($name), $config['creatureNameTags'])) {
				$errors[] = 'The chosen name is the name of an existing creature.';
			}

			$selected_vocation = (int)$_POST['selected_vocation'];
			if (!in_array($selected_vocation, $config['available_vocations'])) {
				$errors[] = 'Invalid vocation selected.';
			}

			$selected_town = (int)$_POST['selected_town'];
			if (!in_array($selected_town, $config['available_towns'])) {
				$errors[] = 'Invalid town selected.';
			}

			// Rookgaard & No Vocation rule
			if ($selected_vocation === 0) {
				$selected_town = 10; // Enforce Rookgaard for No Vocation
			} else {
				if ($selected_town === 10) {
					$errors[] = 'Mainland vocations cannot start in Rookgaard. Please choose a mainland town.';
				}
			}

			$selected_gender = (int)$_POST['selected_gender'];
			if (!in_array($selected_gender, array(0, 1))) {
				$errors[] = 'Invalid gender selected.';
			}

			$char_count = user_character_list_count($session_user_id);
			if ($char_count >= $config['max_characters'] && !is_admin($user_data)) {
				$errors[] = 'Your account has already reached the maximum limit of ' . $config['max_characters'] . ' characters.';
			}
		}
	}

	if (empty($errors)) {
		if ($config['log_ip']) {
			znote_visitor_insert_detailed_data(2);
		}

		$character_data = array(
			'name'       => format_character_name($_POST['name']),
			'account_id' => $session_user_id,
			'vocation'   => (int)$_POST['selected_vocation'],
			'town_id'    => (int)$selected_town,
			'sex'        => (int)$_POST['selected_gender'],
			'lastip'     => getIPLong(),
			'created'    => time()
		);

		user_create_character($character_data);
		header('Location: createcharacter.php?success=1');
		exit();
	}
}

include 'layout/overall/header.php';
?>
<h1>Create Character</h1>
<?php
if (isset($_GET['success'])) {
	echo '<div class="alert alert-success"><h3>Congratulations!</h3><p>Your character was created successfully. See you in-game!</p><p><a href="myaccount.php" class="btn btn-primary">Back to My Account</a></p></div>';
} else {
	if (!empty($errors)) {
		echo output_errors($errors);
	}
	$selectedVocPost = isset($_POST['selected_vocation']) ? (int)$_POST['selected_vocation'] : 0;
	$selectedTownPost = isset($_POST['selected_town']) ? (int)$_POST['selected_town'] : 10;
	?>
	<form action="createcharacter.php" method="post" id="createCharForm">
		<ul class="form-list">
			<li>
				<label for="name">Character Name:</label>
				<input type="text" name="name" id="name" placeholder="e.g. Sir Lancelot" required autofocus value="<?php if (isset($_POST['name'])) echo htmlspecialchars($_POST['name']); ?>">
			</li>
			<li>
				<label for="selected_vocation">Vocation:</label>
				<select name="selected_vocation" id="selected_vocation" required onchange="updateTownOptions();">
					<?php foreach ($config['available_vocations'] as $vid): ?>
					<option value="<?php echo $vid; ?>" <?php if ($selectedVocPost === $vid) echo 'selected'; ?>>
						<?php 
							$vName = vocation_id_to_name($vid);
							echo htmlspecialchars(($vid === 0) ? $vName . ' (Level 1)' : $vName . ' (Level 8)'); 
						?>
					</option>
					<?php endforeach; ?>
				</select>
			</li>
			<li>
				<label for="selected_gender">Gender:</label>
				<select name="selected_gender" id="selected_gender" required>
					<option value="1" <?php if (isset($_POST['selected_gender']) && $_POST['selected_gender'] == 1) echo 'selected'; ?>>Male</option>
					<option value="0" <?php if (isset($_POST['selected_gender']) && $_POST['selected_gender'] == 0) echo 'selected'; ?>>Female</option>
				</select>
			</li>
			<li>
				<label for="selected_town">Starting Town:</label>
				<select name="selected_town" id="selected_town" required>
					<!-- Options will be populated dynamically based on selected vocation -->
				</select>
			</li>
			<?php Token::create(); ?>
			<li>
				<input type="submit" value="Create Character" class="btn btn-primary">
			</li>
		</ul>
	</form>

	<script type="text/javascript">
	var allTowns = <?php 
		$townMap = array();
		foreach ($config['available_towns'] as $tid) {
			$townMap[$tid] = town_id_to_name($tid);
		}
		echo json_encode($townMap);
	?>;

	function updateTownOptions() {
		var vocSelect = document.getElementById('selected_vocation');
		var townSelect = document.getElementById('selected_town');
		if (!vocSelect || !townSelect) return;

		var selectedVoc = parseInt(vocSelect.value, 10);
		var currentTown = townSelect.value;
		townSelect.innerHTML = '';

		if (selectedVoc === 0) {
			// No Vocation -> Rookgaard only (ID 10)
			var opt = document.createElement('option');
			opt.value = 10;
			opt.text = allTowns[10] ? allTowns[10] : 'Rookgaard';
			opt.selected = true;
			townSelect.appendChild(opt);
		} else {
			// Mainland vocations -> Towns 1 to 9
			var defaultTownId = 1;
			var isSet = false;
			for (var tid in allTowns) {
				if (allTowns.hasOwnProperty(tid)) {
					var idNum = parseInt(tid, 10);
					if (idNum !== 10) {
						var opt = document.createElement('option');
						opt.value = idNum;
						opt.text = allTowns[tid];
						if (currentTown == idNum) {
							opt.selected = true;
							isSet = true;
						}
						townSelect.appendChild(opt);
					}
				}
			}
			if (!isSet && townSelect.options.length > 0) {
				townSelect.selectedIndex = 0;
			}
		}
	}

	// Initialize on page load
	document.addEventListener('DOMContentLoaded', function() {
		updateTownOptions();
		<?php if (!empty($selectedTownPost)): ?>
		var ts = document.getElementById('selected_town');
		if (ts) {
			for (var i = 0; i < ts.options.length; i++) {
				if (ts.options[i].value == "<?php echo (int)$selectedTownPost; ?>") {
					ts.selectedIndex = i;
					break;
				}
			}
		}
		<?php endif; ?>
	});
	</script>
	<?php
}

include 'layout/overall/footer.php';
?>
