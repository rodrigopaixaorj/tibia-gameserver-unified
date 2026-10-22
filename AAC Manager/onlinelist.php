<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
?>

<h1>Quem está online?</h1>

<?php
$loadFlags = ($config['country_flags']['enabled'] && $config['country_flags']['onlinelist']);
$loadOutfits = ($config['show_outfits']['onlinelist']);

$cache = new Cache('engine/cache/onlinelist');
$cache->setExpiration(15);

if ($cache->hasExpired()) {
	$array = mysql_select_multi("SELECT `c`.`Name` AS `name`, `c`.`Level` AS `level`, `c`.`Profession` AS `vocation`, `g`.`Name` AS `gname`, `za`.`flag`,
		`ps`.`OrigLookType` AS `type`, `ps`.`OrigLookHead` AS `head`, `ps`.`OrigLookBody` AS `body`, `ps`.`OrigLookLegs` AS `legs`, `ps`.`OrigLookFeet` AS `feet`
		FROM `Characters` AS `c`
		LEFT JOIN `PlayerState` AS `ps` ON `c`.`CharacterID` = `ps`.`CharacterID`
		LEFT JOIN `GuildMembers` AS `gm` ON `c`.`CharacterID` = `gm`.`CharacterID`
		LEFT JOIN `Guilds` AS `g` ON `gm`.`GuildID` = `g`.`GuildID`
		LEFT JOIN `znote_accounts` AS `za` ON `c`.`AccountID` = `za`.`account_id`
		WHERE `c`.`IsOnline` = 1 AND `c`.`Deleted` = 0
		ORDER BY `c`.`Level` DESC;");

	$cache->setContent($array);
	$cache->save();
} else {
	$array = $cache->load();
}

if (!empty($array) && $array !== false) {
	?>
	<div class="table-responsive">
		<table id="onlinelistTable" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<?php if ($loadOutfits): ?><th>Outfit</th><?php endif; ?>
					<th>Nome</th>
					<th>Guilda</th>
					<th>Level</th>
					<th>Vocação</th>
				</tr>
			</thead>
			<tbody>
				<?php
				foreach ($array as $value) {
					$url = "characterprofile.php?name=" . urlencode($value['name']);
					$flag = ($loadFlags && !empty($value['flag'])) ? '<img src="' . $config['country_flags']['server'] . '/' . $value['flag'] . '.png" alt="' . $value['flag'] . '"> ' : '';
					$guildname = (!empty($value['gname'])) ? '<a href="guilds.php?name=' . urlencode($value['gname']) . '">' . htmlspecialchars($value['gname']) . '</a>' : '-';
					?>
					<tr>
						<?php if ($loadOutfits): ?>
							<td class="outfitColumn" style="width: 40px; text-align: center;">
								<img src="<?php echo $config['show_outfits']['imageServer']; ?>?id=<?php echo $value['type']; ?>&head=<?php echo $value['head']; ?>&body=<?php echo $value['body']; ?>&legs=<?php echo $value['legs']; ?>&feet=<?php echo $value['feet']; ?>" alt="outfit">
							</td>
						<?php endif; ?>
						<td><?php echo $flag; ?><a href="<?php echo $url; ?>"><strong><?php echo htmlspecialchars($value['name']); ?></strong></a></td>
						<td><?php echo $guildname; ?></td>
						<td><?php echo $value['level']; ?></td>
						<td><?php echo htmlspecialchars($value['vocation']); ?></td>
					</tr>
					<?php
				}
				?>
			</tbody>
		</table>
	</div>
	<p>Total de jogadores online: <strong><?php echo count($array); ?></strong></p>
	<?php
} else {
	echo '<div class="alert alert-info">Nenhum jogador está online no momento.</div>';
}

include 'layout/overall/footer.php';
?>
