<?php
require_once 'engine/init.php';
logged_in_redirect();
include 'layout/overall/header.php';
require_once('config.countries.php');

$errors = array();

if (!empty($_POST)) {
	$required_fields = array('password', 'password_again', 'email', 'selected');
	foreach ($required_fields as $field) {
		if (empty($_POST[$field])) {
			$errors[] = 'Você precisa preencher todos os campos obrigatórios.';
			break;
		}
	}

	if (empty($errors)) {
		if (!Token::isValid($_POST['token'])) {
			$errors[] = 'Token de sessão inválido. Tente enviar novamente.';
		}

		if ($config['use_captcha'] && !verifyGoogleReCaptcha($_POST['g-recaptcha-response'])) {
			$errors[] = 'Por favor, confirme que você não é um robô.';
		}

		// Validar número da conta
		$username = trim($_POST['username']);
		if (!empty($username)) {
			if (!ctype_digit($username)) {
				$errors[] = 'O número da conta deve conter apenas dígitos (0-9).';
			} elseif ((int)$username < 100000 || (int)$username > 99999999) {
				$errors[] = 'O número da conta deve ter entre 6 e 8 dígitos.';
			} elseif (user_exist($username)) {
				$errors[] = 'Este número de conta já está em uso. Por favor, escolha outro ou deixe em branco para gerar aleatoriamente.';
			}
		}

		if (strlen($_POST['password']) < 6) {
			$errors[] = 'Sua senha deve ter no mínimo 6 caracteres.';
		}
		if (strlen($_POST['password']) > 29) {
			$errors[] = 'Sua senha deve ter no máximo 29 caracteres.';
		}
		if ($_POST['password'] !== $_POST['password_again']) {
			$errors[] = 'As senhas digitadas não coincidem.';
		}
		if (!filter_var($_POST['email'], FILTER_VALIDATE_EMAIL)) {
			$errors[] = 'Por favor, informe um endereço de e-mail válido.';
		}
		if (user_email_exist($_POST['email'])) {
			$errors[] = 'Este endereço de e-mail já está cadastrado em outra conta.';
		}
		if ((int)$_POST['selected'] !== 1) {
			$errors[] = 'Você deve concordar com as regras do servidor para criar uma conta.';
		}
		if ($config['validate_IP'] && !validate_ip(getIP())) {
			$errors[] = 'Endereço de IP inválido detectado.';
		}
	}
}

?>
<h1>Criar Conta</h1>
<?php
if (isset($_GET['success'])) {
	$new_acc = isset($_GET['acc']) ? (int)$_GET['acc'] : 0;
	if ($config['mailserver']['register']) {
		?>
		<h2>Confirmação de e-mail necessária</h2>
		<p>Enviamos um link de ativação para o seu e-mail.</p>
		<p>Se não encontrar o e-mail em alguns minutos, verifique sua caixa de <strong>Spam / Lixo Eletrônico</strong>.</p>
		<?php
	} else {
		?>
		<div class="success-box">
			<h2>Conta criada com sucesso!</h2>
			<?php if ($new_acc > 0): ?>
			<p>O número da sua conta é: <strong><?php echo $new_acc; ?></strong></p>
			<?php endif; ?>
			<p>Você já pode <a href="login.php">fazer login</a> para criar seu personagem e jogar.</p>
		</div>
		<?php
	}
} elseif (isset($_GET['authenticate'])) {
	$auid = (isset($_GET['u']) && (int)$_GET['u'] > 0) ? (int)$_GET['u'] : false;
	$akey = (isset($_GET['k']) && (int)$_GET['k'] > 0) ? (int)$_GET['k'] : false;

	$user = mysql_select_single("SELECT `id`, `active`, `active_email` FROM `znote_accounts` WHERE `account_id`='$auid' AND `activekey`='$akey' LIMIT 1;");
	if ($user !== false) {
		$uid = (int)$user['id'];
		mysql_update("UPDATE `znote_accounts` SET `active`='1', `active_email`='1' WHERE `id`= '$uid' LIMIT 1;");
		echo '<h2>Conta Ativada!</h2><p>Sua conta foi ativada com sucesso. Você já pode <a href="login.php">fazer login</a>.</p>';
	} else {
		echo '<h2>Falha na ativação</h2><p>Link de ativação inválido ou conta já ativada.</p>';
	}
} else {
	if (!empty($_POST) && empty($errors)) {
		if ($config['log_ip']) {
			znote_visitor_insert_detailed_data(1);
		}

		$accountNumber = !empty($_POST['username']) ? (int)$_POST['username'] : rand(100000, 999999);
		while (user_exist($accountNumber)) {
			$accountNumber = rand(100000, 999999);
		}

		$flag = !empty($_POST['flag']) ? sanitize($_POST['flag']) : 'br';

		$register_data = array(
			'id'       => $accountNumber,
			'password' => $_POST['password'],
			'email'    => $_POST['email'],
			'created'  => time(),
			'ip'       => getIPLong(),
			'flag'     => $flag
		);

		$accCreated = user_create_account($register_data, $config['mailserver']);

		if (!$config['mailserver']['debug']) {
			header('Location: register.php?success&acc=' . $accCreated);
			exit();
		}
	} elseif (!empty($errors)) {
		echo output_errors($errors);
	}
	?>
	<form action="register.php" method="post">
		<ul class="form-list">
			<li>
				<label for="username">Número da Conta (Opcional - deixe em branco para gerar automaticamente):</label>
				<input type="text" name="username" id="username" placeholder="Ex: 123456 (6 a 8 dígitos)" value="<?php if (isset($_POST['username'])) echo htmlspecialchars($_POST['username']); ?>">
			</li>
			<li>
				<label for="password">Senha:</label>
				<input type="password" name="password" id="password" required>
			</li>
			<li>
				<label for="password_again">Confirmar Senha:</label>
				<input type="password" name="password_again" id="password_again" required>
			</li>
			<li>
				<label for="email">E-mail:</label>
				<input type="email" name="email" id="email" required value="<?php if (isset($_POST['email'])) echo htmlspecialchars($_POST['email']); ?>">
			</li>
			<li>
				<label for="flag">País:</label>
				<select name="flag" id="flag">
					<option value="br">Brasil</option>
					<option value="us">United States</option>
					<option value="pl">Poland</option>
					<option value="se">Sweden</option>
					<option value="gb">United Kingdom</option>
					<option value="">----------</option>
					<?php
					foreach ($config['countries'] as $code => $c) {
						echo '<option value="' . $code . '">' . $c . '</option>';
					}
					?>
				</select>
			</li>
			<?php if ($config['use_captcha']): ?>
			<li>
				<div class="g-recaptcha" data-sitekey="<?php echo $config['captcha_site_key']; ?>"></div>
			</li>
			<?php endif; ?>
			<li>
				<h3>Regras do Servidor</h3>
				<p>1. Respeite os outros jogadores e divirta-se.</p>
				<p>2. É proibido o uso de softwares ilegais ou bots não autorizados.</p>
				<p>3. Não abuse de falhas no jogo (bugs). Comunique imediatamente à equipe.</p>
			</li>
			<li>
				<label for="selected">Você concorda com as regras do servidor?</label>
				<select name="selected" id="selected" required>
					<option value="0">Selecione uma opção</option>
					<option value="1">Sim, eu concordo</option>
					<option value="2">Não</option>
				</select>
			</li>
			<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
			<li>
				<input type="submit" value="Criar Minha Conta">
			</li>
		</ul>
	</form>
	<?php
}

include 'layout/overall/footer.php';
?>
