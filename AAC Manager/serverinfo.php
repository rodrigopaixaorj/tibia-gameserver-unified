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
	// Fallback padrão
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

function boolToPt($val) {
	$val = strtolower((string)$val);
	return in_array($val, array('1', 'true', 'yes', 'on')) ? '<span style="color:green; font-weight:bold;">Sim</span>' : '<span style="color:red; font-weight:bold;">Não</span>';
}

?>

<h1>Informações do Servidor</h1>
<p>Confira abaixo as principais taxas, configurações e mecânicas ativas em nosso servidor 7.72 clássico.</p>

<h2>Geral</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 35%;"><strong>Nome do Servidor:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['server_name']) ? $serverCfg['server_name'] : 'CipSoft 7.72'); ?></td>
			</tr>
			<tr>
				<td><strong>Mundo (World):</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['world_name']) ? $serverCfg['world_name'] : 'Zanera'); ?></td>
			</tr>
			<tr>
				<td><strong>Versão do Cliente:</strong></td>
				<td>7.72 Clássico</td>
			</tr>
			<tr>
				<td><strong>Porta de Conexão:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['login_port']) ? $serverCfg['login_port'] : '7171'); ?></td>
			</tr>
			<tr>
				<td><strong>Limite de Jogadores:</strong></td>
				<td><?php echo htmlspecialchars(isset($serverCfg['max_players']) ? $serverCfg['max_players'] : '1000'); ?> jogadores</td>
			</tr>
		</tbody>
	</table>
</div>

<h2>Taxas do Servidor (Rates)</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 35%;"><strong>Experiência (Exp Rate):</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_exp']) ? $serverCfg['rate_exp'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Habilidades (Skills Rate):</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_skill']) ? $serverCfg['rate_skill'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Magic Level Rate:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_magic']) ? $serverCfg['rate_magic'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Saque (Loot Rate):</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_loot']) ? $serverCfg['rate_loot'] : '1'; ?>x</strong></td>
			</tr>
			<tr>
				<td><strong>Respawn de Criaturas:</strong></td>
				<td><strong><?php echo isset($serverCfg['rate_spawn']) ? $serverCfg['rate_spawn'] : '1'; ?>x</strong></td>
			</tr>
		</tbody>
	</table>
</div>

<h2>Jogabilidade e Mecânicas</h2>
<div class="table-responsive">
	<table class="table table-striped table-hover">
		<tbody>
			<tr>
				<td style="width: 50%;"><strong>Aprender magias em NPCs (Learn Spells):</strong></td>
				<td><?php echo boolToPt(isset($serverCfg['learn_spells']) ? $serverCfg['learn_spells'] : false); ?></td>
			</tr>
			<tr>
				<td><strong>Cargas de runas consumidas ao usar:</strong></td>
				<td><?php echo boolToPt(isset($serverCfg['remove_charges_from_runes']) ? $serverCfg['remove_charges_from_runes'] : true); ?></td>
			</tr>
			<tr>
				<td><strong>Esvaziar frascos de fluidos (Life/Mana Fluid):</strong></td>
				<td><?php echo boolToPt(isset($serverCfg['remove_charges_from_vials']) ? $serverCfg['remove_charges_from_vials'] : true); ?></td>
			</tr>
			<tr>
				<td><strong>Consumir munição e armas de arremesso:</strong></td>
				<td><?php echo boolToPt(isset($serverCfg['remove_weapon_ammunition']) ? $serverCfg['remove_weapon_ammunition'] : true); ?></td>
			</tr>
		</tbody>
	</table>
</div>

<?php include 'layout/overall/footer.php'; ?>
