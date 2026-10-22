<div class="well widget">
	<div class="header">
		Top 5 Jogadores
	</div>
	<div class="body">
		<table>
			<?php
			$cache = new Cache('engine/cache/topPlayer');
			$cache->setExpiration(60);
			if ($cache->hasExpired()) {
				$players = mysql_select_multi("SELECT `Name` AS `name`, `Level` AS `level` FROM `Characters` WHERE `Deleted` = 0 ORDER BY `Level` DESC LIMIT 5;");
				$cache->setContent($players);
				$cache->save();
			} else {
				$players = $cache->load();
			}

			if (!empty($players)) {
				foreach ($players as $count => $player) {
					$nr = $count + 1;
					echo "<tr><td style='width: 25px;'><strong>{$nr}º</strong></td><td><a href='characterprofile.php?name=" . urlencode($player['name']) . "'><strong>" . htmlspecialchars($player['name']) . "</strong></a> (Level {$player['level']})</td></tr>";
				}
			} else {
				echo "<tr><td>Nenhum jogador classificado.</td></tr>";
			}
			?>
		</table>
	</div>
</div>
