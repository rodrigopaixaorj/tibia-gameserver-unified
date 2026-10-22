<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
?>

<h1>Downloads e Como Conectar</h1>
<p>Para jogar em nosso servidor, você precisará do cliente do Tibia 7.72 e de um IP Changer configurado.</p>

<div class="well" style="background: rgba(0,0,0,0.03); padding: 15px; border-radius: 4px; margin-bottom: 20px;">
	<h3>Links de Download</h3>
	<ul>
		<li><strong>Cliente Tibia 7.72 (Windows):</strong> <a href="<?php echo $config['client_download']; ?>" target="_blank" class="btn btn-primary btn-sm">Baixar Cliente 7.72</a></li>
		<li style="margin-top: 10px;"><strong>IP Changer (Universal):</strong> <a href="https://github.com/jo3bingham/tibia-ip-changer/releases/latest" target="_blank" class="btn btn-secondary btn-sm">Baixar IP Changer</a></li>
	</ul>
</div>

<h2>Passo a Passo para Jogar</h2>
<ol style="line-height: 1.8;">
	<li>Baixe e instale o cliente do <strong>Tibia 7.72</strong> no seu computador.</li>
	<li>Abra o <strong>IP Changer</strong>.</li>
	<li>No IP Changer, aponte para o executável do Tibia 7.72 (<code>Tibia.exe</code>).</li>
	<li>No campo de IP / Hostname, preencha: <code><?php echo $_SERVER['SERVER_NAME']; ?></code> e porta <code><?php echo $config['port']; ?></code>.</li>
	<li>Clique em <strong>Apply</strong> ou <strong>Change IP</strong>.</li>
	<li>Abra o cliente, clique em <strong>Enter Game</strong>, digite o número da sua conta e senha para jogar!</li>
</ol>

<p>Ainda não tem uma conta? <a href="register.php">Cadastre-se gratuitamente aqui</a>.</p>

<?php include 'layout/overall/footer.php'; ?>
