<?php
$time = time();
if (!isset($version)) $version = '1.6';

if (!function_exists("elapsedTime")) {
	function elapsedTime($l_start = false, $l_time = false) {
		if ($l_start === false) global $l_start;
		if ($l_time === false) global $l_time;

		$l_time = microtime(true);
		return round(($l_time - $l_start), 4);
	}
}

$install = "
<h2>Instalação / Configuração do Banco de Dados:</h2>
<ol>
	<li>
		<p>
			Certifique-se de que o banco de dados do <b>tibia-gameserver-unified</b> foi importado (arquivo <code>schema_mysql.sql</code> ou <code>tibia_mysql_full.sql</code>).
		</p>
	</li>
	<li>Importe o <a href='/engine/database/znote_schema.sql'>Znote AAC schema</a> para o mesmo banco de dados no MySQL.</li>
	<li>
		<p>
			Verifique o arquivo <code>config.php</code> com os dados corretos de conexão (host, porta, usuário, senha e banco de dados).
		</p>
	</li>
</ol>
";

$sqlPort = isset($config['sqlPort']) ? (int)$config['sqlPort'] : 3306;
$connect = new mysqli($config['sqlHost'], $config['sqlUser'], $config['sqlPassword'], $config['sqlDatabase'], $sqlPort);

if ($connect->connect_errno) {
	die("Falha ao conectar no MySQL: (" . $connect->connect_errno . ") " . $connect->connect_error . $install);
}

$connect->set_charset("utf8mb4");

function mysql_znote_escape_string($escapestr) {
	global $connect;
	return mysqli_real_escape_string($connect, (string)$escapestr);
}

// Select single row from database
function mysql_select_single($query) {
	global $connect;
	global $aacQueries;
	$aacQueries++;

	global $accQueriesData;
	$accQueriesData[] = "[" . elapsedTime() . "] " . $query;
	$result = mysqli_query($connect, $query) or die(var_dump($query)."<br>(query - <font color='red'>SQL error</font>) <br>Type: <b>select_single</b><br><br>".mysqli_error($connect));
	$row = mysqli_fetch_assoc($result);
	return !empty($row) ? $row : false;
}

// Selecting multiple rows from database.
function mysql_select_multi($query){
	global $connect;
	global $aacQueries;
	$aacQueries++;
	global $accQueriesData;
	$accQueriesData[] = "[" . elapsedTime() . "] " . $query;
	$array = array();
	$results = mysqli_query($connect, $query) or die(var_dump($query)."<br>(query - <font color='red'>SQL error</font>) <br>Type: <b>select_multi</b><br><br>".mysqli_error($connect));
	while($row = mysqli_fetch_assoc($results)) {
		$array[] = $row;
	}
	return !empty($array) ? $array : false;
}

//////
// Query database without expecting returned results

// - mysql update
function mysql_update($query){ voidQuery($query); }
// mysql insert
function mysql_insert($query){ voidQuery($query); }
// mysql delete
function mysql_delete($query){ voidQuery($query); }
// Send a void query
function voidQuery($query) {
	global $connect;
	global $aacQueries;
	$aacQueries++;
	global $accQueriesData;
	$accQueriesData[] = "[" . elapsedTime() . "] " . $query;
	mysqli_query($connect, $query) or die(var_dump($query)."<br>(query - <font color='red'>SQL error</font>) <br>Type: <b>voidQuery</b><br><br>".mysqli_error($connect));
}
?>
