<?php
require_once 'engine/init.php';
logged_in_redirect();
include 'layout/overall/header.php';

$mode = isset($_GET['mode']) ? sanitize($_GET['mode']) : '';
$email = isset($_POST['email']) ? trim($_POST['email']) : '';
$character = isset($_POST['character']) ? trim($_POST['character']) : '';
$accNumber = isset($_POST['account_number']) ? (int)$_POST['account_number'] : 0;
$password = isset($_POST['password']) ? $_POST['password'] : '';

?>
<h1>Recuperação de Conta</h1>
<?php

if (!empty($_POST)) {
	$status = true;
	if ($config['use_captcha'] && !verifyGoogleReCaptcha($_POST['g-recaptcha-response'])) {
		$status = false;
		echo '<div class="alert alert-danger">Falha na verificação do Captcha.</div>';
	}

	if ($status) {
		$char = mysql_select_single("SELECT `CharacterID`, `AccountID` FROM `Characters` WHERE `Name` = '" . sanitize($character) . "' AND `Deleted` = 0 LIMIT 1;");

		if ($char !== false) {
			$acc = mysql_select_single("SELECT `AccountID`, `Email`, `Auth` FROM `Accounts` WHERE `AccountID` = '" . $char['AccountID'] . "' AND `Deleted` = 0 LIMIT 1;");

			if ($acc !== false && strtolower($acc['Email']) === strtolower($email)) {
				// Recuperar Número da Conta informando a Senha
				if ($mode === 'account') {
					if (cipsoft_verify_password($password, $acc['Auth'])) {
						echo '<div class="alert alert-success"><h3>Conta Encontrada!</h3><p>O número da sua conta é: <strong>' . $acc['AccountID'] . '</strong>.</p><p><a href="login.php">Clique aqui para fazer login</a>.</p></div>';
					} else {
						echo '<div class="alert alert-danger">Senha incorreta para esta conta.</div>';
					}
				}
				// Recuperar Senha informando o Número da Conta
				elseif ($mode === 'password') {
					if ($accNumber === (int)$acc['AccountID']) {
						$newpass = substr(sha1(rand(1000000, 99999999)), 0, 10);
						user_change_password($acc['AccountID'], $newpass);
						echo '<div class="alert alert-success"><h3>Nova Senha Gerada!</h3><p>Sua nova senha de acesso é: <strong>' . $newpass . '</strong></p><p>Faça login imediatamente e altere-a para sua segurança.</p><p><a href="login.php">Ir para a página de login</a>.</p></div>';
					} else {
						echo '<div class="alert alert-danger">O número da conta informado não confere com o personagem e e-mail.</div>';
					}
				}
			} else {
				echo '<div class="alert alert-danger">O e-mail informado não coincide com os registros do personagem.</div>';
			}
		} else {
			echo '<div class="alert alert-danger">Personagem não encontrado.</div>';
		}
	}
} else {
	if ($mode === 'account') {
		?>
		<h2>Recuperar Número da Conta</h2>
		<p>Informe o nome de um personagem da sua conta, o e-mail cadastrado e sua senha:</p>
		<form action="recovery.php?mode=account" method="POST">
			<ul class="form-list">
				<li>
					<label for="character">Nome de um Personagem:</label>
					<input type="text" name="character" id="character" required autofocus>
				</li>
				<li>
					<label for="email">E-mail Cadastrado:</label>
					<input type="email" name="email" id="email" required>
				</li>
				<li>
					<label for="password">Sua Senha:</label>
					<input type="password" name="password" id="password" required>
				</li>
				<?php if ($config['use_captcha']): ?>
				<li>
					<div class="g-recaptcha" data-sitekey="<?php echo $config['captcha_site_key']; ?>"></div>
				</li>
				<?php endif; ?>
				<li>
					<input type="submit" value="Recuperar Número da Conta" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	} elseif ($mode === 'password') {
		?>
		<h2>Recuperar / Redefinir Senha</h2>
		<p>Informe o número da sua conta, o nome de um personagem e o e-mail cadastrado:</p>
		<form action="recovery.php?mode=password" method="POST">
			<ul class="form-list">
				<li>
					<label for="account_number">Número da Conta:</label>
					<input type="number" name="account_number" id="account_number" required autofocus>
				</li>
				<li>
					<label for="character">Nome de um Personagem:</label>
					<input type="text" name="character" id="character" required>
				</li>
				<li>
					<label for="email">E-mail Cadastrado:</label>
					<input type="email" name="email" id="email" required>
				</li>
				<?php if ($config['use_captcha']): ?>
				<li>
					<div class="g-recaptcha" data-sitekey="<?php echo $config['captcha_site_key']; ?>"></div>
				</li>
				<?php endif; ?>
				<li>
					<input type="submit" value="Gerar Nova Senha" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	} else {
		?>
		<p>Selecione a opção de recuperação desejada:</p>
		<ul>
			<li><a href="recovery.php?mode=account"><strong>Esqueci o Número da Minha Conta</strong> (requer nome do char, e-mail e senha)</a></li>
			<li><a href="recovery.php?mode=password"><strong>Esqueci Minha Senha</strong> (requer número da conta, nome do char e e-mail)</a></li>
		</ul>
		<?php
	}
}

include 'layout/overall/footer.php';
?>
