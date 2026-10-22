<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if ($config['log_ip']) {
	znote_visitor_insert_detailed_data(3);
}

$house_id = isset($_GET['id']) ? (int)$_GET['id'] : 0;
$house = ($house_id > 0) ? fetchHouse($house_id) : false;

if ($house) {
	$townName = isset($config['towns'][$house['town_id']]) ? $config['towns'][$house['town_id']] : 'Unknown';
	?>
	<h1>House Details: <?php echo htmlspecialchars($house['name']); ?></h1>

	<div class="house-detail-card" style="margin-bottom: 20px;">
		<table class="table table-striped table-hover">
			<tbody>
				<tr>
					<td style="width: 200px;"><strong>House Name:</strong></td>
					<td><?php echo htmlspecialchars($house['name']); ?></td>
				</tr>
				<tr>
					<td><strong>Location (Town):</strong></td>
					<td><?php echo htmlspecialchars($townName); ?></td>
				</tr>
				<tr>
					<td><strong>Size:</strong></td>
					<td><?php echo $house['size']; ?> SQM</td>
				</tr>
				<tr>
					<td><strong>Monthly Rent:</strong></td>
					<td><?php echo number_format($house['rent'], 0, '', ','); ?> gp</td>
				</tr>
				<tr>
					<td><strong>Owner:</strong></td>
					<td>
						<?php
						if (!empty($house['owner_name'])) {
							echo '<a href="characterprofile.php?name=' . urlencode($house['owner_name']) . '"><strong>' . htmlspecialchars($house['owner_name']) . '</strong></a>';
						} else {
							echo '<span style="color: green; font-weight: bold;">Available for Rent / Purchase</span>';
						}
						?>
					</td>
				</tr>
				<?php if (!empty($house['paid'])): ?>
					<tr>
						<td><strong>Rent Paid Until:</strong></td>
						<td><?php echo date('M d Y, H:i', $house['paid']); ?></td>
					</tr>
				<?php endif; ?>
			</tbody>
		</table>
	</div>

	<p><a href="houses.php?town=<?php echo $house['town_id']; ?>" class="btn btn-secondary">&laquo; Back to <?php echo htmlspecialchars($townName); ?> houses</a></p>
	<?php
} else {
	echo '<div class="alert alert-danger">House not found or invalid identifier.</div>';
	echo '<p><a href="houses.php" class="btn btn-primary">Back to house list</a></p>';
}

include 'layout/overall/footer.php';
?>
