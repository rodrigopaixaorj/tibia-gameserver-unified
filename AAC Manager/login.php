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
		$errors[] = 'You need to enter both account number and password.';
	} else if (!is_numeric($username)) {
		$errors[] = 'Account number must be numeric.';
	} else if (strlen($username) > 32 || strlen($password) > 64) {
		$errors[] = 'Account number or password is too long.';
	} else if ($config['use_token'] && !Token::isValid($token_post)) {
		$errors[] = 'Security token expired or invalid. Please try again.';
	} else if (!user_exist($username)) {
		$errors[] = 'Account not found. Please check your credentials or <a href="register.php">create a new account</a>.';
	} else {
		$login = user_login($username, $password);
		if ($login === false) {
			$errors[] = 'Incorrect account number or password.';
		} else {
			$status = true;
			if ($config['mailserver']['register']) {
				$authenticate = mysql_select_single("SELECT `id` FROM `znote_accounts` WHERE `account_id`='$login' AND `active`='1' LIMIT 1;");
				if ($authenticate === false) {
					$status = false;
					$errors[] = "Your account is not activated yet. Please check your registration email.";
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
<h1>Account Login</h1>
<p>Please enter your account number and password to access the management panel.</p>

<?php
if (!empty($errors)) {
	echo output_errors($errors);
}
?>

<form action="login.php" method="post">
	<ul class="form-list">
		<li>
			<label for="username">Account Number:</label>
			<input type="text" name="username" id="username" placeholder="e.g. 111111" value="<?php echo isset($_POST['username']) ? htmlspecialchars($_POST['username']) : ''; ?>" required autofocus>
		</li>
		<li>
			<label for="password">Password:</label>
			<input type="password" name="password" id="password" required>
		</li>
		<?php if ($config['use_token']): ?>
			<?php Token::create(); ?>
		<?php endif; ?>
		<li>
			<input type="submit" value="Login" class="btn btn-primary">
		</li>
	</ul>
</form>

<p>Do not have an account yet? <a href="register.php">Click here to register</a>.</p>
<p>Forgot your account information? <a href="recovery.php">Recover your account</a>.</p>

<?php
include 'layout/overall/footer.php';
?>
