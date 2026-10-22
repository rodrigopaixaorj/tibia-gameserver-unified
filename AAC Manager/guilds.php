<?php
require_once 'engine/init.php';
if ($config['require_login']['guilds']) protect_page();

include 'layout/overall/header.php';

function cipsoft_guild_list() {
	$cache = new Cache('engine/cache/guildlist');
	$cache->setExpiration(60);
	if ($cache->hasExpired()) {
		$guilds = mysql_select_multi("SELECT `g`.`GuildID` AS `id`, `g`.`Name` AS `name`, `g`.`Founded` AS `creationdata`, `g`.`Description` AS `motd`, 
			(SELECT COUNT(`CharacterID`) FROM `GuildMembers` WHERE `GuildID` = `g`.`GuildID`) AS `total` 
			FROM `Guilds` AS `g` 
			ORDER BY `g`.`Name` ASC;");

		if ($guilds !== false) {
			for ($i = 0; $i < count($guilds); $i++) {
				$guilds[$i]['level'] = get_guild_level_data($guilds[$i]['id']);
			}
		}
		$cache->setContent($guilds);
		$cache->save();
	} else {
		$guilds = $cache->load();
	}
	return $guilds;
}

$user_chars = array();
$user_char_ids = array();
if (user_logged_in()) {
	$char_array = user_character_list($session_user_id);
	if ($char_array) {
		foreach ($char_array as $c) {
			$user_chars[] = $c['name'];
			$user_char_ids[] = (int)$c['id'];
		}
	}
}

// Visualização de uma Guilda específica
if (!empty($_GET['name'])) {
	$guild_name = sanitize($_GET['name']);
	$guild = get_guild_data($guild_name);

	if (!$guild) {
		echo '<div class="alert alert-danger">Guilda não encontrada.</div>';
		echo '<p><a href="guilds.php" class="btn btn-primary">Voltar para a lista de guildas</a></p>';
		include 'layout/overall/footer.php';
		exit();
	}

	$gid = (int)$guild['id'];
	$players = get_guild_players($gid);
	$ranks = get_guild_rank_data($gid);
	$inv_data = guild_invite_list($gid);

	// Determinar o maior nível de acesso do visitante logado nesta guilda (3=Leader, 2=Vice-Leader, 1=Member, 0=None)
	$highest_access = 0;
	if (user_logged_in() && $players) {
		foreach ($players as $p) {
			if (in_array((int)$p['id'], $user_char_ids)) {
				$rank_lvl = (int)$p['rank_level'];
				if ($rank_lvl > $highest_access) {
					$highest_access = $rank_lvl;
				}
			}
		}
	}

	// Processamento POST
	if (user_logged_in() && !empty($_POST)) {
		// Aceitar convite
		if (!empty($_POST['joinguild'])) {
			$join_id = (int)$_POST['joinguild'];
			if (in_array($join_id, $user_char_ids)) {
				if (!user_is_online($join_id)) {
					if (get_character_guild_rank($join_id) === false) {
						guild_player_join($join_id, $gid);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">Este personagem já pertence a uma guilda.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">O personagem precisa estar offline no jogo para entrar na guilda.</div>';
				}
			}
		}

		// Rejeitar convite
		if (!empty($_POST['reject_invite'])) {
			$uninv_id = (int)$_POST['reject_invite'];
			if (in_array($uninv_id, $user_char_ids) || $highest_access >= 2) {
				guild_remove_invitation($uninv_id, $gid);
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}
		}

		// Sair da guilda
		if (!empty($_POST['leave_guild'])) {
			$leave_id = (int)$_POST['leave_guild'];
			if (in_array($leave_id, $user_char_ids)) {
				if (!user_is_online($leave_id)) {
					$p_rank = get_character_guild_rank($leave_id);
					if ($p_rank != 3) { // Líder não pode sair sem passar a liderança ou disbandar
						guild_player_leave($leave_id);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">O líder não pode sair da guilda sem transferir a liderança ou desfazê-la.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">O personagem precisa estar offline no jogo para sair da guilda.</div>';
				}
			}
		}

		// Ações de Vice-Líder e Líder (Acesso >= 2)
		if ($highest_access >= 2) {
			// Convidar jogador
			if (!empty($_POST['invite_player'])) {
				$inv_name = sanitize($_POST['invite_player']);
				$inv_cid = user_character_id($inv_name);
				if ($inv_cid !== false) {
					if (get_character_guild_rank($inv_cid) === false) {
						guild_invite_player($inv_cid, $gid, $session_user_id);
						header('Location: guilds.php?name=' . urlencode($guild_name));
						exit();
					} else {
						echo '<div class="alert alert-danger">Esse jogador já faz parte de uma guilda.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Personagem não encontrado.</div>';
				}
			}

			// Alterar MOTD (Mensagem da guilda)
			if (isset($_POST['guild_motd'])) {
				$new_motd = sanitize($_POST['guild_motd']);
				mysql_update("UPDATE `Guilds` SET `Description`='$new_motd' WHERE `GuildID`='$gid' LIMIT 1;");
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}

			// Alterar Nick/Título de membro
			if (!empty($_POST['member_nick_char'])) {
				$nick_cid = user_character_id($_POST['member_nick_char']);
				$new_nick = sanitize($_POST['member_nick']);
				update_player_guildnick($nick_cid, $new_nick);
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}

			// Promover / Rebaixar membro
			if (!empty($_POST['promote_char']) && !empty($_POST['promote_rank'])) {
				$prom_cid = user_character_id($_POST['promote_char']);
				$prom_rid = (int)$_POST['promote_rank'];
				if ($prom_rid < 3 || $highest_access == 3) {
					update_player_guild_position($prom_cid, $prom_rid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				}
			}

			// Remover / Expulsar membro
			if (!empty($_POST['kick_member'])) {
				$kick_cid = user_character_id($_POST['kick_member']);
				$kick_rank = get_character_guild_rank($kick_cid);
				if ($kick_rank < $highest_access) {
					guild_remove_member($kick_cid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				}
			}
		}

		// Ações exclusivas do Líder (Acesso == 3)
		if ($highest_access == 3) {
			// Alterar nomes dos cargos/ranks
			if (isset($_POST['change_rank_titles'])) {
				foreach ($ranks as $r) {
					$fName = 'rank_name_' . $r['id'];
					if (!empty($_POST[$fName])) {
						guild_change_rank($gid, $r['id'], $_POST[$fName]);
					}
				}
				header('Location: guilds.php?name=' . urlencode($guild_name));
				exit();
			}

			// Transferir liderança
			if (!empty($_POST['new_guild_leader'])) {
				$new_lid = (int)$_POST['new_guild_leader'];
				$old_lid = guild_leader($gid);
				if (!user_is_online($new_lid) && !user_is_online($old_lid)) {
					guild_change_leader($new_lid, $old_lid);
					header('Location: guilds.php?name=' . urlencode($guild_name));
					exit();
				} else {
					echo '<div class="alert alert-danger">Ambos os jogadores precisam estar offline para transferir a liderança.</div>';
				}
			}

			// Desfazer guilda
			if (isset($_POST['disband_guild'])) {
				guild_delete($gid);
				header('Location: guilds.php');
				exit();
			}
		}
	}
	?>

	<h1>Guilda: <?php echo htmlspecialchars($guild['name']); ?></h1>
	<?php if (!empty($guild['motd'])): ?>
		<p class="guild-motd" style="font-style: italic; background: rgba(0,0,0,0.04); padding: 10px; border-radius: 4px;">
			<?php echo nl2br(htmlspecialchars($guild['motd'])); ?>
		</p>
	<?php endif; ?>

	<h2>Membros da Guilda (<?php echo count($players); ?>)</h2>
	<div class="table-responsive">
		<table class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Cargo (Rank)</th>
					<th>Nome</th>
					<th>Level</th>
					<th>Vocação</th>
					<th>Status</th>
				</tr>
			</thead>
			<tbody>
				<?php
				if ($players) {
					foreach ($players as $p) {
						$online_status = ($p['online'] == 1) ? '<span style="color:green; font-weight:bold;">Online</span>' : '<span style="color:gray;">Offline</span>';
						$nick = !empty($p['guildnick']) ? ' (' . htmlspecialchars($p['guildnick']) . ')' : '';
						?>
						<tr>
							<td><strong><?php echo htmlspecialchars($p['rank_name']); ?></strong></td>
							<td><a href="characterprofile.php?name=<?php echo urlencode($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?></a><?php echo $nick; ?></td>
							<td><?php echo $p['level']; ?></td>
							<td><?php echo htmlspecialchars($p['vocation']); ?></td>
							<td><?php echo $online_status; ?></td>
						</tr>
						<?php
					}
				}
				?>
			</tbody>
		</table>
	</div>

	<?php if (!empty($inv_data)): ?>
		<h3>Personagens Convidados</h3>
		<div class="table-responsive">
			<table class="table table-bordered">
				<thead>
					<tr>
						<th>Nome</th>
						<th>Ações</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($inv_data as $inv): ?>
						<tr>
							<td><?php echo htmlspecialchars($inv['name']); ?></td>
							<td>
								<?php if (in_array((int)$inv['player_id'], $user_char_ids)): ?>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="joinguild" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Aceitar Convite" class="btn btn-success btn-sm">
									</form>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="reject_invite" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Recusar" class="btn btn-secondary btn-sm">
									</form>
								<?php elseif ($highest_access >= 2): ?>
									<form action="" method="post" style="display:inline;">
										<input type="hidden" name="reject_invite" value="<?php echo $inv['player_id']; ?>">
										<input type="submit" value="Cancelar Convite" class="btn btn-danger btn-sm">
									</form>
								<?php else: ?>
									Aguardando resposta do jogador
								<?php endif; ?>
							</td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		</div>
	<?php endif; ?>

	<?php
	// Opção para membro sair da guilda
	if (user_logged_in() && $players) {
		$my_guild_chars = array();
		foreach ($players as $p) {
			if (in_array((int)$p['id'], $user_char_ids) && $p['rank_level'] != 3) {
				$my_guild_chars[] = $p;
			}
		}
		if (!empty($my_guild_chars)) {
			?>
			<h3>Sair da Guilda</h3>
			<form action="" method="post" class="form-inline">
				<select name="leave_guild" class="form-control" required>
					<?php foreach ($my_guild_chars as $mgc): ?>
						<option value="<?php echo $mgc['id']; ?>"><?php echo htmlspecialchars($mgc['name']); ?></option>
					<?php endforeach; ?>
				</select>
				<input type="submit" value="Sair da Guilda" class="btn btn-warning" onclick="return confirm('Deseja realmente sair da guilda?');">
			</form>
			<?php
		}
	}
	?>

	<?php if ($highest_access >= 2): ?>
		<hr>
		<h2>Painel de Gerenciamento da Guilda</h2>

		<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
			<div style="flex: 1; min-width: 280px;">
				<h3>Convidar Jogador</h3>
				<form action="" method="post">
					<input type="text" name="invite_player" placeholder="Nome do personagem" class="form-control" required style="margin-bottom: 8px;">
					<input type="submit" value="Enviar Convite" class="btn btn-primary">
				</form>

				<h3 style="margin-top: 15px;">Atualizar Mensagem (MOTD)</h3>
				<form action="" method="post">
					<textarea name="guild_motd" class="form-control" rows="3" style="margin-bottom: 8px;"><?php echo htmlspecialchars($guild['motd']); ?></textarea>
					<input type="submit" value="Salvar Mensagem" class="btn btn-secondary">
				</form>
			</div>

			<div style="flex: 1; min-width: 280px;">
				<h3>Alterar Título / Nick</h3>
				<form action="" method="post">
					<select name="member_nick_char" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($players as $p): ?>
							<option value="<?php echo htmlspecialchars($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?></option>
						<?php endforeach; ?>
					</select>
					<input type="text" name="member_nick" placeholder="Novo título (ou deixe vazio para remover)" class="form-control" style="margin-bottom: 8px;">
					<input type="submit" value="Atualizar Título" class="btn btn-secondary">
				</form>

				<h3 style="margin-top: 15px;">Promover / Rebaixar Membro</h3>
				<form action="" method="post">
					<select name="promote_char" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($players as $p): if ($p['rank_level'] != 3): ?>
							<option value="<?php echo htmlspecialchars($p['name']); ?>"><?php echo htmlspecialchars($p['name']); ?> (<?php echo htmlspecialchars($p['rank_name']); ?>)</option>
						<?php endif; endforeach; ?>
					</select>
					<select name="promote_rank" class="form-control" required style="margin-bottom: 8px;">
						<?php foreach ($ranks as $r): if ($r['level'] != 3 || $highest_access == 3): ?>
							<option value="<?php echo $r['id']; ?>"><?php echo htmlspecialchars($r['name']); ?></option>
						<?php endif; endforeach; ?>
					</select>
					<input type="submit" value="Alterar Cargo" class="btn btn-secondary">
				</form>
			</div>
		</div>

		<?php if ($highest_access == 3): ?>
			<hr>
			<h3>Ações Exclusivas do Líder</h3>
			<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
				<div style="flex: 1; min-width: 280px;">
					<h4>Transferir Liderança</h4>
					<form action="" method="post">
						<select name="new_guild_leader" class="form-control" required style="margin-bottom: 8px;">
							<?php foreach ($players as $p): if ($p['rank_level'] != 3): ?>
								<option value="<?php echo $p['id']; ?>"><?php echo htmlspecialchars($p['name']); ?></option>
							<?php endif; endforeach; ?>
						</select>
						<input type="submit" value="Passar Liderança" class="btn btn-warning" onclick="return confirm('Tem certeza que deseja transferir a liderança da guilda?');">
					</form>
				</div>
				<div style="flex: 1; min-width: 280px;">
					<h4>Desfazer Guilda (Disband)</h4>
					<form action="" method="post">
						<input type="hidden" name="disband_guild" value="1">
						<input type="submit" value="Desfazer Guilda Permanentemente" class="btn btn-danger" onclick="return confirm('ATENÇÃO: Desfazer a guilda é uma ação irreversível. Confirmar?');">
					</form>
				</div>
			</div>
		<?php endif; ?>
	<?php endif; ?>

	<p style="margin-top: 25px;"><a href="guilds.php">&laquo; Voltar para a lista de todas as guildas</a></p>

	<?php
} else {
	// LISTAGEM GERAL DE GUILDAS
	$guilds = cipsoft_guild_list();
	?>
	<h1>Guildas</h1>

	<?php if (!empty($guilds)): ?>
		<div class="table-responsive">
			<table id="guildsTable" class="table table-striped table-hover">
				<thead>
					<tr class="yellow">
						<th>Nome da Guilda</th>
						<th>Descrição</th>
						<th>Membros</th>
						<th>Level Médio</th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($guilds as $g): ?>
						<tr onclick="window.location.href='guilds.php?name=<?php echo urlencode($g['name']); ?>';" style="cursor: pointer;">
							<td><a href="guilds.php?name=<?php echo urlencode($g['name']); ?>"><strong><?php echo htmlspecialchars($g['name']); ?></strong></a></td>
							<td><?php echo htmlspecialchars($g['motd']); ?></td>
							<td><?php echo isset($g['level']['players']) ? $g['level']['players'] : 0; ?></td>
							<td><?php echo isset($g['level']['avg']) ? $g['level']['avg'] : 0; ?></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		</div>
	<?php else: ?>
		<div class="alert alert-info">Nenhuma guilda foi criada ainda no servidor.</div>
	<?php endif; ?>

	<?php if (user_logged_in()): ?>
		<hr>
		<h2>Fundar uma Nova Guilda</h2>
		<?php
		if (!empty($_POST['selected_char']) && !empty($_POST['guild_name'])) {
			$founder_name = sanitize($_POST['selected_char']);
			$guild_create_name = sanitize($_POST['guild_name']);
			$founder_id = user_character_id($founder_name);

			if (in_array($founder_id, $user_char_ids)) {
				$char_info = user_character_data($founder_id, 'level');
				if ($char_info['level'] >= $config['create_guild_level']) {
					if (!user_is_online($founder_id)) {
						if (get_character_guild_rank($founder_id) === false) {
							if (preg_match("/^[a-zA-Z ]+$/", $guild_create_name) && strlen($guild_create_name) <= 30) {
								if (get_guild_id($guild_create_name) === false) {
									create_guild($founder_id, $guild_create_name);
									echo '<div class="alert alert-success">Guilda <strong>' . htmlspecialchars($guild_create_name) . '</strong> fundada com sucesso! <a href="guilds.php?name=' . urlencode($guild_create_name) . '">Clique aqui para gerenciá-la</a>.</div>';
								} else {
									echo '<div class="alert alert-danger">Já existe uma guilda com este nome.</div>';
								}
							} else {
								echo '<div class="alert alert-danger">O nome da guilda deve conter apenas letras e espaços (máximo 30 caracteres).</div>';
							}
						} else {
							echo '<div class="alert alert-danger">Este personagem já pertence a uma guilda.</div>';
						}
					} else {
						echo '<div class="alert alert-danger">O personagem fundador deve estar offline no jogo.</div>';
					}
				} else {
					echo '<div class="alert alert-danger">Seu personagem precisa ter level ' . $config['create_guild_level'] . ' ou superior para fundar uma guilda.</div>';
				}
			}
		}
		?>
		<form action="guilds.php" method="post" class="form-inline">
			<ul class="form-list">
				<li>
					<label for="selected_char">Personagem Fundador:</label>
					<select name="selected_char" id="selected_char" required>
						<?php foreach ($user_chars as $uc): ?>
							<option value="<?php echo htmlspecialchars($uc); ?>"><?php echo htmlspecialchars($uc); ?></option>
						<?php endforeach; ?>
					</select>
				</li>
				<li>
					<label for="guild_name">Nome da Guilda:</label>
					<input type="text" name="guild_name" id="guild_name" placeholder="Ex: Knights of Honor" required maxlength="30">
				</li>
				<li>
					<input type="submit" value="Fundar Guilda" class="btn btn-primary">
				</li>
			</ul>
		</form>
	<?php else: ?>
		<p><a href="login.php">Faça login</a> para fundar uma guilda ou gerenciar seus convites.</p>
	<?php endif; ?>

	<?php
}

include 'layout/overall/footer.php';
?>
