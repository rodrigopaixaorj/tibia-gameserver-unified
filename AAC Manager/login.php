<?php
require_once 'engine/init.php';
logged_in_redirect();
include 'layout/overall/header.php';

if (!empty($_POST)) {
	if ($config['log_ip']) {
		znote_visitor_insert_detailed_data(5);
	}

	$username = isset($_POST['username']) ? trim($_POST['username']) : '';
	$password = isset($_POST['password']) ? $_POST['password'] : '';

	if (empty($username) || empty($password)) {
		$errors[] = 'Você precisa informar o número da conta e a senha.';
	} else if (!is_numeric($username)) {
		$errors[] = 'O número da conta deve ser numérico.';
	} else if (strlen($username) > 32 || strlen($password) > 64) {
		$errors[] = 'Número da conta ou senha muito longos.';
	} else if (!user_exist($username)) {
		$errors[] = 'Conta não encontrada. Verifique os dados ou <a href="register.php">crie uma nova conta</a>.';
	} else if ($config['use_token'] && !Token::isValid($_POST['token'])) {
		$errors[] = 'Token de segurança inválido. Tente novamente.';
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

if (!empty($errors)) {
	?>
	<h1>Falha no Login</h1>
	<?php
	echo output_errors($errors);
} else {
	?>
	<h1>Entrar na Conta</h1>
	<p>Informe o número da sua conta e senha para acessar o painel de gerenciamento.</p>
	<form action="login.php" method="post">
		<ul class="form-list">
			<li>
				<label for="username">Número da Conta:</label>
				<input type="text" name="username" id="username" placeholder="Ex: 111111" required autofocus>
			</li>
			<li>
				<label for="password">Senha:</label>
				<input type="password" name="password" id="password" required>
			</li>
			<?php if ($config['use_token']): ?>
			<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
			<?php endif; ?>
			<li>
				<input type="submit" value="Entrar">
			</li>
		</ul>
	</form>
	<p>Não tem uma conta ainda? <a href="register.php">Clique aqui para se registrar</a>.</p>
	<p>Esqueceu seus dados de acesso? <a href="recovery.php">Recupere sua conta</a>.</p>
	<?php
}

include 'layout/overall/footer.php';
?>
