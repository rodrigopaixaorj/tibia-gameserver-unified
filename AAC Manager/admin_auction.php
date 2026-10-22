<?php
require_once 'engine/init.php';
protect_page();
admin_only($user_data);
include 'layout/overall/header.php';
?>
<h1>Gerenciamento de Leilões</h1>
<div class="alert alert-info">
	<p><strong>Painel de Leilões:</strong></p>
	<p>O sistema de leilão de personagens está inativo no servidor clássico Tibia 7.72 (CipSoft).</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
