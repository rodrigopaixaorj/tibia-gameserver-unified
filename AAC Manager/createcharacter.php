<?php
require_once 'engine/init.php';
protect_page();
include 'layout/overall/header.php';

$errors = array();

if (!empty($_POST)) {
	$required_fields = array('name', 'selected_town');
	foreach ($required_fields as $field) {
		if (empty($_POST[$field])) {
			$errors[] = 'Você precisa preencher todos os campos.';
			break;
		}
	}

	if (empty($errors)) {
		if (!Token::isValid($_POST['token'])) {
			$errors[] = 'Token de sessão inválido.';
		}

		$name = validate_name($_POST['name']);
		if ($name === false) {
			$errors[] = 'O nome do personagem não pode ter mais de 2 palavras.';
		} else {
			if (user_character_exist($name) !== false) {
				$errors[] = 'Desculpe, já existe um personagem com este nome.';
			}
			if (!preg_match("/^[a-zA-Z ]+$/", $name)) {
				$errors[] = 'O nome pode conter apenas letras de a-z e espaços.';
			}
			if (strlen($name) < $config['minL'] || strlen($name) > $config['maxL']) {
				$errors[] = 'O nome deve ter entre ' . $config['minL'] . ' e ' . $config['maxL'] . ' caracteres.';
			}

			$resname = explode(" ", $name);
			foreach ($resname as $res) {
				if (in_array(strtolower($res), $config['invalidNameTags'])) {
					$errors[] = 'O nome escolhido contém uma palavra proibida.';
				}
				if (strlen($res) == 1) {
					$errors[] = 'Palavras de apenas uma letra não são permitidas no nome.';
				}
			}

			if (in_array(strtolower($name), $config['creatureNameTags'])) {
				$errors[] = 'O nome escolhido é o nome de uma criatura do jogo.';
			}

			$selected_vocation = (int)$_POST['selected_vocation'];
			if (!in_array($selected_vocation, $config['available_vocations'])) {
				$errors[] = 'Vocação inválida selecionada.';
			}

			$selected_town = (int)$_POST['selected_town'];
			if (!in_array($selected_town, $config['available_towns'])) {
				$errors[] = 'Cidade inválida selecionada.';
			}

			$selected_gender = (int)$_POST['selected_gender'];
			if (!in_array($selected_gender, array(0, 1))) {
				$errors[] = 'Sexo inválido selecionado.';
			}

			$char_count = user_character_list_count($session_user_id);
			if ($char_count >= $config['max_characters'] && !is_admin($user_data)) {
				$errors[] = 'Sua conta já atingiu o limite máximo de ' . $config['max_characters'] . ' personagens.';
			}
		}
	}
}

?>
<h1>Criar Personagem</h1>
<?php
if (isset($_GET['success'])) {
	echo '<div class="alert alert-success"><h3>Parabéns!</h3><p>Seu personagem foi criado com sucesso. Nos vemos no jogo!</p><p><a href="myaccount.php" class="btn btn-primary">Voltar para Minha Conta</a></p></div>';
} else {
	if (!empty($_POST) && empty($errors)) {
		if ($config['log_ip']) {
			znote_visitor_insert_detailed_data(2);
		}

		$character_data = array(
			'name'       => format_character_name($_POST['name']),
			'account_id' => $session_user_id,
			'vocation'   => (int)$_POST['selected_vocation'],
			'town_id'    => (int)$_POST['selected_town'],
			'sex'        => (int)$_POST['selected_gender'],
			'lastip'     => getIPLong(),
			'created'    => time()
		);

		user_create_character($character_data);
		header('Location: createcharacter.php?success');
		exit();
	} elseif (!empty($errors)) {
		echo output_errors($errors);
	}
	?>
	<form action="createcharacter.php" method="post">
		<ul class="form-list">
			<li>
				<label for="name">Nome do Personagem:</label>
				<input type="text" name="name" id="name" placeholder="Ex: Sir Lancelot" required autofocus value="<?php if (isset($_POST['name'])) echo htmlspecialchars($_POST['name']); ?>">
			</li>
			<li>
				<label for="selected_vocation">Vocação:</label>
				<select name="selected_vocation" id="selected_vocation" required>
					<?php foreach ($config['available_vocations'] as $vid): ?>
					<option value="<?php echo $vid; ?>" <?php if (isset($_POST['selected_vocation']) && $_POST['selected_vocation'] == $vid) echo 'selected'; ?>>
						<?php echo htmlspecialchars(vocation_id_to_name($vid)); ?>
					</option>
					<?php endforeach; ?>
				</select>
			</li>
			<li>
				<label for="selected_gender">Sexo:</label>
				<select name="selected_gender" id="selected_gender" required>
					<option value="1" <?php if (isset($_POST['selected_gender']) && $_POST['selected_gender'] == 1) echo 'selected'; ?>>Masculino (Male)</option>
					<option value="0" <?php if (isset($_POST['selected_gender']) && $_POST['selected_gender'] == 0) echo 'selected'; ?>>Feminino (Female)</option>
				</select>
			</li>
			<li>
				<label for="selected_town">Cidade Inicial:</label>
				<select name="selected_town" id="selected_town" required>
					<?php foreach ($config['available_towns'] as $tid): ?>
					<option value="<?php echo $tid; ?>" <?php if (isset($_POST['selected_town']) && $_POST['selected_town'] == $tid) echo 'selected'; ?>>
						<?php echo htmlspecialchars(town_id_to_name($tid)); ?>
					</option>
					<?php endforeach; ?>
				</select>
			</li>
			<input type="hidden" name="token" value="<?php echo Token::generate(); ?>">
			<li>
				<input type="submit" value="Criar Personagem" class="btn btn-primary">
			</li>
		</ul>
	</form>
	<?php
}

include 'layout/overall/footer.php';
?>
