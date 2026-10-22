<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';
require_once('config.countries.php');

$errors = array();

if (!empty($_POST)) {
	if (!Token::isValid($_POST['token'])) {
		$errors[] = 'Invalid session token.';
	}
	$required_fields = array('new_email', 'new_flag');
	foreach ($required_fields as $field) {
		if (empty($_POST[$field])) {
			$errors[] = 'All fields are required.';
			break;
		}
	}

	if (empty($errors)) {
		if (!filter_var($_POST['new_email'], FILTER_VALIDATE_EMAIL)) {
			$errors[] = 'Invalid email address.';
		} elseif (user_email_exist($_POST['new_email']) && $user_data['email'] !== $_POST['new_email']) {
			$errors[] = 'This email is already in use by another account.';
		}
	}
}

?>
<h1>Account Settings</h1>

<?php
if (isset($_GET['success'])) {
	echo '<div class="alert alert-success">Your settings have been updated successfully!</div>';
} else {
	if (!empty($_POST) && empty($errors)) {
		$new_email = sanitize($_POST['new_email']);
		$new_flag = sanitize($_POST['new_flag']);

		// Update Email in Accounts table
		mysql_update("UPDATE `Accounts` SET `Email`='$new_email' WHERE `AccountID`='$session_user_id' LIMIT 1;");

		// Update Flag in znote_accounts
		mysql_update("UPDATE `znote_accounts` SET `flag`='$new_flag' WHERE `account_id`='$session_user_id' LIMIT 1;");

		header('Location: settings.php?success');
		exit();
	} elseif (!empty($errors)) {
		echo output_errors($errors);
	}
	?>

	<form action="settings.php" method="post">
		<ul class="form-list">
			<li>
				<label for="new_email">Email Address:</label>
				<input type="email" name="new_email" id="new_email" value="<?php echo htmlspecialchars($user_data['email']); ?>" required>
			</li>
			<li>
				<label for="new_flag">Country:</label>
				<select name="new_flag" id="new_flag" required>
					<option value="">(Select your country)</option>
					<option value="br" <?php if (isset($user_znote_data['flag']) && $user_znote_data['flag'] == 'br') echo 'selected'; ?>>Brazil</option>
					<option value="us" <?php if (isset($user_znote_data['flag']) && $user_znote_data['flag'] == 'us') echo 'selected'; ?>>United States</option>
					<option value="pl" <?php if (isset($user_znote_data['flag']) && $user_znote_data['flag'] == 'pl') echo 'selected'; ?>>Poland</option>
					<option value="se" <?php if (isset($user_znote_data['flag']) && $user_znote_data['flag'] == 'se') echo 'selected'; ?>>Sweden</option>
					<option value="gb" <?php if (isset($user_znote_data['flag']) && $user_znote_data['flag'] == 'gb') echo 'selected'; ?>>United Kingdom</option>
					<option value="">----------</option>
					<?php
					foreach ($config['countries'] as $code => $c) {
						$sel = (isset($user_znote_data['flag']) && $user_znote_data['flag'] == $code) ? 'selected' : '';
						echo '<option value="' . $code . '" ' . $sel . '>' . $c . '</option>';
					}
					?>
				</select>
			</li>
			<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
			<li>
				<input type="submit" value="Save Changes" class="btn btn-primary">
			</li>
		</ul>
	</form>
	<?php
}

include 'layout/overall/footer.php';
?>
