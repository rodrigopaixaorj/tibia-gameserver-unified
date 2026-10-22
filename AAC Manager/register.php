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
			$errors[] = 'You need to fill in all required fields.';
			break;
		}
	}

	if (empty($errors)) {
		if (!Token::isValid($_POST['token'])) {
			$errors[] = 'Invalid session token. Please try submitting again.';
		}

		if ($config['use_captcha'] && !verifyGoogleReCaptcha($_POST['g-recaptcha-response'])) {
			$errors[] = 'Please verify that you are not a robot.';
		}

		// Validate account number
		$username = trim($_POST['username']);
		if (!empty($username)) {
			if (!ctype_digit($username)) {
				$errors[] = 'Account number must contain only numbers (0-9).';
			} elseif ((int)$username < 100000 || (int)$username > 99999999) {
				$errors[] = 'Account number must be between 6 and 8 digits.';
			} elseif (user_exist($username)) {
				$errors[] = 'This account number is already in use. Please choose another or leave it blank to generate automatically.';
			}
		}

		if (strlen($_POST['password']) < 6) {
			$errors[] = 'Your password must be at least 6 characters long.';
		}
		if (strlen($_POST['password']) > 29) {
			$errors[] = 'Your password cannot exceed 29 characters.';
		}
		if ($_POST['password'] !== $_POST['password_again']) {
			$errors[] = 'The entered passwords do not match.';
		}
		if (!filter_var($_POST['email'], FILTER_VALIDATE_EMAIL)) {
			$errors[] = 'Please provide a valid email address.';
		}
		if (user_email_exist($_POST['email'])) {
			$errors[] = 'This email address is already registered to another account.';
		}
		if ((int)$_POST['selected'] !== 1) {
			$errors[] = 'You must agree to the server rules to create an account.';
		}
		if ($config['validate_IP'] && !validate_ip(getIP())) {
			$errors[] = 'Invalid IP address detected.';
		}
	}
}

?>
<h1>Create Account</h1>
<?php
if (isset($_GET['success'])) {
	$new_acc = isset($_GET['acc']) ? (int)$_GET['acc'] : 0;
	if ($config['mailserver']['register']) {
		?>
		<h2>Email confirmation required</h2>
		<p>We have sent an activation link to your email.</p>
		<p>If you cannot find it within a few minutes, please check your <strong>Spam / Junk</strong> folder.</p>
		<?php
	} else {
		?>
		<div class="success-box">
			<h2>Account created successfully!</h2>
			<?php if ($new_acc > 0): ?>
			<p>Your account number is: <strong><?php echo $new_acc; ?></strong></p>
			<?php endif; ?>
			<p>You can now <a href="login.php">login</a> to create your character and play.</p>
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
		echo '<h2>Account Activated!</h2><p>Your account has been activated successfully. You can now <a href="login.php">login</a>.</p>';
	} else {
		echo '<h2>Activation failed</h2><p>Invalid activation link or account already activated.</p>';
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

		$flag = !empty($_POST['flag']) ? sanitize($_POST['flag']) : 'us';

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
				<label for="username">Account Number (Optional - leave blank to generate automatically):</label>
				<input type="text" name="username" id="username" placeholder="e.g. 123456 (6 to 8 digits)" value="<?php if (isset($_POST['username'])) echo htmlspecialchars($_POST['username']); ?>">
			</li>
			<li>
				<label for="password">Password:</label>
				<input type="password" name="password" id="password" required>
			</li>
			<li>
				<label for="password_again">Repeat Password:</label>
				<input type="password" name="password_again" id="password_again" required>
			</li>
			<li>
				<label for="email">Email:</label>
				<input type="email" name="email" id="email" required value="<?php if (isset($_POST['email'])) echo htmlspecialchars($_POST['email']); ?>">
			</li>
			<li>
				<label for="flag">Country:</label>
				<select name="flag" id="flag">
					<option value="us">United States</option>
					<option value="gb">United Kingdom</option>
					<option value="pl">Poland</option>
					<option value="se">Sweden</option>
					<option value="br">Brazil</option>
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
				<h3>Server Rules</h3>
				<p>1. Respect other players and have fun.</p>
				<p>2. The use of unauthorized bots or malicious tools is strictly prohibited.</p>
				<p>3. Do not exploit bugs. Report any issues immediately to the staff.</p>
			</li>
			<li>
				<label for="selected">Do you agree to the server rules?</label>
				<select name="selected" id="selected" required>
					<option value="0">Select an option</option>
					<option value="1">Yes, I agree</option>
					<option value="2">No</option>
				</select>
			</li>
			<?php Token::create(); ?>
			<li>
				<input type="submit" value="Create Account" class="btn btn-primary">
			</li>
		</ul>
	</form>
	<?php
}

include 'layout/overall/footer.php';
?>
