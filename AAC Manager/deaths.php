<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

$cache = new Cache('engine/cache/deaths');
$cache->setExpiration(30);

if ($cache->hasExpired()) {
	$deaths = fetchLatestDeaths(0, 30);
	$cache->setContent($deaths);
	$cache->save();
} else {
	$deaths = $cache->load();
}
?>

<h1>Últimas Mortes</h1>

<?php if (!empty($deaths)): ?>
	<div class="table-responsive">
		<table id="deathsTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Vítima</th>
					<th>Data / Hora</th>
					<th>Causa / Assassino</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($deaths as $death): ?>
					<tr>
						<td>
							No level <?php echo $death['level']; ?>: 
							<a href="characterprofile.php?name=<?php echo urlencode($death['victim']); ?>">
								<strong><?php echo htmlspecialchars($death['victim']); ?></strong>
							</a>
						</td>
						<td><?php echo getClock($death['time'], true); ?></td>
						<td>
							<?php
							if ($death['is_player'] == 1) {
								echo 'Jogador: <a href="characterprofile.php?name=' . urlencode($death['killed_by']) . '"><strong>' . htmlspecialchars($death['killed_by']) . '</strong></a>';
							} else {
								echo 'Monstro/Ambiente: ' . htmlspecialchars($death['killed_by']);
							}
							?>
						</td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
<?php else: ?>
	<div class="alert alert-info">Nenhuma morte registrada ainda no servidor.</div>
<?php endif; ?>

<?php include 'layout/overall/footer.php'; ?>
