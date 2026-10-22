<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

function parse_cipsoft_cfg($filePath) {
	if (!file_exists($filePath)) return false;
	$lines = file($filePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
	$config = array();
	foreach ($lines as $line) {
		$line = trim($line);
		if (empty($line) || $line[0] === '#' || $line[0] === ';') continue;
		if (strpos($line, '=') !== false) {
			list($key, $val) = explode('=', $line, 2);
			$key = trim($key);
			$val = trim($val);
			$val = trim($val, '"\'');
			$config[$key] = $val;
		}
	}
	return $config;
}

$cfgPath = $config['server_path'] . '/config.cfg';
$serverCfg = parse_cipsoft_cfg($cfgPath);

if (!$serverCfg) {
	// Default fallback
	$serverCfg = array(
		'server_name' => 'CipSoft Tibia 7.72',
		'world_name' => 'Classic',
		'rate_exp' => '1',
		'rate_skill' => '1',
		'rate_magic' => '1',
		'rate_loot' => '1',
		'rate_spawn' => '1',
		'login_port' => '7171',
		'game_port' => '7172',
		'max_players' => '1000'
	);
}

function boolToText($val) {
	$val = strtolower((string)$val);
	return in_array($val, array('1', 'true', 'yes', 'on')) ? '<span style="color:green; font-weight:bold;">Yes</span>' : '<span style="color:red; font-weight:bold;">No</span>';
}

?>

<h1>Server Information</h1>
<p>Check below the main rates, settings, and mechanics active on our classic 7.72 server.</p>

<h2>General</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 35%;"><strong>Server Name:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['server_name']) ? $serverCfg['server_name'] : 'CipSoft 7.72'); ?></td>
			</tr>
			<tr>
				<td><strong>World:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['world_name']) ? $serverCfg['world_name'] : 'Zanera'); ?></td>
			</tr>
			<tr>
				<td><strong>Client Version:</strong></td>
				<td>7.72 Classic</td>
			</tr>
			<tr>
				<td><strong>Port:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['login_port']) ? $serverCfg['login_port'] : '7171'); ?></td>
			</tr>
			<tr>
				<td><strong>Player Limit:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['max_players']) ? $serverCfg['max_players'] : '1000'); ?> players</td>
			</tr>
		</tbody>
	</table>
</div>

<h2>Rates</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 35%;"><strong>Experience Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_exp']) ? $serverCfg['rate_exp'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Skills Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_skill']) ? $serverCfg['rate_skill'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Magic Level Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_magic']) ? $serverCfg['rate_magic'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Loot Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_loot']) ? $serverCfg['rate_loot'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Spawn Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_spawn']) ? $serverCfg['rate_spawn'] : '1'; ?>x</strong></td>
			</tr>
		</tbody>
	</table>
</div>

<h2>Gameplay & Mechanics</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 50%;"><strong>Learn spells from NPCs:</strong></td>
				<td><?php echo boolToText(isset($serverCfg['learn_spells']) ? $serverCfg['learn_spells'] : false); ?></td>
			</tr>
			<tr>
				<td><strong>Consume rune charges:</strong></td>
				<td><?php echo boolToText(isset($serverCfg['remove_charges_from_runes']) ? $serverCfg['remove_charges_from_runes'] : true); ?></td>
			</tr>
			<tr>
				<td><strong>Consume potion/fluid vials:</strong></td>
				<td><?php echo boolToText(isset($serverCfg['remove_charges_from_vials']) ? $serverCfg['remove_charges_from_vials'] : true); ?></td>
			</tr>
			<tr>
				<td><strong>Consume ammunition and throwables:</strong></td>
				<td><?php echo boolToText(isset($serverCfg['remove_weapon_ammunition']) ? $serverCfg['remove_weapon_ammunition'] : true); ?></td>
			</tr>
		</tbody>
	</table>
</div>

<?php include 'layout/overall/footer.php'; ?>
