<?php
require_once 'engine/init.php';
logged_in_redirect();

if (!empty($_POST)) {
	if ($config['log_ip']) {
		znote_visitor_insert_detailed_data(5);
	}

	$username = isset($_POST['username']) ? trim($_POST['username']) : '';
	$password = isset($_POST['password']) ? $_POST['password'] : '';
	$token_post = isset($_POST['token']) ? $_POST['token'] : '';

	if (empty($username) || empty($password)) {
		$errors[] = 'Você precisa informar o número da conta e a senha.';
	} else if (!is_numeric($username)) {
		$errors[] = 'O número da conta deve ser numérico.';
	} else if (strlen($username) > 32 || strlen($password) > 64) {
		$errors[] = 'Número da conta ou senha muito longos.';
	} else if ($config['use_token'] && !Token::isValid($token_post)) {
		$errors[] = 'Token de segurança expirado ou inválido. Por favor, tente novamente.';
	} else if (!user_exist($username)) {
		$errors[] = 'Conta não encontrada. Verifique os dados ou <a href="register.php">crie uma nova conta</a>.';
	} else {
		$login = user_login($username, $password);
		if ($login === false) {
			$errors[] = 'Número de conta ou senha incorretos.';
		} else {
			$status = true;
			if ($config['mailserver']['register']) {
				$authenticate = mysql_select_single("SELECT `id` FROM `znote_accounts` WHERE `account_id`='$login' AND `active`='1' LIMIT 1;");
				if ($authenticate === false) {
					$status = false;
					$errors[] = "Sua conta ainda não foi ativada. Verifique o link enviado ao seu e-mail de cadastro.";
				}
			}

			if ($status) {
				setSession('user_id', $login);

				$znote_data = user_znote_account_data($login, 'ip');
				if ($znote_data !== false && $znote_data['ip'] == 0) {
					user_update_znote_account(array('ip' => getIPLong()));
				}

				header('Location: myaccount.php');
				exit();
			}
		}
	}
}

include 'layout/overall/header.php';
?>
<h1>Entrar na Conta</h1>
<p>Informe o número da sua conta e senha para acessar o painel de gerenciamento.</p>

<?php
if (!empty($errors)) {
	echo output_errors($errors);
}
?>

<form action="login.php" method="post">
	<ul class="form-list">
		<li>
			<label for="username">Número da Conta:</label>
			<input type="text" name="username" id="username" placeholder="Ex: 111111" value="<?php echo isset($_POST['username']) ? htmlspecialchars($_POST['username']) : ''; ?>" required autofocus>
		</li>
		<li>
			<label for="password">Senha:</label>
			<input type="password" name="password" id="password" required>
		</li>
		<?php if ($config['use_token']): ?>
			<?php Token::create(); ?>
		<?php endif; ?>
		<li>
			<input type="submit" value="Entrar" class="btn btn-primary">
		</li>
	</ul>
</form>

<p>Não tem uma conta ainda? <a href="register.php">Clique aqui para se registrar</a>.</p>
<p>Esqueceu seus dados de acesso? <a href="recovery.php">Recupere sua conta</a>.</p>

<?php
include 'layout/overall/footer.php';
?>
