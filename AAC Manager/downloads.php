<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
?>

<h1>Downloads & How to Connect</h1>
<p>To play on our server, you will need the Tibia 7.72 client and a configured IP Changer.</p>

<div class="well" style="background: rgba(0,0,0,0.03); padding: 15px; border-radius: 4px; margin-bottom: 20px;">
	<h3>Download Links</h3>
	<ul>
		<li><strong>Tibia 7.72 Client (Windows):</strong> <a href="<?php echo $config['client_download']; ?>" target="_blank" class="btn btn-primary btn-sm">Download Client 7.72</a></li>
		<li style="margin-top: 10px;"><strong>IP Changer (Universal):</strong> <a href="https://github.com/jo3bingham/tibia-ip-changer/releases/latest" target="_blank" class="btn btn-secondary btn-sm">Download IP Changer</a></li>
	</ul>
</div>

<h2>Step-by-Step Connection Guide</h2>
<ol style="line-height: 1.8;">
	<li>Download and install the <strong>Tibia 7.72</strong> client on your computer.</li>
	<li>Open the <strong>IP Changer</strong>.</li>
	<li>In the IP Changer, locate and select the Tibia 7.72 executable (<code>Tibia.exe</code>).</li>
	<li>In the IP / Hostname field, enter: <code><?php echo $_SERVER['SERVER_NAME']; ?></code> and port <code><?php echo $config['port']; ?></code>.</li>
	<li>Click <strong>Apply</strong> or <strong>Change IP</strong>.</li>
	<li>Launch the client, click <strong>Enter Game</strong>, type your account number and password to log in!</li>
</ol>

<p>Don't have an account yet? <a href="register.php">Register for free here</a>.</p>

<?php include 'layout/overall/footer.php'; ?>
