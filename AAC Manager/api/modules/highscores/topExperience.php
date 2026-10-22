<?php require_once '../../module.php';

// Configure module version number
$response['version']['module'] = 1;

// Fetch number of rows
$rows = (isset($_GET['rows']) && (int)$_GET['rows'] > 0) ? (int)getValue($_GET['rows']) : 10;

// Show which configuration is used
$response['config']['rows'] = $rows;

// Fetch top players
$players = mysql_select_multi("SELECT `p`.`Name` AS `name`, `p`.`Level` AS `level`, `p`.`Experience` AS `experience`, `p`.`Profession` AS `vocation`, `p`.`LastLogin` AS `lastlogin`, COALESCE(`z`.`created`, 0) AS `created` FROM `Characters` AS `p` LEFT JOIN `znote_players` AS `z` ON `p`.`CharacterID` = `z`.`player_id` LEFT JOIN `PlayerState` AS `ps` ON `p`.`CharacterID` = `ps`.`CharacterID` WHERE `p`.`Deleted` = 0 AND COALESCE(`ps`.`AccountAccess`, 0) < 2 ORDER BY `p`.`Experience` DESC LIMIT $rows;");
if (!empty($players)) {
	for ($i = 0; $i < count($players); $i++) {
		$players[$i]['vocation_name'] = isset($config['vocations'][$players[$i]['vocation']]) ? $config['vocations'][$players[$i]['vocation']] : 'None';
	}
} else {
	$players = array();
}
$response['data']['players'] = $players;


SendResponse($response);
?>