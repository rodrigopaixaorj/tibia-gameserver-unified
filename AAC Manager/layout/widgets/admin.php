<div class="well myaccount_widget widget" id="loginContainer">
	<div class="header">
		Administração
	</div>
	<div class="body">
		<ul class="linkbuttons">
			<li>
				<a href='admin.php'>Painel de Admin</a>
			</li>
			<li>
				<a href='admin_news.php'>Notícias</a>
			</li>
			<li>
				<a href='admin_gallery.php'>Galeria</a>
			</li>
			<li>
				<a href='admin_skills.php'>Habilidades</a>
			</li>
			<li>
				<a href='admin_reports.php'>Relatórios</a>
			</li>
			<li>
				<a href='admin_helpdesk.php'>Atendimento</a>
			</li>
			<li>
				<a href='admin_shop.php'>Loja</a>
			</li>
			<li>
				<a href='admin_auction.php'>Leilões</a>
			</li>
			<?php
			$new = 0;
			$cat = 4;
			$threads = mysql_select_multi("SELECT `id`, `player_id` FROM `znote_forum_threads` WHERE `forum_id`='$cat' AND `closed`='0';");
			if ($threads !== false) {
				$staffs = mysql_select_multi("SELECT `CharacterID` AS `id` FROM `Characters` WHERE `AccountID` = 666666;");

				foreach ($threads as $thread) {
					$response = false;
					$posts = mysql_select_multi("SELECT `id`, `player_id` FROM `znote_forum_posts` WHERE `thread_id`='" . $thread['id'] . "';");
					if ($posts !== false && $staffs !== false) {
						foreach ($posts as $post) {
							foreach ($staffs as $staff) {
								if ($post['player_id'] == $staff['id']) $response = true;
							}
						}
					}
					if (!$response) $new++;
				}
			}
			?>
			<li>
				<a href='forum.php?cat=4'>Feedback: [<?php echo $new; ?>] novos</a>
			</li>
		</ul>
	</div>
</div>
