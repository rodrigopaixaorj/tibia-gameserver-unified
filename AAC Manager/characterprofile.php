<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if ($config['log_ip']) {
	znote_visitor_insert_detailed_data(4);
}

if (!empty($_GET['name'])) {
	$name = sanitize($_GET['name']);
	$user_id = user_character_exist($name);

	if ($user_id !== false) {
		$char = mysql_select_single("SELECT `c`.`CharacterID`, `c`.`AccountID`, `c`.`Name`, `c`.`Sex`, `c`.`Level`, `c`.`Profession`, `c`.`Residence`, `c`.`LastLoginTime`, `c`.`IsOnline`,
			`ps`.`OrigLookType`, `ps`.`OrigLookHead`, `ps`.`OrigLookBody`, `ps`.`OrigLookLegs`, `ps`.`OrigLookFeet`
			FROM `Characters` AS `c`
			LEFT JOIN `PlayerState` AS `ps` ON `c`.`CharacterID` = `ps`.`CharacterID`
			WHERE `c`.`CharacterID` = '$user_id' AND `c`.`Deleted` = 0 LIMIT 1;");

		if ($char === false) {
			echo '<div class="alert alert-warning">Personagem não encontrado.</div>';
			include 'layout/overall/footer.php';
			exit();
		}

		$znote_char = user_znote_character_data($user_id, 'created', 'hide_char', 'comment');
		$account_flag = user_znote_account_data($char['AccountID'], 'flag');

		// Dados de Guild
		$guild_data = mysql_select_single("SELECT `g`.`GuildID`, `g`.`Name` AS `guild_name`, `gr`.`RankName` AS `rank_name` 
			FROM `GuildMembers` AS `gm`
			INNER JOIN `Guilds` AS `g` ON `gm`.`GuildID` = `g`.`GuildID`
			LEFT JOIN `GuildRanks` AS `gr` ON `gm`.`GuildID` = `gr`.`GuildID` AND `gm`.`RankID` = `gr`.`RankID`
			WHERE `gm`.`CharacterID` = '$user_id' LIMIT 1;");

		// Dados de Casa
		$houses = mysql_select_multi("SELECT `h`.`Name`, `h`.`TownID` 
			FROM `Houses` AS `h`
			INNER JOIN `HouseOwners` AS `ho` ON `h`.`HouseID` = `ho`.`HouseID`
			WHERE `ho`.`OwnerID` = '$user_id';");

		// Deletion flag
		$deletion_time = mysql_select_single("SELECT `time` FROM `znote_deleted_characters` WHERE `character_name`='$name' AND `done` = '0' LIMIT 1;");

		// Skills do jogador
		$skills_raw = mysql_select_multi("SELECT `SkillID`, `Actual`, `Maximum`, `Experience` FROM `PlayerSkills` WHERE `CharacterID` = '$user_id';");
		$skills = array();
		if ($skills_raw) {
			foreach ($skills_raw as $s) {
				$skills[(int)$s['SkillID']] = $s;
			}
		}

		$exp = isset($skills[0]) ? number_format($skills[0]['Experience'], 0, '', ',') : 0;
		$hp = isset($skills[2]) ? $skills[2]['Actual'] : 0;
		$hpMax = isset($skills[2]) ? $skills[2]['Maximum'] : 0;
		$mana = isset($skills[3]) ? $skills[3]['Actual'] : 0;
		$manaMax = isset($skills[3]) ? $skills[3]['Maximum'] : 0;
		$cap = isset($skills[5]) ? $skills[5]['Actual'] : 0;
		$magic = isset($skills[1]) ? $skills[1]['Actual'] : 0;
		$shield = isset($skills[6]) ? $skills[6]['Actual'] : 0;
		$dist = isset($skills[7]) ? $skills[7]['Actual'] : 0;
		$sword = isset($skills[8]) ? $skills[8]['Actual'] : 0;
		$club = isset($skills[9]) ? $skills[9]['Actual'] : 0;
		$axe = isset($skills[10]) ? $skills[10]['Actual'] : 0;
		$fist = isset($skills[11]) ? $skills[11]['Actual'] : 0;
		$fish = isset($skills[13]) ? $skills[13]['Actual'] : 0;
		?>

		<div class="profile-container">
			<table id="characterProfileTable" class="table table-striped table-hover">
				<thead>
					<tr class="yellow">
						<th style="width: 80px; text-align: center;">
							<?php if ($config['show_outfits']['characterprofile'] && isset($char['OrigLookType'])): ?>
								<div class="outfit">
									<img src="<?php echo $config['show_outfits']['imageServer']; ?>?id=<?php echo $char['OrigLookType']; ?>&head=<?php echo $char['OrigLookHead']; ?>&body=<?php echo $char['OrigLookBody']; ?>&legs=<?php echo $char['OrigLookLegs']; ?>&feet=<?php echo $char['OrigLookFeet']; ?>" alt="Outfit">
								</div>
							<?php endif; ?>
							<?php if ($config['country_flags']['enabled'] && !empty($account_flag['flag'])): ?>
								<div class="flag" style="margin-top: 4px;">
									<img src="<?php echo $config['country_flags']['server'] . '/' . $account_flag['flag']; ?>.png" alt="País">
								</div>
							<?php endif; ?>
						</th>
						<th>
							<h1 style="margin: 0; font-size: 24px;"><?php echo htmlspecialchars($char['Name']); ?></h1>
						</th>
					</tr>
				</thead>
				<tbody>
					<?php if ($deletion_time !== false): ?>
						<tr>
							<td colspan="2" style="color: red; font-weight: bold;">
								Marcado para exclusão pelo proprietário em: <?php echo $deletion_time['time']; ?>.
							</td>
						</tr>
					<?php endif; ?>
					<tr>
						<td><strong>Sexo:</strong></td>
						<td><?php echo ($char['Sex'] == 1) ? 'Masculino (Male)' : 'Feminino (Female)'; ?></td>
					</tr>
					<tr>
						<td><strong>Level:</strong></td>
						<td><?php echo $char['Level']; ?> (Experiência: <?php echo $exp; ?>)</td>
					</tr>
					<tr>
						<td><strong>Vocação:</strong></td>
						<td><?php echo htmlspecialchars($char['Profession']); ?></td>
					</tr>
					<tr>
						<td><strong>Residência:</strong></td>
						<td><?php echo htmlspecialchars($char['Residence']); ?></td>
					</tr>
					<?php if ($guild_data !== false): ?>
						<tr>
							<td><strong>Guilda:</strong></td>
							<td>
								<strong><?php echo htmlspecialchars($guild_data['rank_name']); ?></strong> da 
								<a href="guilds.php?name=<?php echo urlencode($guild_data['guild_name']); ?>">
									<?php echo htmlspecialchars($guild_data['guild_name']); ?>
								</a>
							</td>
						</tr>
					<?php endif; ?>
					<?php if ($houses !== false): foreach ($houses as $h): ?>
						<tr>
							<td><strong>Casa:</strong></td>
							<td><?php echo htmlspecialchars($h['Name']); ?> (<?php echo town_id_to_name($h['TownID']); ?>)</td>
						</tr>
					<?php endforeach; endif; ?>
					<tr>
						<td><strong>Último Acesso:</strong></td>
						<td><?php echo ($char['LastLoginTime'] > 0) ? getClock($char['LastLoginTime'], true, true) : 'Nunca acessou.'; ?></td>
					</tr>
					<tr>
						<td><strong>Status:</strong></td>
						<td>
							<?php if ($char['IsOnline'] == 1): ?>
								<span style="color: green; font-weight: bold;">Online</span>
							<?php else: ?>
								<span style="color: gray;">Offline</span>
							<?php endif; ?>
						</td>
					</tr>
					<?php if (!empty($znote_char['created'])): ?>
						<tr>
							<td><strong>Criado em:</strong></td>
							<td><?php echo getClock($znote_char['created'], true); ?></td>
						</tr>
					<?php endif; ?>
				</tbody>
			</table>

			<?php if (!empty($skills)): ?>
				<h3>Estatísticas e Habilidades</h3>
				<table class="table table-bordered">
					<tr>
						<td><strong>Hitpoints (HP):</strong> <?php echo $hp; ?> / <?php echo $hpMax; ?></td>
						<td><strong>Mana (MP):</strong> <?php echo $mana; ?> / <?php echo $manaMax; ?></td>
						<td><strong>Capacidade (Cap):</strong> <?php echo $cap; ?></td>
					</tr>
					<tr>
						<td><strong>Magic Level:</strong> <?php echo $magic; ?></td>
						<td><strong>Shielding:</strong> <?php echo $shield; ?></td>
						<td><strong>Distance:</strong> <?php echo $dist; ?></td>
					</tr>
					<tr>
						<td><strong>Sword Fighting:</strong> <?php echo $sword; ?></td>
						<td><strong>Axe Fighting:</strong> <?php echo $axe; ?></td>
						<td><strong>Club Fighting:</strong> <?php echo $club; ?></td>
					</tr>
					<tr>
						<td><strong>Fist Fighting:</strong> <?php echo $fist; ?></td>
						<td><strong>Fishing:</strong> <?php echo $fish; ?></td>
						<td></td>
					</tr>
				</table>
			<?php endif; ?>

			<?php if (!empty($znote_char['comment'])): ?>
				<h3>Comentário do Personagem</h3>
				<div class="character-comment" style="padding: 10px; background: rgba(0,0,0,0.05); border-left: 3px solid #b89b4b; border-radius: 4px; margin-bottom: 20px;">
					<?php echo nl2br(htmlspecialchars($znote_char['comment'])); ?>
				</div>
			<?php endif; ?>

			<h3>Histórico de Mortes</h3>
			<table class="table table-striped table-hover">
				<thead>
					<tr class="yellow">
						<th style="width: 180px;">Data / Hora</th>
						<th>Descrição da Morte</th>
					</tr>
				</thead>
				<tbody>
					<?php
					$deaths = user_fetch_deathlist($user_id);
					if ($deaths) {
						foreach ($deaths as $d) {
							$killer_link = ($d['is_player'] == 1) 
								? '<a href="characterprofile.php?name=' . urlencode($d['killed_by']) . '"><strong>' . htmlspecialchars($d['killed_by']) . '</strong></a>' 
								: htmlspecialchars($d['killed_by']);
							?>
							<tr>
								<td><?php echo getClock($d['time'], true, true); ?></td>
								<td>Morto no level <?php echo $d['level']; ?> por <?php echo $killer_link; ?>.</td>
							</tr>
							<?php
						}
					} else {
						?>
						<tr>
							<td colspan="2">Este personagem nunca morreu.</td>
						</tr>
						<?php
					}
					?>
				</tbody>
			</table>

			<?php
			// Outros personagens visíveis na mesma conta
			$otherChars = mysql_select_multi("SELECT `c`.`CharacterID`, `c`.`Name`, `c`.`Level`, `c`.`Profession`, `c`.`LastLoginTime`, `c`.`IsOnline` 
				FROM `Characters` AS `c`
				LEFT JOIN `znote_players` AS `zp` ON `c`.`CharacterID` = `zp`.`player_id`
				WHERE `c`.`AccountID` = '{$char['AccountID']}' AND `c`.`CharacterID` != '$user_id' AND `c`.`Deleted` = 0 AND (`zp`.`hide_char` IS NULL OR `zp`.`hide_char` = 0)
				ORDER BY `c`.`Level` DESC;");

			if ($otherChars):
				?>
				<h3>Outros Personagens na Conta</h3>
				<div class="table-responsive">
					<table class="table table-striped table-hover">
						<thead>
							<tr class="yellow">
								<th>Nome</th>
								<th>Level</th>
								<th>Vocação</th>
								<th>Último Acesso</th>
								<th>Status</th>
							</tr>
						</thead>
						<tbody>
							<?php foreach ($otherChars as $oc): ?>
								<tr>
									<td><a href="characterprofile.php?name=<?php echo urlencode($oc['Name']); ?>"><strong><?php echo htmlspecialchars($oc['Name']); ?></strong></a></td>
									<td><?php echo $oc['Level']; ?></td>
									<td><?php echo htmlspecialchars($oc['Profession']); ?></td>
									<td><?php echo ($oc['LastLoginTime'] > 0) ? getClock($oc['LastLoginTime'], true, true) : 'Nunca'; ?></td>
									<td><?php echo ($oc['IsOnline'] == 1) ? '<span style="color:green;">Online</span>' : '<span style="color:gray;">Offline</span>'; ?></td>
								</tr>
							<?php endforeach; ?>
						</tbody>
					</table>
				</div>
			<?php endif; ?>
		</div>
		<?php
	} else {
		echo '<div class="alert alert-danger">O personagem <strong>' . htmlspecialchars($name) . '</strong> não foi encontrado.</div>';
	}
} else {
	header('Location: index.php');
	exit();
}

include 'layout/overall/footer.php';
?>
