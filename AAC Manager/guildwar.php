<?php
require_once 'engine/init.php';

if ($config['require_login']['guildwars']) {
	protect_page();
}

include 'layout/overall/header.php';
?>
<h1>Guild Wars</h1>
<div class="alert alert-info">
	<p><strong>War Information:</strong></p>
	<p>In classic Tibia 7.72 (CipSoft), guild conflicts and wars take place directly in the game world through open PvP and territorial dominance.</p>
	<p>Check the list of active guilds on our server on the <a href="guilds.php">Guilds</a> page.</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
