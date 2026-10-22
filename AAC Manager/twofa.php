<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';
?>
<h1>Two-Factor Authentication (2FA)</h1>
<div class="alert alert-info">
	<p><strong>Compatibility Notice:</strong></p>
	<p>The CipSoft Tibia 7.72 game client uses the classic protocol with account number and password, and does not natively support 2FA tokens.</p>
	<p>For your security, always use strong passwords and keep your Recovery Key in a safe place.</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
