<div class="well loginContainer widget" id="loginContainer">
	<div class="header">
		Login / Register
	</div>
	<div class="body">
		<form class="loginForm" action="login.php" method="post">
			<div class="well">
				<label for="login_username">Account Number:</label> 
				<input type="text" name="username" id="login_username" placeholder="e.g. 111111" required>
			</div>
			<div class="well">
				<label for="login_password">Password:</label> 
				<input type="password" name="password" id="login_password" required>
			</div>
			<div class="well">
				<input type="submit" value="Login" class="submitButton btn btn-primary btn-block">
			</div>
			<?php Token::create(); ?>
			<div style="text-align: center; margin-top: 10px;">
				<p><a href="register.php"><strong>Create Account</strong></a></p>
				<p style="font-size: 11px;"><a href="recovery.php">Forgot account or password?</a></p>
			</div>
		</form>
	</div>
</div>
