<?php
require_once 'engine/init.php';
include 'layout/overall/header.php';
protect_page();
admin_only($user_data);

// Process POST
if (!empty($_POST['option'])) {
	list($action, $id) = explode('!', sanitize($_POST['option']));

	// Delete
	if ($action === 'd') {
		echo '<div class="alert alert-success">News deleted successfully!</div>';
		mysql_delete("DELETE FROM `znote_news` WHERE `id`='$id';");
		$cache = new Cache('engine/cache/news');
		$news = fetchAllNews();
		$cache->setContent($news);
		$cache->save();
	}
	// Add News Form
	if ($action === 'a') {
		$char_array = user_character_list($user_data['id']);
		?>
		<h2>Create News Article</h2>
		<form action="admin_news.php" method="post">
			<input type="hidden" name="option" value="i!0">
			<ul class="form-list">
				<li>
					<label for="selected_char">Author (Character):</label>
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
					<label for="title">News Title:</label>
					<input type="text" name="title" id="title" class="form-control" placeholder="Title" required>
				</li>
				<li>
					<label for="text">Content:</label>
					<textarea name="text" id="text" cols="75" rows="10" class="form-control" placeholder="News content (supports BBCode tags)..." required></textarea>
				</li>
				<li>
					<input type="submit" value="Publish News" class="btn btn-primary">
				</li>
			</ul>
		</form>
		<?php
	}
	// Insert News
	if ($action === 'i') {
		echo '<div class="alert alert-success">News published successfully!</div>';
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
	// Save Edit
	if ($action === 's') {
		echo '<div class="alert alert-success">News updated successfully!';
		$title = sanitize($_POST['title']);
		$text = sanitize($_POST['text']);
		mysql_update("UPDATE `znote_news` SET `title`='$title', `text`='$text' WHERE `id`='$id';");
		$cache = new Cache('engine/cache/news');
		$news = fetchAllNews();
		$cache->setContent($news);
		$cache->save();
	}
	// Edit Form
	if ($action === 'e') {
		$news = fetchAllNews();
		$edit = array();
		if ($news) {
			foreach ($news as $n) {
				if ($n['id'] == $id) $edit = $n;
			}
		}
		?>
		<h2>Edit News Article</h2>
		<form action="admin_news.php" method="post">
			<input type="hidden" name="option" value="s!<?php echo $id; ?>">
			<ul class="form-list">
				<li>
					<label for="title">Title:</label>
					<input type="text" name="title" id="title" value="<?php echo htmlspecialchars($edit['title']); ?>" class="form-control" required>
				</li>
				<li>
					<label for="text">Content:</label>
					<textarea name="text" id="text" cols="75" rows="10" class="form-control" required><?php echo htmlspecialchars($edit['text']); ?></textarea>
				</li>
				<li>
					<input type="submit" value="Save Changes" class="btn btn-success">
				</li>
			</ul>
		</form>
		<?php
	}
}

?>

<h1>News Management</h1>
<form action="admin_news.php" method="post" style="margin-bottom: 20px;">
	<input type="hidden" name="option" value="a!0">
	<input type="submit" value="Add New Article" class="btn btn-primary">
</form>

<?php
$news = fetchAllNews();
if ($news !== false && !empty($news)) {
	?>
	<div class="table-responsive">
		<table id="news" class="table table-striped table-hover">
			<thead>
				<tr class="yellow">
					<th>Date</th>
					<th>Author</th>
					<th>Title</th>
					<th style="width: 80px;">Edit</th>
					<th style="width: 80px;">Delete</th>
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
								<input type="submit" value="Edit" class="btn btn-secondary btn-sm">
							</form>
						</td>
						<td>
							<form action="admin_news.php" method="post">
								<input type="hidden" name="option" value="d!<?php echo $n['id']; ?>">
								<input type="submit" value="Delete" class="btn btn-danger btn-sm" onclick="return confirm('Do you really want to delete this news article?');">
							</form>
						</td>
					</tr>
				<?php endforeach; ?>
			</tbody>
		</table>
	</div>
	<?php
} else {
	echo '<p>No news articles found.</p>';
}

include 'layout/overall/footer.php';
?>
