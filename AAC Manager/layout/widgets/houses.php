<div class="well widget">
	<div class="header">
		Casas por Cidade
	</div>
	<div class="body">
		<form action="houses.php" method="get">
			<select name="town" class="form-control" style="margin-bottom: 8px;">
				<?php
				foreach ($config['towns'] as $id => $name) {
					echo '<option value="' . $id . '">' . htmlspecialchars($name) . '</option>';
				}
				?>
			</select>
			<input type="submit" value="Ver Casas" class="btn btn-primary btn-block">
		</form>
	</div>
</div>
