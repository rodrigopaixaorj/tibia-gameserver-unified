<?php
require_once 'engine/init.php';
protect_page();
admin_only($user_data);
include 'layout/overall/header.php';
?>
<h1>Auction Management</h1>
<div class="alert alert-info">
	<p><strong>Auction Panel:</strong></p>
	<p>The character auction system is inactive on the classic CipSoft Tibia 7.72 server.</p>
</div>
<?php
include 'layout/overall/footer.php';
?>
