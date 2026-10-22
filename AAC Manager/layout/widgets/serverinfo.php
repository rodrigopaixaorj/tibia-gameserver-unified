<div class="well widget">
	<div class="header">
		Informações do Servidor
	</div>
	<div class="body">
		<ul>
			<?php
			$status = true;
			if ($config['status']['status_check']) {
				@$sock = fsockopen($config['status']['status_ip'], $config['status']['status_port'], $errno, $errstr, 1);
				if (!$sock) {
					echo "<span style='color:red;font-weight:bold;'><center>Servidor Offline!</center></span><br/>";
					$status = false;
				} else {
					fclose($sock);
					echo "<span style='color:green;font-weight:bold;'><center>Servidor Online!</center></span><br />";
				}
			}
			if ($status) {
				?>
				<li><a href="onlinelist.php"><strong>Jogadores online:</strong> <?php echo user_count_online(); ?></a></li>
				<?php
			}
			?>
			<li><strong>Contas registradas:</strong> <?php echo user_count_accounts(); ?></li>
			<li><strong>Versão:</strong> Tibia 7.72 Clássico</li>
			<li><strong>Porta:</strong> <?php echo $config['port']; ?></li>
		</ul>
	</div>
</div>
