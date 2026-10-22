<nav>
	<div class="container">
		<!-- Menu alinhado à esquerda -->
		<div class="pull-left">
			<ul>
				<li><a href="index.php"><i class="fa fa-home"></i> Início</a>
					<ul>
						<li><a href="changelog.php">Histórico de Atualizações</a></li>
					</ul>
				</li>
				<li><a id="accountLink" href="myaccount.php"><i class="fa fa-user-circle"></i> Conta</a>
					<ul>
						<li><a href="downloads.php">Downloads / Como Jogar</a></li>
						<li><a href="recovery.php">Recuperar Conta</a></li>
						<li><a href="myaccount.php">Minha Conta</a></li>
						<li><a href="register.php">Criar Conta</a></li>
					</ul>
				</li>
				<li><a href="onlinelist.php"><i class="fa fa-users"></i> Comunidade</a>
					<ul>
						<li><a href="forum.php">Fórum</a></li>
						<li><a href="guilds.php">Guildas</a></li>
						<li><a href="highscores.php">Rankings (Highscores)</a></li>
						<li><a href="houses.php">Casas</a></li>
						<li><a href="killers.php">Estatísticas PvP</a></li>
						<li><a href="deaths.php">Últimas Mortes</a></li>
						<li><a href="onlinelist.php">Quem está Online</a></li>
					</ul>
				</li>
				<li><a href="serverinfo.php"><i class="fa fa-book"></i> Biblioteca</a>
					<ul>
						<li><a href="serverinfo.php">Informações do Servidor</a></li>
						<li><a href="spells.php">Magias</a></li>
					</ul>
				</li>
				<li><a href="support.php"><i class="fa fa-info-circle"></i> Suporte</a>
					<ul>
						<li><a href="support.php">Equipe do Servidor</a></li>
						<li><a href="helpdesk.php">Atendimento / Tickets</a></li>
						<li><a href="recovery.php">Recuperar Conta</a></li>
					</ul>
				</li>
				<li><a href="shop.php"><i class="fa fa-shopping-cart"></i> Loja</a>
					<ul>
						<li><a href="buypoints.php">Adquirir Pontos</a></li>
						<li><a href="shop.php">Loja Virtual</a></li>
					</ul>
				</li>
			</ul>
		</div>
		<!-- Menu alinhado à direita -->
		<div class="pull-right">
			<ul>
				<?php if (user_logged_in()): ?>
					<li><a href="myaccount.php"><i class="fa fa-user"></i> Minha Conta</a></li>
					<li><a href="logout.php"><i class="fa fa-sign-out"></i> Sair</a></li>
				<?php else: ?>
					<li><a href="login.php" class="modIcon loginBtn"><i class="fa fa-lock"></i> Entrar</a></li>
					<li><a href="register.php"><i class="fa fa-key"></i> Cadastre-se</a></li>
				<?php endif; ?>
			</ul>
		</div>
	</div>
</nav>