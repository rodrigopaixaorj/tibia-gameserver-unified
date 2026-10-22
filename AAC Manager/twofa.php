<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';
?>
<h1>Autenticação de Dois Fatores (2FA)</h1>
<div class="alert alert-info">
	<p><strong>Aviso de Compatibilidade:</strong></p>
	<p>O cliente de jogo CipSoft Tibia 7.72 utiliza o protocolo clássico com número de conta e senha, não possuindo suporte nativo no cliente para tokens 2FA.</p>
	<p>Para sua segurança, utilize sempre senhas fortes e guarde a sua Chave de Recuperação (Recovery Key) em local seguro.</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
