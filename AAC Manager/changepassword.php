<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';

$errors = array();

if (!empty($_POST)) {
	if (!Token::isValid($_POST['token'])) {
		$errors[] = 'Token de segurança inválido.';
	}

	$required_fields = array('current_password', 'new_password', 'new_password_again');
	foreach ($required_fields as $field) {
		if (empty($_POST[$field])) {
			$errors[] = 'Você precisa preencher todos os campos.';
			break;
		}
	}

	if (empty($errors)) {
		$acc = mysql_select_single("SELECT `Auth` FROM `Accounts` WHERE `AccountID` = '$session_user_id' LIMIT 1;");
		if ($acc !== false && cipsoft_verify_password($_POST['current_password'], $acc['Auth'])) {
			if ($_POST['new_password'] !== $_POST['new_password_again']) {
				$errors[] = 'As novas senhas digitadas não coincidem.';
			} elseif (strlen($_POST['new_password']) < 6) {
				$errors[] = 'Sua nova senha deve ter no mínimo 6 caracteres.';
			} elseif (strlen($_POST['new_password']) > 64) {
				$errors[] = 'Sua nova senha deve ter no máximo 64 caracteres.';
			} else {
				user_change_password($session_user_id, $_POST['new_password']);
				header('Location: changepassword.php?success');
				exit();
			}
		} else {
			$errors[] = 'A senha atual informada está incorreta.';
		}
	}
}

?>
<h1>Alterar Senha</h1>

<?php
if (isset($_GET['success'])) {
	echo '<div class="alert alert-success">Sua senha foi alterada com sucesso! Você precisará fazer login novamente.</div>';
	session_destroy();
	header("refresh:3;url=login.php");
	exit();
} else {
	if (!empty($errors)) {
		echo output_errors($errors);
	}
	?>
	<form action="changepassword.php" method="post">
		<ul class="form-list">
			<li>
				<label for="current_password">Senha Atual:</label>
				<input type="password" name="current_password" id="current_password" required autofocus>
			</li>
			<li>
				<label for="new_password">Nova Senha:</label>
				<input type="password" name="new_password" id="new_password" required>
			</li>
			<li>
				<label for="new_password_again">Confirmar Nova Senha:</label>
				<input type="password" name="new_password_again" id="new_password_again" required>
			</li>
			<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
			<li>
				<input type="submit" value="Salvar Nova Senha" class="btn btn-primary">
			</li>
		</ul>
	</form>
	<?php
}

include 'layout/overall/footer.php';
?>
