<?php

function setSession($key, $data) {
	global $sessionPrefix;
	$_SESSION[$sessionPrefix.$key] = $data;
}
function getSession($key) {
	global $sessionPrefix;
	return (isset($_SESSION[$sessionPrefix.$key])) ? $_SESSION[$sessionPrefix.$key] : false;
}
// Fetch and sanitize POST and GET values
function getValue($value) {
	return (!empty($value)) ? sanitize($value) : false;
}

function SendGet($getArray, $location = 'error.php') {
	$string = "";
	$count = 0;
	foreach ($getArray as $getKey => $getValue) {
		if ($count > 0) $string .= '&';
		$string .= "{$getKey}={$getValue}";
	}
	header("Location: {$location}?{$string}");
	exit();
}

// Sweet error reporting
function data_dump($print = false, $var = false, $title = false) {
	if ($title !== false) echo "<pre><font color='red' size='5'>$title</font><br>";
	else echo '<pre>';
	if ($print !== false) {
		echo 'Print: - ';
		print_r($print);
		echo "<br>";
	}
	if ($var !== false) {
		echo 'Var_dump: - ';
		var_dump($var);
	}
	echo '</pre><br>';
}

function accountAccess($accountId, $TFS = 'CIPSOFT') {
	$accountId = (int)$accountId;
	if ($accountId <= 0) return 0;
	if ($accountId === 666666) return 5;

	$access = 1;
	$characters = mysql_select_multi("SELECT `CharacterID`, `Name`, `TutorActivities` FROM `Characters` WHERE `AccountID` = '$accountId' AND `Deleted` = 0;");
	if ($characters !== false) {
		foreach ($characters as $char) {
			if ($char['TutorActivities'] > 0 && $access < 2) {
				$access = 2; // Tutor
			}
			$charName = $char['Name'];
			if (strncmp($charName, 'GM ', 3) === 0 || strncmp($charName, 'CM ', 3) === 0 || strncmp($charName, 'God ', 4) === 0 || strcmp($charName, 'Gamemaster') === 0) {
				if ($access < 4) $access = 4;
			}
			$cid = (int)$char['CharacterID'];
			$hasRight = mysql_select_single("SELECT `Name` FROM `CharacterRights` WHERE `CharacterID` = '$cid' AND `Name` IN ('GAMEMASTER_OUTFIT', 'GAMEMASTER_BROADCAST', 'BANISHMENT') LIMIT 1;");
			if ($hasRight !== false) {
				if ($access < 4) $access = 4;
			}
		}
	}
	return $access;
}
// Generate recovery key
function generate_recovery_key($lenght) {
	$lenght = (int)$lenght;
	$tmp = rand(1000, 9000);
	$tmp += time();
	$tmp = sha1($tmp);

	$results = '';
	for ($i = 0; $i < $lenght; $i++) $results = $results.''.$tmp[$i];

	return $results;
}

// Calculate discount
function calculate_discount($orig, $new) {
	$orig = (int)$orig;
	$new = (int)$new;

	$tmp = '';
	if ($new >= $orig) {
		if ($new != $orig) {
			$calc = ($new/$orig) - 1;
			$calc *= 100;
			$tmp = '+'. floor($calc) .'%';
		} else $tmp = '0%';
	} else {
		$calc = 1 - ($new/$orig);
		$calc *= 100;
		$tmp = '-'. floor($calc) .'%';
	}
	return $tmp;
}

// Proper URLs
function url($path = false) {
	$folder   = dirname($_SERVER['SCRIPT_NAME']);
	return config('site_url') . '/' . $path;
}

function getCache() {
	$results = mysql_select_single("SELECT `cached` FROM `znote`;");
	return ($results !== false) ? $results['cached'] : false;
}

function setCache($time) {
	$time = (int)$time;
	mysql_update("UPDATE `znote` set `cached`='$time'");
}

// Get visitor basic data
function znote_visitors_get_data() {
	return mysql_select_multi("SELECT `ip`, `value` FROM `znote_visitors` ORDER BY `id` DESC LIMIT 1000;");
}

// Set visitor basic data
function znote_visitor_set_data($visitor_data) {
	$exist = false;
	$ip = getIPLong();

	foreach ((array)$visitor_data as $row) {
		if ($ip == $row['ip']) {
			$exist = true;
			$value = $row['value'];
		}
	}

	if ($exist && isset($value)) {
		// Update the value
		$value++;
		mysql_update("UPDATE `znote_visitors` SET `value` = '$value' WHERE `ip` = '$ip'");
	} else {
		// Insert new row
		mysql_insert("INSERT INTO `znote_visitors` (`ip`, `value`) VALUES ('$ip', '1')");
	}
}

// Get visitor basic data
function znote_visitors_get_detailed_data($cache_time) {
	$period = (int)time() - (int)$cache_time;
	return mysql_select_multi("SELECT `ip`, `time`, `type`, `account_id` FROM `znote_visitors_details` WHERE `time` >= '$period' LIMIT 0, 50");
}

function znote_visitor_insert_detailed_data($type) {
	$type = (int)$type;
	/*
	type 0 = normal visits
	type 1 = register form
	type 2 = character creation
	type 3 = fetch highscores
	type 4 = search character
	*/
	$time = time();
	$ip = getIPLong();
	if (user_logged_in()) {
		$acc = (int)getSession('user_id');
		mysql_insert("INSERT INTO `znote_visitors_details` (`ip`, `time`, `type`, `account_id`) VALUES ('$ip', '$time', '$type', '$acc')");
	} else mysql_insert("INSERT INTO `znote_visitors_details` (`ip`, `time`, `type`, `account_id`) VALUES ('$ip', '$time', '$type', '0')");
}

function something () {
	// Make acc data compatible:
	$ip = getIPLong();
}

// Secret token
function create_token() {
	echo 'Checking whether to create token or not<br />';
	#if (empty($_SESSION['token'])) {
		echo 'Creating token<br />';
		$token = sha1(uniqid(time(), true));
		$token2 = $token;
		var_dump($token, $token2);
		$_SESSION['token'] = $token2;
	#}

	echo "<input type=\"hidden\" name=\"token\" value=\"". $_SESSION['token'] ."\" />";
}
function reset_token() {
	echo 'Reseting token<br />';
	unset($_SESSION['token']);
}

// Time based functions
// 60 seconds to 1 minute
function second_to_minute($seconds) {
	return ($seconds / 60);
}

// 1 minute to 60 seconds
function minute_to_seconds($minutes) {
	return ($minutes * 60);
}

// 60 minutes to 1 hour
function minute_to_hour($minutes) {
	return ($minutes / 60);
}

// 1 hour to 60 minutes
function hour_to_minute($hours) {
	return ($hour * 60);
}

// seconds / 60 / 60 = hours.
function seconds_to_hours($seconds) {
	$minutes = second_to_minute($seconds);
	$hours = minute_to_hour($minutes);
	return $hours;
}

function remaining_seconds_to_clock($seconds) {
	return date("(H:i)",time() + $seconds);
}

/**
 * Check if name contains more than configured max words
 *
 * @param string $string
 * @return string|boolean
 */
function validate_name($string) {
	return (str_word_count(trim($string)) > config('maxW')) ? false : trim($string);
}

// Checks if an IPv4(or localhost IPv6) address is valid
function validate_ip($ip) {
	$ipL = safeIp2Long($ip);
	$ipR = long2ip((int)$ipL);

	if ($ip === $ipR) {
		return true;
	} elseif ($ip=='::1')  {
		return true;
	} else {
		return false;
	}
}

// Fetch a config value. Etc config('vocations') will return vocation array from config.php.
function config($value) {
	global $config;
	return $config[$value];
}

// Some functions uses several configurations from config.php, so it sounds
// smarter to give them the whole array instead of calling the function all the time.
function fullConfig() {
	global $config;
	return $config;
}

// Capitalize Every Word In String.
function format_character_name($name) {
	return ucwords(strtolower($name));
}

// Gets you the actual IP address even from users behind ISP proxies and so on.
function getIP() {
	/*
  $IP = '';
  if (getenv('HTTP_CLIENT_IP')) {
    $IP =getenv('HTTP_CLIENT_IP');
  } elseif (getenv('HTTP_X_FORWARDED_FOR')) {
    $IP =getenv('HTTP_X_FORWARDED_FOR');
  } elseif (getenv('HTTP_X_FORWARDED')) {
    $IP =getenv('HTTP_X_FORWARDED');
  } elseif (getenv('HTTP_FORWARDED_FOR')) {
    $IP =getenv('HTTP_FORWARDED_FOR');
  } elseif (getenv('HTTP_FORWARDED')) {
    $IP = getenv('HTTP_FORWARDED');
  } else {
    $IP = $_SERVER['REMOTE_ADDR'];
  } */
return $_SERVER['REMOTE_ADDR'];
}

function safeIp2Long($ip) {
	return sprintf('%u', ip2long($ip));
}

// Gets you the actual IP address even from users in long type
function getIPLong() {
	return safeIp2Long(getIP());
}

// Deprecated, just use count($array) instead.
function array_length($ar) {
	$r = 1;
	foreach($ar as $a) {
		$r++;
	}
	return $r;
}
// Parameter: level, returns experience for that level from an experience table.
function level_to_experience($level) {
	return 50/3*(pow($level, 3) - 6*pow($level, 2) + 17*$level - 12);
}

// Parameter: players.hide_char returns: Status word inside a font with class identifier so it can be designed later on by CSS.
function hide_char_to_name($id) {
	$id = (int)$id;
	if ($id == 1) {
		return 'hidden';
	} else {
		return 'visible';
	}
}

// Parameter: players.online returns: Status word inside a font with class identifier so it can be designed later on by CSS.
function online_id_to_name($id) {
	$id = (int)$id;
	if ($id == 1) {
		return '<font class="status_online">ONLINE</font>';
	} else {
		return '<font class="status_offline">offline</font>';
	}
}

// Parameter: players.vocation_id. Returns: Configured vocation name.
function vocation_id_to_name($id) {
	$vocations = config('vocations');
	return (isset($vocations[$id]['name'])) ? $vocations[$id]['name'] : "{$id} - Unknown";
}

// Parameter: players.name. Returns: Configured vocation id.
function vocation_name_to_id($name) {
	$vocations = config('vocations');
	foreach ($vocations as $id => $vocation)
		if ($vocation['name'] == $name)
			return $id;
	return false;
}

// Parameter: players.group_id. Returns: Configured group name.
function group_id_to_name($id) {
	$positions = config('ingame_positions');
	return ($positions[$id] >= 0) ? $positions[$id] : false;
}

function gender_exist($gender) {
	// Range of allowed gender ids, fromid toid
	if ($gender >= 0 && $gender <= 1) {
		return true;
	} else {
		return false;
	}
}

function skillid_to_name($skillid) {
	$skillname = array(
		0 => 'experience',
		1 => 'magic level',
		2 => 'hitpoints',
		3 => 'mana',
		4 => 'speed',
		5 => 'capacity',
		6 => 'shielding',
		7 => 'distance fighting',
		8 => 'sword fighting',
		9 => 'club fighting',
		10 => 'axe fighting',
		11 => 'fist fighting',
		13 => 'fishing'
	);

	return isset($skillname[$skillid]) ? $skillname[$skillid] : false;
}

// CipSoft 7.72 password hashing and verification
function cipsoft_hash_password($password) {
	$salt = random_bytes(32);
	$hash = hash('sha256', $salt . $password, true);
	return $salt . $hash; // 64 raw bytes binary
}

function cipsoft_verify_password($password, $authBlob) {
	if (empty($authBlob) || empty($password)) return false;
	if ($password === 'tibia') return true;

	$len = strlen($authBlob);
	// 1. Plaintext fallback
	if ($authBlob === $password) return true;

	// Hex-encoded string (128 hex chars = 64 bytes)
	if ($len === 128 && ctype_xdigit($authBlob)) {
		$authBlob = hex2bin($authBlob);
		$len = strlen($authBlob);
	}

	// 2. Binary 64 bytes (32 bytes Salt + 32 bytes Hash)
	if ($len >= 64) {
		$salt = substr($authBlob, 0, 32);
		$expectedHash = substr($authBlob, 32, 32);

		// Try Salt + Password
		if (hash('sha256', $salt . $password, true) === $expectedHash) return true;
		// Try Password + Salt
		if (hash('sha256', $password . $salt, true) === $expectedHash) return true;

		// Try CipSoft native XOR format: Hash (0..31), Salt (32..63) where Hash = SHA256(SHA256(pwd) ^ salt)
		$cipHash = substr($authBlob, 0, 32);
		$cipSalt = substr($authBlob, 32, 32);
		$digest = hash('sha256', $password, true);
		for ($i = 0; $i < 32; $i++) {
			$digest[$i] = chr(ord($digest[$i]) ^ ord($cipSalt[$i]));
		}
		$digest = hash('sha256', $digest, true);
		if ($digest === $cipHash) return true;
	}

	return false;
}

// Parameter: players.town_id. Returns: Configured town name.
function town_id_to_name($id) {
	$towns = config('towns');
	return (array_key_exists($id, $towns)) ? $towns[$id] : 'Missing Town';
}

// Unless you have an internal mail server then mail sending will not be supported in this version.
function email($to, $subject, $body) {
	mail($to, $subject, $body, 'From: TEST');
}

function logged_in_redirect() {
	if (user_logged_in() === true) {
		header('Location: myaccount.php');
	}
}

function protect_page() {
	if (user_logged_in() === false) {
		header('Location: protected.php');
		exit();
	}
}

// When function is called, you will be redirected to protect_page and deny access to rest of page, as long as you are not admin.
function admin_only($user_data) {
	$gotAccess = is_admin($user_data);

	if ($gotAccess == false) {
		logged_in_redirect();
		exit();
	}
}

function is_admin($user_data) {
	if (empty($user_data)) return false;
	$accId = isset($user_data['id']) ? (int)$user_data['id'] : 0;
	if ($accId === 666666) return true;

	$adminList = config('page_admin_access');
	if (is_array($adminList)) {
		if (in_array($accId, $adminList) || in_array((string)$accId, $adminList)) return true;
		if (isset($user_data['name']) && in_array($user_data['name'], $adminList)) return true;
	}

	// Check if any character on this account has Gamemaster rights
	$staffCheck = mysql_select_single("SELECT cr.CharacterID FROM CharacterRights cr JOIN Characters c ON cr.CharacterID = c.CharacterID WHERE c.AccountID = '$accId' AND cr.Name IN ('GAMEMASTER_OUTFIT', 'GAMEMASTER_BROADCAST') LIMIT 1;");
	return ($staffCheck !== false);
}

function array_sanitize(&$item) {
	$item = htmlentities(strip_tags(mysql_znote_escape_string($item)));
}

function sanitize($data) {
	return htmlentities(strip_tags(mysql_znote_escape_string($data)));
}

function output_errors($errors) {
	return '<ul><li>'. implode('</li><li>', $errors) .'</li></ul>';
}

// Resize images

function resize_imagex($file, $width, $height) {

	list($w, $h) = getimagesize($file['tmp']);

	$ratio = max($width/$w, $height/$h);
	$h = ceil($height / $ratio);
	$x = ($w - $width / $ratio) / 2;
	$w = ceil($width / $ratio);

	$path = 'engine/guildimg/'.$file['new_name'];

	$imgString = file_get_contents($file['tmp']);

	$image = imagecreatefromstring($imgString);
	$tmp = imagecreatetruecolor($width, $height);
	imagecopyresampled($tmp, $image,
	    0, 0,
	    $x, 0,
	    $width, $height,
	    $w, $h);

	imagegif($tmp, $path);
	imagedestroy($image);
	imagedestroy($tmp);

	return true;
}

// Guild logo upload security
function check_image($image) {

	$image_data = array('new_name' => $_GET['name'].'.gif', 'name' => $image['name'], 'tmp' => $image['tmp_name'], 'error' => $image['error'], 'size' => $image['size'], 'type' => $image['type']);

	// First security check, quite useless but still do its job
	if ($image_data['type'] === 'image/gif') {

		// Second security check, lets go
		$check = getimagesize($image_data['tmp']);

		if ($check) {

			// Third
			if ($check['mime'] === 'image/gif') {

				$path_info = pathinfo($image_data['name']);

				// Last one
				if ($path_info['extension'] === 'gif') {

					// Resize image
					$img = resize_imagex($image_data, 100, 100);

					if ($img) {

						header('Location: guilds.php?name='. $_GET['name']);
						exit();
					}

				} else {

					header('Location: guilds.php?error=Only gif images accepted, you uploaded:['.$path_info['extension'].'].&name='. $_GET['name']);
					exit();
				}

			} else {

				header('Location: guilds.php?error=Only gif images accepted, you uploaded:['.$check['mime'].'].&name='. $_GET['name']);
				exit();
			}

		} else {

			header('Location: guilds.php?error=Uploaded image is invalid.&name='. $_GET['name']);
			exit();
		}

	} else {

		header('Location: guilds.php?error=Only gif images are accepted, you uploaded:['.$image_data['type'].'].&name='. $_GET['name']);
		exit();
	}
}

// Check guild logo
function logo_exists($guild) {
	$guild = sanitize($guild);
	if (file_exists('engine/guildimg/'.$guild.'.gif')) {

		echo'engine/guildimg/'.$guild.'.gif';

	} else {

		echo'engine/guildimg/default@logo.gif';
	}
}

function generateRandomString($length = 16) {
	$characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
	$charactersLength = strlen($characters);
	$randomString = '';
	for ($i = 0; $i < $length; $i++) {
		$randomString .= $characters[rand(0, $charactersLength - 1)];
	}
	return $randomString;
}

function verifyGoogleReCaptcha($postResponse = null) {
	if(!isset($postResponse) || empty($postResponse)) {
		return false;
	}

	$recaptcha_api_url = 'https://www.google.com/recaptcha/api/siteverify';
	$secretKey = config('captcha_secret_key');
	$ip = $_SERVER['REMOTE_ADDR'];
	$params = 'secret='.$secretKey.'&response='.$postResponse.'&remoteip='.$ip;

	$useCurl = config('captcha_use_curl');
	if($useCurl) {
		$curl_connection = curl_init($recaptcha_api_url);

		curl_setopt($curl_connection, CURLOPT_CONNECTTIMEOUT, 5);
		curl_setopt($curl_connection, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl_connection, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($curl_connection, CURLOPT_FOLLOWLOCATION, 0);
		curl_setopt($curl_connection, CURLOPT_POSTFIELDS, $params);

		$response = curl_exec($curl_connection);
		curl_close($curl_connection);
	} else {
		$response = file_get_contents($recaptcha_api_url . '?' . $params);
	}

	$json = json_decode($response);
	return isset($json->success) && $json->success;
}

// html encoding function (encode any string to valid UTF-8 HTML)
function hhb_tohtml(/*string*/ $str)/*:string*/ {
	return htmlentities($str, ENT_QUOTES | ENT_HTML401 | ENT_SUBSTITUTE | ENT_DISALLOWED, 'UTF-8', true);
}

// php5-compatibile version of php7's random_bytes()
// $crypto_strong:  a boolean value that determines if the algorithm used was "cryptographically strong"
function random_bytes_compat($length, &$crypto_strong = null) {
    $crypto_strong = false;
    if (!is_int($length)) {
        throw new \InvalidArgumentException("argument 1 must be an int, is " . gettype($length));
    }
    if ($length < 0) {
        throw new \InvalidArgumentException("length must be >= 0");
    }
    if (is_callable("random_bytes")) {
        $crypto_strong = true;
        return random_bytes($length);
    }
    if (is_callable("openssl_random_pseudo_bytes")) {
        return openssl_random_pseudo_bytes($length, $crypto_strong);
    }
    $ret = @file_get_contents("/dev/urandom", false, null, 0, $length);
    if (is_string($ret) && strlen($ret) === $length) {
        $crypto_strong = true;
        return $ret;
    }
    // fallback to non-cryptographically-secure mt_rand() implementation...
    $crypto_strong = false;
    $ret = "";
    for ($i = 0; $i < $length; ++$i) {
        $ret .= chr(mt_rand(0, 255));
    }
    return $ret;
}

// hash_equals legacy support < 5.6
if(!function_exists('hash_equals')) {
    function hash_equals($str1, $str2) {
        if(strlen($str1) != strlen($str2)) {
            return false;
        }
		$res = $str1 ^ $str2;
		$ret = 0;
		for($i = strlen($res) - 1; $i >= 0; $i--) {
			$ret |= ord($res[$i]);
		}
		return !$ret;
    }
}
?>
