<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

$cache_killers = new Cache('engine/cache/killers');
$cache_killers->setExpiration(60);
if ($cache_killers->hasExpired()) {
	$killers = fetchMurders();
	$cache_killers->setContent($killers);
	$cache_killers->save();
} else {
	$killers = $cache_killers->load();
}

$cache_victims = new Cache('engine/cache/victims');
$cache_victims->setExpiration(60);
if ($cache_victims->hasExpired()) {
	$victims = fetchLoosers();
	$cache_victims->setContent($victims);
	$cache_victims->save();
} else {
	$victims = $cache_victims->load();
}

$cache_last = new Cache('engine/cache/lastkillers');
$cache_last->setExpiration(30);
if ($cache_last->hasExpired()) {
	$latests = mysql_select_multi("SELECT `c`.`Name` AS `victim`, `k`.`Name` AS `killed_by`, `d`.`Timestamp` AS `time` 
		FROM `CharacterDeaths` AS `d`
		INNER JOIN `Characters` AS `c` ON `d`.`CharacterID` = `c`.`CharacterID`
		INNER JOIN `Characters` AS `k` ON `d`.`OffenderID` = `k`.`CharacterID`
		WHERE `d`.`OffenderID` > 0
		ORDER BY `d`.`Timestamp` DESC LIMIT 20;");
	$cache_last->setContent($latests);
	$cache_last->save();
} else {
	$latests = $cache_last->load();
}
?>

<h1>Estatísticas de Assassinatos (PvP)</h1>

<h2>Maiores Assassinos (Top Fraggers)</h2>
<?php if (!empty($killers)): ?>
	<div class="table-responsive">
		<table id="killersTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Jogador</th>
					<th>Quantidade de Mortes Provocadas</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($killers as $killer): ?>
					<tr>
						<td><a href="characterprofile.php?name=<?php echo urlencode($killer['killed_by']); ?>"><strong><?php echo htmlspecialchars($killer['killed_by']); ?></strong></a></td>
						<td><?php echo $killer['kills']; ?></td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
<?php else: ?>
	<p>Nenhum assassinato registrado ainda.</p>
<?php endif; ?>

<h2>Maiores Vítimas</h2>
<?php if (!empty($victims)): ?>
	<div class="table-responsive">
		<table id="victimsTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Jogador</th>
					<th>Total de Mortes Sofridas</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($victims as $victim): ?>
					<tr>
						<td><a href="characterprofile.php?name=<?php echo urlencode($victim['name']); ?>"><strong><?php echo htmlspecialchars($victim['name']); ?></strong></a></td>
						<td><?php echo $victim['Deaths']; ?></td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
<?php else: ?>
	<p>Nenhuma vítima registrada ainda.</p>
<?php endif; ?>

<h2>Últimos Assassinatos (Player vs Player)</h2>
<?php if (!empty($latests)): ?>
	<div class="table-responsive">
		<table id="latestKillersTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Assassino</th>
					<th>Data / Hora</th>
					<th>Vítima</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($latests as $last): ?>
					<tr>
						<td><a href="characterprofile.php?name=<?php echo urlencode($last['killed_by']); ?>"><strong><?php echo htmlspecialchars($last['killed_by']); ?></strong></a></td>
						<td><?php echo getClock($last['time'], true); ?></td>
						<td><a href="characterprofile.php?name=<?php echo urlencode($last['victim']); ?>"><strong><?php echo htmlspecialchars($last['victim']); ?></strong></a></td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
<?php else: ?>
	<p>Nenhum assassinato PvP recente registrado.</p>
<?php endif; ?>

<?php include 'layout/overall/footer.php'; ?>
