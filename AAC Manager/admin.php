<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';

if (!isset($_SESSION['csrf_token'])) {
	$_SESSION['csrf_token'] = bin2hex(random_bytes(16));
}

protect_page();
admin_only($user_data);

$enc = 100;
$messages = array();

if (!empty($_POST)) {
	if (empty($_POST['csrf_token']) || !hash_equals($_POST['csrf_token'], $_SESSION['csrf_token'])) {
		http_response_code(400);
		die("Erro: Token CSRF inválido!");
	}

	// Sistema de Banimento
	if (!empty($_POST['ban_char']) && !empty($_POST['ban_reason']) && !empty($_POST['ban_time']) && !empty($_POST['ban_comment'])) {
		$charname = sanitize($_POST['ban_char']);
		if (user_character_exist($charname)) {
			$reasonid = (int)$_POST['ban_reason'] - $enc;
			$time = (int)$_POST['ban_time'] - $enc;
			$comment = sanitize($_POST['ban_comment']);

			if (set_rule_violation($charname, 1, 1, $reasonid, $time, $comment)) {
				$messages[] = 'Banimento aplicado com sucesso ao personagem ' . htmlspecialchars($charname) . '.';
			} else {
				$messages[] = 'Erro ao registrar banimento para o personagem.';
			}
		} else {
			$messages[] = 'O personagem ' . htmlspecialchars($charname) . ' não existe.';
		}
	}

	// Excluir Personagem Permanentemente
	if (!empty($_POST['del_name'])) {
		$del_name = sanitize($_POST['del_name']);
		$cid = user_character_id($del_name);
		if ($cid !== false) {
			user_delete_character($cid);
			$messages[] = 'Personagem ' . htmlspecialchars($del_name) . ' excluído permanentemente.';
		} else {
			$messages[] = 'O personagem ' . htmlspecialchars($del_name) . ' não foi encontrado.';
		}
	}

	// Redefinir Senha da Conta
	if (!empty($_POST['reset_pass']) && !empty($_POST['new_pass'])) {
		$char_name = sanitize($_POST['reset_pass']);
		if (user_character_exist($char_name)) {
			$acc_id = user_character_account_id($char_name);
			user_change_password($acc_id, $_POST['new_pass']);
			$messages[] = 'A senha da conta (' . $acc_id . ') do personagem ' . htmlspecialchars($char_name) . ' foi alterada para: ' . htmlspecialchars($_POST['new_pass']) . '.';
		} else {
			$messages[] = 'Personagem não encontrado.';
		}
	}

	// Dar Pontos de Loja
	if (!empty($_POST['points_char']) && !empty($_POST['points_value'])) {
		$char = sanitize($_POST['points_char']);
		$points = (int)$_POST['points_value'];
		$account = mysql_select_single("SELECT `AccountID` FROM `Characters` WHERE `Name`='$char' LIMIT 1;");
		if ($account !== false) {
			$acc_id = $account['AccountID'];
			$znote_account = mysql_select_single("SELECT `points` FROM `znote_accounts` WHERE `account_id`='$acc_id' LIMIT 1;");
			$currPoints = ($znote_account !== false) ? (int)$znote_account['points'] : 0;
			$totalPoints = $currPoints + $points;
			mysql_update("UPDATE `znote_accounts` SET `points`='$totalPoints' WHERE `account_id`='$acc_id';");
			$messages[] = 'Adicionado ' . $points . ' pontos à conta ' . $acc_id . ' (Total: ' . $totalPoints . ').';
		} else {
			$messages[] = 'Personagem não encontrado.';
		}
	}

	// Teleportar Jogador (Atualiza PlayerState)
	if (!empty($_POST['teleport_char'])) {
		$tp_char = sanitize($_POST['teleport_char']);
		$tp_cid = user_character_id($tp_char);
		if ($tp_cid !== false) {
			$town_id = (int)$_POST['teleport_town'];
			$defPos = $config['default_pos'];
			mysql_update("UPDATE `PlayerState` SET `PosX`='{$defPos['x']}', `PosY`='{$defPos['y']}', `PosZ`='{$defPos['z']}' WHERE `CharacterID`='$tp_cid' LIMIT 1;");
			if (isset($config['towns'][$town_id])) {
				$townName = $config['towns'][$town_id];
				mysql_update("UPDATE `Characters` SET `Residence`='$townName' WHERE `CharacterID`='$tp_cid' LIMIT 1;");
			}
			$messages[] = 'Personagem ' . htmlspecialchars($tp_char) . ' teleportado com sucesso para a posição padrão do templo.';
		} else {
			$messages[] = 'Personagem não encontrado.';
		}
	}
}

?>

<h1>Painel de Administração</h1>

<?php
if (!empty($messages)) {
	echo '<div class="alert alert-info"><ul>';
	foreach ($messages as $msg) {
		echo '<li>' . $msg . '</li>';
	}
	echo '</ul></div>';
}
?>

<div class="row" style="display: flex; gap: 20px; flex-wrap: wrap;">
	<!-- Banimentos -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Banir Jogador</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<label for="ban_char">Nome do Personagem:</label>
			<input type="text" name="ban_char" id="ban_char" class="form-control" required style="margin-bottom: 8px;">

			<label for="ban_reason">Motivo:</label>
			<select name="ban_reason" id="ban_reason" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['ban_reason'] as $id => $reason): ?>
					<option value="<?php echo $id + $enc; ?>"><?php echo htmlspecialchars($reason); ?></option>
				<?php endforeach; ?>
			</select>

			<label for="ban_time">Duração:</label>
			<select name="ban_time" id="ban_time" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['ban_time'] as $sec => $desc): ?>
					<option value="<?php echo $sec + $enc; ?>"><?php echo htmlspecialchars($desc); ?></option>
				<?php endforeach; ?>
			</select>

			<label for="ban_comment">Comentário:</label>
			<input type="text" name="ban_comment" id="ban_comment" class="form-control" placeholder="Motivo detalhado" required style="margin-bottom: 12px;">

			<input type="submit" value="Aplicar Banimento" class="btn btn-danger">
		</form>
	</div>

	<!-- Gerenciamento de Senha e Pontos -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Redefinir Senha da Conta</h3>
		<form action="admin.php" method="post" style="margin-bottom: 20px;">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="reset_pass" placeholder="Nome de um personagem" class="form-control" required style="margin-bottom: 8px;">
			<input type="text" name="new_pass" placeholder="Nova senha" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Alterar Senha" class="btn btn-warning">
		</form>

		<h3>Entregar Pontos de Loja</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="points_char" placeholder="Nome do personagem" class="form-control" required style="margin-bottom: 8px;">
			<input type="number" name="points_value" placeholder="Quantidade de pontos" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Entregar Pontos" class="btn btn-primary">
		</form>
	</div>

	<!-- Teleport e Exclusão -->
	<div style="flex: 1; min-width: 300px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 4px;">
		<h3>Teleportar Personagem para o Templo</h3>
		<form action="admin.php" method="post" style="margin-bottom: 20px;">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="teleport_char" placeholder="Nome do personagem" class="form-control" required style="margin-bottom: 8px;">
			<label for="teleport_town">Cidade de Destino:</label>
			<select name="teleport_town" id="teleport_town" class="form-control" style="margin-bottom: 8px;">
				<?php foreach ($config['towns'] as $tid => $tname): ?>
					<option value="<?php echo $tid; ?>"><?php echo htmlspecialchars($tname); ?></option>
				<?php endforeach; ?>
			</select>
			<input type="submit" value="Teleportar" class="btn btn-secondary">
		</form>

		<h3>Excluir Personagem</h3>
		<form action="admin.php" method="post">
			<input type="hidden" name="csrf_token" value="<?php echo $_SESSION['csrf_token']; ?>">
			<input type="text" name="del_name" placeholder="Nome do personagem" class="form-control" required style="margin-bottom: 8px;">
			<input type="submit" value="Excluir Permanentemente" class="btn btn-danger" onclick="return confirm('ATENÇÃO: Deseja realmente excluir este personagem?');">
		</form>
	</div>
</div>

<?php include 'layout/overall/footer.php'; ?>
