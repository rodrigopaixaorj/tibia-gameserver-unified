<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
?>

<h1>Support & Staff Team</h1>

<?php
$cache = new Cache('engine/cache/support');
$cache->setExpiration(60);

if ($cache->hasExpired()) {
	$staffs = support_list();
	$groups = array(
		3 => 'Administrator / God',
		2 => 'Gamemaster / Support',
		1 => 'Tutor'
	);

	$srtGrp = array();
	if (!empty($staffs)) {
		foreach ($groups as $group_id => $group_name) {
			foreach ($staffs as $staff) {
				if ($staff['group_id'] == $group_id) {
					$srtGrp[$group_name][] = $staff;
				}
			}
		}
	}
	$cache->setContent($srtGrp);
	$cache->save();
} else {
	$srtGrp = $cache->load();
}

if (!empty($srtGrp)) {
	?>
	<div class="table-responsive">
		<table id="supportTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th style="width: 35%;">Position</th>
					<th style="width: 45%;">Name</th>
					<th style="width: 20%;">Status</th>
				</tr>
			</thead>
			<tbody>
				<?php
				foreach ($srtGrp as $grpName => $grpList) {
					foreach ($grpList as $char) {
						$online = ($char['online'] == 1) ? '<span style="color:green; font-weight:bold;">Online</span>' : '<span style="color:gray;">Offline</span>';
						?>
						<tr>
							<td><strong><?php echo htmlspecialchars($grpName); ?></strong></td>
							<td><a href="characterprofile.php?name=<?php echo urlencode($char['name']); ?>"><strong><?php echo htmlspecialchars($char['name']); ?></strong></a></td>
							<td><?php echo $online; ?></td>
						</tr>
						<?php
					}
				}
				?>
			</tbody>
		</table>
	</div>
	<?php
} else {
	echo '<div class="alert alert-info">No staff members listed at this time.</div>';
}

include 'layout/overall/footer.php';
?>