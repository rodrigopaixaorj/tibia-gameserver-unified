<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if ($config['log_ip']) {
	znote_visitor_insert_detailed_data(3);
}

// Fetch and sanitize GET values
$type = (isset($_GET['type'])) ? (int)$_GET['type'] : 7;
if ($type < 1 || $type > 9) $type = 7;

$vocation = (isset($_GET['vocation'])) ? $_GET['vocation'] : 'all';
$configVocations = $config['vocations'];
$vocationIds = array_keys($configVocations);

if ($vocation !== 'all') {
	$vocation = (int)$vocation;
	if (!in_array($vocation, $vocationIds)) {
		$vocation = "all";
	}
}

// Fetch highscore page
$page = isset($_GET['page']) ? (int)$_GET['page'] : 1;
if ($page <= 0) $page = 1;

$highscore = $config['highscore'];
$loadFlags = ($config['country_flags']['enabled'] && $config['country_flags']['highscores']);
$loadOutfits = ($config['show_outfits']['highscores']);

$rows = $highscore['rows'];
$rowsPerPage = $highscore['rowsPerPage'];

function skillName($type) {
	$types = array(
		1 => "Club Fighting",
		2 => "Sword Fighting",
		3 => "Axe Fighting",
		4 => "Distance Fighting",
		5 => "Shielding",
		6 => "Fishing",
		7 => "Experience / Level",
		8 => "Magic Level",
		9 => "Fist Fighting",
	);
	return isset($types[(int)$type]) ? $types[(int)$type] : 'Experience';
}

function pageCheck($index, $page, $rowPerPage) {
	return ($index < ($page * $rowPerPage) && $index >= ($page * $rowPerPage) - $rowPerPage);
}

$cache = new Cache('engine/cache/highscores');
$cache->setExpiration(60);

if ($cache->hasExpired()) {
	$vocGroups = fetchAllScores($rows, $config['ServerEngine'], $highscore['ignoreGroupId'], $configVocations, $vocation, $loadFlags, $loadOutfits);
	$cache->setContent($vocGroups);
	$cache->save();
} else {
	$vocGroups = $cache->load();
}

if ($vocGroups) {
	$vocGroup = (is_array($vocGroups[$vocation])) ? $vocGroups[$vocation] : $vocGroups[$vocGroups[$vocation]];
	$vocNameTitle = ($vocation === 'all') ? 'All Vocations' : vocation_id_to_name($vocation);
	?>

	<h1>Highscores: <?php echo skillName($type); ?> (<?php echo $vocNameTitle; ?>)</h1>

	<form action="highscores.php" method="GET" class="form-inline" style="margin-bottom: 20px;">
		<div style="display: flex; gap: 10px; flex-wrap: wrap; align-items: center;">
			<select name="type" class="form-control">
				<option value="7" <?php if ($type == 7) echo "selected"; ?>>Experience / Level</option>
				<option value="8" <?php if ($type == 8) echo "selected"; ?>>Magic Level</option>
				<option value="5" <?php if ($type == 5) echo "selected"; ?>>Shielding</option>
				<option value="2" <?php if ($type == 2) echo "selected"; ?>>Sword Fighting</option>
				<option value="3" <?php if ($type == 3) echo "selected"; ?>>Axe Fighting</option>
				<option value="1" <?php if ($type == 1) echo "selected"; ?>>Club Fighting</option>
				<option value="4" <?php if ($type == 4) echo "selected"; ?>>Distance Fighting</option>
				<option value="6" <?php if ($type == 6) echo "selected"; ?>>Fishing</option>
				<option value="9" <?php if ($type == 9) echo "selected"; ?>>Fist Fighting</option>
			</select>

			<select name="vocation" class="form-control">
				<option value="all" <?php if (!is_int($vocation)) echo "selected"; ?>>All Vocations</option>
				<?php
				foreach ($configVocations as $v_id => $v_data) {
					if ($v_data['fromVoc'] === false) {
						$selected = (is_int($vocation) && $vocation == $v_id) ? " selected" : "";
						echo '<option value="' . $v_id . '"' . $selected . '>' . $v_data['name'] . '</option>';
					}
				}
				?>
			</select>

			<select name="page" class="form-control">
				<?php
				$totalScores = isset($vocGroup[$type]) && is_array($vocGroup[$type]) ? count($vocGroup[$type]) : 0;
				$pages = ($totalScores > 0) ? ceil(min(($highscore['rows'] / $highscore['rowsPerPage']), ($totalScores / $highscore['rowsPerPage']))) : 1;
				for ($i = 1; $i <= $pages; $i++) {
					$sel = ($i == $page) ? " selected" : "";
					echo "<option value='" . $i . "'" . $sel . ">Page: " . $i . "</option>";
				}
				?>
			</select>

			<input type="submit" value="Filter" class="btn btn-primary">
		</div>
	</form>

	<div class="table-responsive">
		<table id="highscoresTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<?php if ($loadOutfits): ?><th style="width: 50px;">Outfit</th><?php endif; ?>
					<th style="width: 60px;">Rank</th>
					<th>Name</th>
					<th>Vocation</th>
					<th><?php echo ($type === 7) ? 'Level' : 'Skill'; ?></th>
				</tr>
			</thead>
			<tbody>
				<?php
				if (empty($vocGroup[$type])) {
					?>
					<tr>
						<td colspan="<?php echo ($loadOutfits ? 5 : 4); ?>">
							No players ranked in this category yet.
						</td>
					</tr>
					<?php
				} else {
					for ($i = 0; $i < count($vocGroup[$type]); $i++) {
						if (pageCheck($i, $page, $rowsPerPage)) {
							$row = $vocGroup[$type][$i];
							$flag = ($loadFlags && !empty($row['flag'])) ? '<img src="' . $config['country_flags']['server'] . '/' . $row['flag'] . '.png" alt="' . $row['flag'] . '"> ' : '';
							$vocName = is_numeric($row['vocation']) ? vocation_id_to_name($row['vocation']) : $row['vocation'];
							?>
							<tr>
								<?php if ($loadOutfits): ?>
									<td class="outfitColumn" style="text-align: center;">
										<img src="<?php echo $config['show_outfits']['imageServer']; ?>?id=<?php echo $row['type']; ?>&head=<?php echo $row['head']; ?>&body=<?php echo $row['body']; ?>&legs=<?php echo $row['legs']; ?>&feet=<?php echo $row['feet']; ?>" alt="outfit">
									</td>
								<?php endif; ?>
								<td><strong>#<?php echo $i + 1; ?></strong></td>
								<td><?php echo $flag; ?><a href="characterprofile.php?name=<?php echo urlencode($row['name']); ?>"><strong><?php echo htmlspecialchars($row['name']); ?></strong></a></td>
								<td><?php echo htmlspecialchars($vocName); ?></td>
								<td><?php echo $row['value']; ?></td>
							</tr>
							<?php
						}
					}
				}
				?>
			</tbody>
		</table>
	</div>
	<?php
}

include 'layout/overall/footer.php';
?>
