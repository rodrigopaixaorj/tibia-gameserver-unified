<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
protect_page();
admin_only($user_data);

// Processamento POST
if (!empty($_POST['option'])) {
	list($action, $id) = explode('!', sanitize($_POST['option']));

	// Deletar
	if ($action === 'd') {
		echo '<div class="alert alert-success">Notícia excluída com sucesso!</div>';
		mysql_delete("DELETE FROM `znote_news` WHERE `id`='$id';");
		$cache = new Cache('engine/cache/news');
		$news = fetchAllNews();
		$cache->setContent($news);
		$cache->save();
	}
	// Formulário de Nova Notícia
	if ($action === 'a') {
		$char_array = user_character_list($user_data['id']);
		?>
		<h2>Criar Nova Notícia</h2>
		<form action="admin_news.php" method="post">
			<input type="hidden" name="option" value="i!0">
			<ul class="form-list">
				<li>
					<label for="selected_char">Autor (Personagem):</label>
					<select name="selected_char" id="selected_char" class="form-control" required>
						<?php
						if ($char_array !== false) {
							foreach ($char_array as $char) {
								$cid = user_character_id($char['name']);
								echo '<option value="' . $cid . '">' . htmlspecialchars($char['name']) . '</option>';
							}
						}
						?>
					</select>
				</li>
				<li>
					<label for="title">Título da Notícia:</label>
					<input type="text" name="title" id="title" class="form-control" placeholder="Título" required>
				</li>
				<li>
					<label for="text">Conteúdo:</label>
					<textarea name="text" id="text" cols="75" rows="10" class="form-control" placeholder="Conteúdo da notícia (suporta tags BBCode)..." required></textarea>
				</li>
				<li>
					<input type="submit" value="Publicar Notícia" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	}
	// Inserir Notícia
	if ($action === 'i') {
		echo '<div class="alert alert-success">Notícia publicada com sucesso!</div>';
		$charid = (int)$_POST['selected_char'];
		$title = sanitize($_POST['title']);
		$text = sanitize($_POST['text']);
		$date = time();
		mysql_insert("INSERT INTO `znote_news` (`title`, `text`, `date`, `pid`) VALUES ('$title', '$text', '$date', '$charid');");
		$cache = new Cache('engine/cache/news');
		$news = fetchAllNews();
		$cache->setContent($news);
		$cache->save();
	}
	// Salvar Edição
	if ($action === 's') {
		echo '<div class="alert alert-success">Notícia atualizada com sucesso!</div>';
		$title = sanitize($_POST['title']);
		$text = sanitize($_POST['text']);
		mysql_update("UPDATE `znote_news` SET `title`='$title', `text`='$text' WHERE `id`='$id';");
		$cache = new Cache('engine/cache/news');
		$news = fetchAllNews();
		$cache->setContent($news);
		$cache->save();
	}
	// Formulário de Edição
	if ($action === 'e') {
		$news = fetchAllNews();
		$edit = array();
		if ($news) {
			foreach ($news as $n) {
				if ($n['id'] == $id) $edit = $n;
			}
		}
		?>
		<h2>Editar Notícia</h2>
		<form action="admin_news.php" method="post">
			<input type="hidden" name="option" value="s!<?php echo $id; ?>">
			<ul class="form-list">
				<li>
					<label for="title">Título:</label>
					<input type="text" name="title" id="title" value="<?php echo htmlspecialchars($edit['title']); ?>" class="form-control" required>
				</li>
				<li>
					<label for="text">Conteúdo:</label>
					<textarea name="text" id="text" cols="75" rows="10" class="form-control" required><?php echo htmlspecialchars($edit['text']); ?></textarea>
				</li>
				<li>
					<input type="submit" value="Salvar Alterações" class="btn btn-success">
				</li>
			</ul>
		</form>
		<?php
	}
}

?>

<h1>Gerenciamento de Notícias</h1>
<form action="admin_news.php" method="post" style="margin-bottom: 20px;">
	<input type="hidden" name="option" value="a!0">
	<input type="submit" value="Adicionar Nova Notícia" class="btn btn-primary">
</form>

<?php
$news = fetchAllNews();
if ($news !== false && !empty($news)) {
	?>
	<div class="table-responsive">
		<table id="news" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Data</th>
					<th>Autor</th>
					<th>Título</th>
					<th style="width: 80px;">Editar</th>
					<th style="width: 80px;">Excluir</th>
				</tr>
			</thead>
			<tbody>
				<?php foreach ($news as $n): ?>
					<tr>
						<td><?php echo getClock($n['date'], true); ?></td>
						<td><a href="characterprofile.php?name=<?php echo urlencode($n['name']); ?>"><?php echo htmlspecialchars($n['name']); ?></a></td>
						<td><?php echo htmlspecialchars($n['title']); ?></td>
						<td>
							<form action="admin_news.php" method="post">
								<input type="hidden" name="option" value="e!<?php echo $n['id']; ?>">
								<input type="submit" value="Editar" class="btn btn-secondary btn-sm">
							</form>
						</td>
						<td>
							<form action="admin_news.php" method="post">
								<input type="hidden" name="option" value="d!<?php echo $n['id']; ?>">
								<input type="submit" value="Excluir" class="btn btn-danger btn-sm" onclick="return confirm('Deseja excluir esta notícia?');">
							</form>
						</td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
	<?php
} else {
	echo '<p>Nenhuma notícia cadastrada ainda.</p>';
}

include 'layout/overall/footer.php';
?>
