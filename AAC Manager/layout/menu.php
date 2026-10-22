<nav>
	<div class="container">
		<!-- Left Menu -->
		<div class="pull-left">
			<ul>
				<li><a href="index.php"><i class="fa fa-home"></i> Home</a>
					<ul>
						<li><a href="changelog.php">Changelog</a></li>
					</ul>
				</li>
				<li><a id="accountLink" href="myaccount.php"><i class="fa fa-user-circle"></i> Account</a>
					<ul>
						<li><a href="downloads.php">Downloads / How to Play</a></li>
						<li><a href="recovery.php">Lost Account</a></li>
						<li><a href="myaccount.php">My Account</a></li>
						<li><a href="register.php">Create Account</a></li>
					</ul>
				</li>
				<li><a href="onlinelist.php"><i class="fa fa-users"></i> Community</a>
					<ul>
						<li><a href="forum.php">Forum</a></li>
						<li><a href="guilds.php">Guilds</a></li>
						<li><a href="highscores.php">Highscores</a></li>
						<li><a href="houses.php">Houses</a></li>
						<li><a href="killers.php">Top Fraggers</a></li>
						<li><a href="deaths.php">Latest Deaths</a></li>
						<li><a href="onlinelist.php">Who is Online?</a></li>
					</ul>
				</li>
				<li><a href="serverinfo.php"><i class="fa fa-book"></i> Library</a>
					<ul>
						<li><a href="serverinfo.php">Server Information</a></li>
						<li><a href="spells.php">Spells</a></li>
					</ul>
				</li>
				<li><a href="support.php"><i class="fa fa-info-circle"></i> Support</a>
					<ul>
						<li><a href="support.php">Staff Team</a></li>
						<li><a href="helpdesk.php">Helpdesk / Tickets</a></li>
						<li><a href="recovery.php">Lost Account</a></li>
					</ul>
				</li>
				<li><a href="shop.php"><i class="fa fa-shopping-cart"></i> Shop</a>
					<ul>
						<li><a href="buypoints.php">Buy Points</a></li>
						<li><a href="shop.php">Shop Offers</a></li>
					</ul>
				</li>
			</ul>
		</div>
		<!-- Right Menu -->
		<div class="pull-right">
			<ul>
				<?php if (user_logged_in()): ?>
					<li><a href="myaccount.php"><i class="fa fa-user"></i> My Account</a></li>
					<li><a href="logout.php"><i class="fa fa-sign-out"></i> Logout</a></li>
				<?php else: ?>
					<li><a href="login.php" class="modIcon loginBtn"><i class="fa fa-lock"></i> Login</a></li>
					<li><a href="register.php"><i class="fa fa-key"></i> Register</a></li>
				<?php endif; ?>
			</ul>
		</div>
	</div>
</nav>