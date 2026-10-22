<div class="well loginContainer widget" id="loginContainer">
	<div class="header">
		Entrar / Cadastrar
	</div>
	<div class="body">
		<form class="loginForm" action="login.php" method="post">
			<div class="well">
				<label for="login_username">Nº da Conta:</label> 
				<input type="text" name="username" id="login_username" placeholder="Ex: 111111" required>
			</div>
			<div class="well">
				<label for="login_password">Senha:</label> 
				<input type="password" name="password" id="login_password" required>
			</div>
			<div class="well">
				<input type="submit" value="Entrar" class="submitButton btn btn-primary btn-block">
			</div>
			<?php Token::create(); ?>
			<div style="text-align: center; margin-top: 10px;">
				<p><a href="register.php"><strong>Criar Nova Conta</strong></a></p>
				<p style="font-size: 11px;"><a href="recovery.php">Esqueceu a conta ou senha?</a></p>
			</div>
		</form>
	</div>
</div>
