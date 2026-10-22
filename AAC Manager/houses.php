<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if ($config['log_ip']) {
	znote_visitor_insert_detailed_data(3);
}

$defaultTown = isset($config['houseConfig']['HouseListDefaultTown']) ? (int)$config['houseConfig']['HouseListDefaultTown'] : 1;
$selectedTown = isset($_GET['town']) ? (int)$_GET['town'] : (isset($_POST['selected']) ? (int)$_POST['selected'] : $defaultTown);

if (!isset($config['towns'][$selectedTown])) {
	$selectedTown = 1;
}

$cache = new Cache('engine/cache/houses_' . $selectedTown);
$cache->setExpiration(60);

if ($cache->hasExpired()) {
	$houses = fetchTownHouses($selectedTown);
	$cache->setContent($houses);
	$cache->save();
} else {
	$houses = $cache->load();
}
?>

<h1>Lista de Casas</h1>

<div class="well widget" style="margin-bottom: 20px; padding: 15px; background: rgba(0,0,0,0.03); border-radius: 4px;">
	<form action="houses.php" method="get" class="form-inline">
		<label for="town"><strong>Selecione a Cidade:</strong></label>
		<select name="town" id="town" class="form-control" style="margin: 0 10px;">
			<?php
			foreach ($config['towns'] as $id => $name) {
				$sel = ($id == $selectedTown) ? 'selected' : '';
				echo '<option value="' . $id . '" ' . $sel . '>' . htmlspecialchars($name) . '</option>';
			}
			?>
		</select>
		<input type="submit" value="Buscar Casas" class="btn btn-primary">
	</form>
</div>

<h2>Casas em <?php echo htmlspecialchars($config['towns'][$selectedTown]); ?></h2>

<?php if (!empty($houses)): ?>
	<div class="table-responsive">
		<table id="housesTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Nome da Casa</th>
					<th>Tamanho</th>
					<th>Aluguel</th>
					<th>Proprietário</th>
					<th>Status</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($houses as $h): ?>
					<tr>
						<td>
							<a href="house.php?id=<?php echo $h['id']; ?>">
								<strong><?php echo htmlspecialchars($h['name']); ?></strong>
							</a>
						</td>
						<td><?php echo $h['size']; ?> m² (SQM)</td>
						<td><?php echo number_format($h['rent'], 0, '', '.'); ?> gp</td>
						<td>
							<?php
							if (!empty($h['owner_name'])) {
								echo '<a href="characterprofile.php?name=' . urlencode($h['owner_name']) . '">' . htmlspecialchars($h['owner_name']) . '</a>';
							} else {
								echo '<span style="color: gray;">Nenhum</span>';
							}
							?>
						</td>
						<td>
							<?php
							if (!empty($h['owner'])) {
								echo '<span style="color: red; font-weight: bold;">Ocupada</span>';
							} else {
								echo '<span style="color: green; font-weight: bold;">Disponível</span>';
							}
							?>
						</td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
<?php else: ?>
	<div class="alert alert-info">Nenhuma casa encontrada cadastrada para a cidade de <?php echo htmlspecialchars($config['towns'][$selectedTown]); ?>.</div>
<?php endif; ?>

<?php include 'layout/overall/footer.php'; ?>
