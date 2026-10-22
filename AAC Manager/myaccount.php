<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';

#region CANCEL CHARACTER DELETE
$undelete_id = isset($_GET['cancel_delete_id']) ? (int)$_GET['cancel_delete_id'] : 0;
if ($undelete_id > 0) {
	$undelete_q1 = mysql_select_single('SELECT `character_name` FROM `znote_deleted_characters` WHERE `done` = 0 AND `id` = ' . $undelete_id . ' AND `original_account_id` = ' . $session_user_id . ' AND NOW() < `time`');
	if ($undelete_q1) {
		mysql_delete('DELETE FROM `znote_deleted_characters` WHERE `id` = ' . $undelete_id);
		echo '<p class="success">A exclusão do personagem <strong>' . htmlspecialchars($undelete_q1['character_name']) . '</strong> foi cancelada com sucesso.</p>';
	}
}
#endregion

$render_page = true;

// Handle POST
if (!empty($_POST['selected_character']) && !empty($_POST['action'])) {
	if (!Token::isValid($_POST['token'])) {
		exit('Token de segurança inválido.');
	}

	$action = sanitize($_POST['action']);
	$char_name = sanitize($_POST['selected_character']);

	switch ($action) {
		case 'update_comment':
			if (user_character_account_id($char_name) === $session_user_id) {
				user_update_comment(user_character_id($char_name), $_POST['comment']);
				echo '<p class="success">Comentário atualizado com sucesso.</p>';
			}
			break;

		case 'toggle_hide':
			if (user_character_account_id($char_name) === $session_user_id) {
				$hide = (user_character_hide($char_name) == 1 ? 0 : 1);
				user_character_set_hide(user_character_id($char_name), $hide);
			}
			break;

		case 'delete_character':
			if (user_character_account_id($char_name) === $session_user_id) {
				$charid = user_character_id($char_name);
				if ($charid !== false) {
					if (!user_is_online($charid)) {
						if (guild_leader_gid($charid) === false) {
							user_delete_character_soft($charid);
							echo '<p class="info">Solicitação de exclusão do personagem enviada com sucesso.</p>';
						} else {
							echo '<p class="error">O personagem é líder de uma guilda. É necessário passar a liderança ou desfazer a guilda antes de excluí-lo.</p>';
						}
					} else {
						echo '<p class="error">O personagem precisa estar offline para ser excluído.</p>';
					}
				}
			}
			break;

		case 'change_name':
			$oldname = $char_name;
			$newname = isset($_POST['newName']) ? trim($_POST['newName']) : '';
			$player = mysql_select_single("SELECT `CharacterID` AS `id`, `AccountID` AS `account_id`, `IsOnline` AS `online` FROM `Characters` WHERE `Name` = '$oldname' AND `Deleted` = 0 LIMIT 1;");

			if ($player !== false) {
				if ($player['online'] == 1) {
					$errors[] = 'O personagem precisa estar offline no jogo.';
				}

				$newname = validate_name($newname);
				if ($newname === false) {
					$errors[] = 'O nome não pode conter mais de 2 palavras.';
				} elseif (empty($newname)) {
					$errors[] = 'Por favor, informe um novo nome.';
				} elseif (user_character_exist($newname) !== false) {
					$errors[] = 'Esse nome de personagem já está em uso.';
				} elseif (!preg_match("/^[a-zA-Z ]+$/", $newname)) {
					$errors[] = 'O nome pode conter apenas letras de a-z e espaços.';
				} elseif (strlen($newname) < $config['minL'] || strlen($newname) > $config['maxL']) {
					$errors[] = 'O nome deve ter entre ' . $config['minL'] . ' e ' . $config['maxL'] . ' caracteres.';
				} elseif (!ctype_upper($newname[0])) {
					$errors[] = 'A primeira letra de cada palavra deve ser maiúscula.';
				}

				if (empty($errors)) {
					mysql_update("UPDATE `Characters` SET `Name`='$newname' WHERE `CharacterID`='" . $player['id'] . "' LIMIT 1;");
					echo '<p class="success">Nome do personagem alterado com sucesso para: <strong>' . htmlspecialchars($newname) . '</strong>.</p>';
				} else {
					echo output_errors($errors);
				}
			}
			break;

		case 'change_gender':
			if (user_character_account_id($char_name) === $session_user_id) {
				$char_id = (int)user_character_id($char_name);
				if (!user_is_online($char_id)) {
					user_character_change_gender($char_name);
					echo '<p class="success">Sexo do personagem ' . htmlspecialchars($char_name) . ' alterado com sucesso.</p>';
				} else {
					echo '<p class="error">O personagem precisa estar offline no jogo.</p>';
				}
			}
			break;

		case 'change_comment':
			$render_page = false;
			if (user_character_account_id($char_name) === $session_user_id) {
				$comment_data = user_znote_character_data(user_character_id($char_name), 'comment');
				?>
				<h1>Editar Comentário: <?php echo htmlspecialchars($char_name); ?></h1>
				<form action="myaccount.php" method="post">
					<ul class="form-list">
						<li>
							<input name="action" type="hidden" value="update_comment">
							<input name="selected_character" type="hidden" value="<?php echo htmlspecialchars($char_name); ?>">
							<label for="comment">Comentário do Personagem:</label>
							<textarea name="comment" id="comment" cols="70" rows="8"><?php echo htmlspecialchars($comment_data['comment']); ?></textarea>
						</li>
						<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
						<li><input type="submit" value="Salvar Comentário"></li>
					</ul>
				</form>
				<?php
			}
			break;
	}
}

if ($render_page) {
	$char_count = user_character_list_count($session_user_id);
	$pending_delete = user_pending_deletes($session_user_id);
	if ($pending_delete) {
		foreach ($pending_delete as $delete) {
			if (new DateTime($delete['time']) > new DateTime()) {
				echo '<div class="alert alert-warning">Atenção! O personagem <strong>' . htmlspecialchars($delete['character_name']) . '</strong> será excluído em ' . $delete['time'] . '. <a href="myaccount.php?cancel_delete_id=' . $delete['id'] . '">Cancelar exclusão</a>.</div>';
			} else {
				user_delete_character(user_character_id($delete['character_name']));
				mysql_update('UPDATE `znote_deleted_characters` SET `done` = 1 WHERE `id` = ' . $delete['id']);
				echo '<div class="alert alert-info">Personagem <strong>' . htmlspecialchars($delete['character_name']) . '</strong> excluído permanentemente.</div>';
				$char_count--;
			}
		}
	}

	$acc_data = user_data($session_user_id, 'id', 'email', 'premdays');
	?>
	<div id="myaccount">
		<h1>Minha Conta</h1>
		<div class="account-summary-card">
			<p><strong>Número da Conta:</strong> <?php echo $acc_data['id']; ?></p>
			<p><strong>E-mail:</strong> <?php echo htmlspecialchars($acc_data['email']); ?></p>
			<p><strong>Status da Conta:</strong> 
				<?php 
				if ($acc_data['premdays'] > 0) {
					echo '<span style="color: green; font-weight: bold;">Conta Premium</span> (' . $acc_data['premdays'] . ' dias restantes)';
				} else {
					echo '<span style="color: gray;">Conta Gratuita (Free Account)</span>';
				}
				?>
			</p>
			<p>
				<a href="changepassword.php" class="btn btn-secondary">Alterar Senha</a> 
				<a href="createcharacter.php" class="btn btn-primary">Criar Novo Personagem</a>
			</p>
		</div>

		<h2>Seus Personagens (<?php echo $char_count; ?> / <?php echo $config['max_characters']; ?>)</h2>
		<?php
		$char_array = user_character_list($session_user_id);
		if ($char_array) {
			?>
			<div class="table-responsive">
				<table id="myaccountTable" class="table table-striped table-hover">
					<thead>
						<tr>
							<th>Nome</th>
							<th>Level</th>
							<th>Vocação</th>
							<th>Cidade</th>
							<th>Último Acesso</th>
							<th>Status</th>
							<th>Oculto</th>
						</tr>
					</thead>
					<tbody>
						<?php
						$characters = array();
						foreach ($char_array as $value) {
							echo '<tr>';
							echo '<td><a href="characterprofile.php?name=' . urlencode($value['name']) . '"><strong>' . htmlspecialchars($value['name']) . '</strong></a></td>';
							echo '<td>' . $value['level'] . '</td>';
							echo '<td>' . htmlspecialchars($value['vocation']) . '</td>';
							echo '<td>' . htmlspecialchars($value['town_id']) . '</td>';
							echo '<td>' . $value['lastlogin'] . '</td>';
							echo '<td>' . $value['online'] . '</td>';
							echo '<td>' . hide_char_to_name(user_character_hide($value['name'])) . '</td>';
							echo '</tr>';
							$characters[] = $value['name'];
						}
						?>
					</tbody>
				</table>
			</div>

			<h3>Gerenciar Personagem</h3>
			<form action="myaccount.php" method="post" class="form-inline">
				<div style="display: flex; gap: 10px; flex-wrap: wrap; align-items: center;">
					<select id="selected_character" name="selected_character" class="form-control" required>
						<?php
						foreach ($characters as $char) {
							echo '<option value="' . htmlspecialchars($char) . '">' . htmlspecialchars($char) . '</option>';
						}
						?>
					</select>

					<select id="action" name="action" class="form-control" onchange="changedOption(this)" required>
						<option value="" disabled selected>Escolha uma ação...</option>
						<option value="toggle_hide">Ocultar / Mostrar no perfil</option>
						<option value="change_comment">Editar comentário</option>
						<option value="change_gender">Alterar sexo</option>
						<option value="change_name">Mudar nome</option>
						<option value="delete_character">Excluir personagem</option>
					</select>

					<div id="new_name_container" style="display: none;">
						<input type="text" name="newName" id="newName" placeholder="Novo Nome" class="form-control">
					</div>

					<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
					<input id="submit_button" type="submit" value="Executar Ação" class="btn btn-primary">
				</div>
			</form>
			<?php
		} else {
			echo '<p>Você ainda não possui nenhum personagem criado. <a href="createcharacter.php">Clique aqui para criar um agora</a>!</p>';
		}
		?>
	</div>

	<script>
		function changedOption(select) {
			var nameContainer = document.getElementById('new_name_container');
			if (select.value === 'change_name') {
				nameContainer.style.display = 'block';
				document.getElementById('newName').required = true;
			} else {
				nameContainer.style.display = 'none';
				document.getElementById('newName').required = false;
			}
		}

		document.getElementById('submit_button').addEventListener('click', function(e) {
			var action = document.getElementById('action').value;
			var char = document.getElementById('selected_character').value;
			if (action === 'delete_character') {
				if (!confirm("Tem certeza que deseja solicitar a EXCLUSÃO do personagem " + char + "?")) {
					e.preventDefault();
				}
			}
		});
	</script>
	<?php
}

include 'layout/overall/footer.php';
?>
