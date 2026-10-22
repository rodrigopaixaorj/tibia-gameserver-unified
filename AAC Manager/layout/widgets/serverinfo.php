<div class="well widget">
	<div class="header">
		Server Information
	</div>
	<div class="body">
		<ul>
			<?php
			$status = true;
			if ($config['status']['status_check']) {
				@$sock = fsockopen($config['status']['status_ip'], $config['status']['status_port'], $errno, $errstr, 1);
				if (!$sock) {
					echo "<span style='color:red;font-weight:bold;'><center>Server Offline!</center></span><br/>";
					$status = false;
				} else {
					fclose($sock);
					echo "<span style='color:green;font-weight:bold;'><center>Server Online!</center></span><br />";
				}
			}
			if ($status) {
				?>
				<li><a href="onlinelist.php"><strong>Players online:</strong> <?php echo user_count_online(); ?></a></li>
				<?php
			}
			?>
			<li><strong>Accounts registered:</strong> <?php echo user_count_accounts(); ?></li>
			<li><strong>Client:</strong> Tibia 7.72 Classic</li>
			<li><strong>Port:</strong> <?php echo $config['port']; ?></li>
		</ul>
	</div>
</div>
