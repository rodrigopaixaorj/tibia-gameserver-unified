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
<h1>Account Recovery</h1>
<?php

if (!empty($_POST)) {
	$status = true;
	if ($config['use_captcha'] && !verifyGoogleReCaptcha($_POST['g-recaptcha-response'])) {
		$status = false;
		echo '<div class="alert alert-danger">Captcha verification failed.</div>';
	}

	if ($status) {
		$char = mysql_select_single("SELECT `CharacterID`, `AccountID` FROM `Characters` WHERE `Name` = '" . sanitize($character) . "' AND `Deleted` = 0 LIMIT 1;");

		if ($char !== false) {
			$acc = mysql_select_single("SELECT `AccountID`, `Email`, `Auth` FROM `Accounts` WHERE `AccountID` = '" . $char['AccountID'] . "' AND `Deleted` = 0 LIMIT 1;");

			if ($acc !== false && strtolower($acc['Email']) === strtolower($email)) {
				// Recover Account Number by providing password
				if ($mode === 'account') {
					if (cipsoft_verify_password($password, $acc['Auth'])) {
						echo '<div class="alert alert-success"><h3>Account Found!</h3><p>Your account number is: <strong>' . $acc['AccountID'] . '</strong>.</p><p><a href="login.php">Click here to log in</a>.</p></div>';
					} else {
						echo '<div class="alert alert-danger">Incorrect password for this account.</div>';
					}
				}
				// Recover Password by providing Account Number
				elseif ($mode === 'password') {
					if ($accNumber === (int)$acc['AccountID']) {
						$newpass = substr(sha1(rand(1000000, 99999999)), 0, 10);
						user_change_password($acc['AccountID'], $newpass);
						echo '<div class="alert alert-success"><h3>New Password Generated!</h3><p>Your new access password is: <strong>' . $newpass . '</strong></p><p>Please log in immediately and change it for your security.</p><p><a href="login.php">Go to login page</a>.</p></div>';
					} else {
						echo '<div class="alert alert-danger">The provided account number does not match character and email records.</div>';
					}
				}
			} else {
				echo '<div class="alert alert-danger">The provided email does not match character records.</div>';
			}
		} else {
			echo '<div class="alert alert-danger">Character not found.</div>';
		}
	}
} else {
	if ($mode === 'account') {
		?>
		<h2>Recover Account Number</h2>
		<p>Enter the name of a character on your account, the registered email, and your password:</p>
		<form action="recovery.php?mode=account" method="POST">
			<ul class="form-list">
				<li>
					<label for="character">Character Name:</label>
					<input type="text" name="character" id="character" required autofocus>
				</li>
				<li>
					<label for="email">Registered Email:</label>
					<input type="email" name="email" id="email" required>
				</li>
				<li>
					<label for="password">Password:</label>
					<input type="password" name="password" id="password" required>
				</li>
				<?php if ($config['use_captcha']): ?>
				<li>
					<div class="g-recaptcha" data-sitekey="<?php echo $config['captcha_site_key']; ?>"></div>
				</li>
				<?php endif; ?>
				<li>
					<input type="submit" value="Recover Account Number" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	} elseif ($mode === 'password') {
		?>
		<h2>Recover / Reset Password</h2>
		<p>Enter your account number, the name of a character, and the registered email:</p>
		<form action="recovery.php?mode=password" method="POST">
			<ul class="form-list">
				<li>
					<label for="account_number">Account Number:</label>
					<input type="number" name="account_number" id="account_number" required autofocus>
				</li>
				<li>
					<label for="character">Character Name:</label>
					<input type="text" name="character" id="character" required>
				</li>
				<li>
					<label for="email">Registered Email:</label>
					<input type="email" name="email" id="email" required>
				</li>
				<?php if ($config['use_captcha']): ?>
				<li>
					<div class="g-recaptcha" data-sitekey="<?php echo $config['captcha_site_key']; ?>"></div>
				</li>
				<?php endif; ?>
				<li>
					<input type="submit" value="Generate New Password" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	} else {
		?>
		<p>Select your recovery option:</p>
		<ul>
			<li><a href="recovery.php?mode=account"><strong>Forgot My Account Number</strong> (requires character name, email, and password)</a></li>
			<li><a href="recovery.php?mode=password"><strong>Forgot My Password</strong> (requires account number, character name, and email)</a></li>
		</ul>
		<?php
	}
}

include 'layout/overall/footer.php';
?>
