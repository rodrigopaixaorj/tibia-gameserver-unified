<?php
require_once 'engine/init.php';

if ($config['require_login']['guildwars']) {
	protect_page();
}

include 'layout/overall/header.php';
?>
<h1>Guerras de Guildas</h1>
<div class="alert alert-info">
	<p><strong>Informação sobre Guerras:</strong></p>
	<p>Na versão clássica Tibia 7.72 (CipSoft), os confrontos e guerras entre guildas ocorrem diretamente no mundo de jogo através do sistema de PvP livre e disputas territoriais.</p>
	<p>Confira a lista de guildas ativas em nosso servidor na página de <a href="guilds.php">Guildas</a>.</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
