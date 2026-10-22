<div class="well search_widget widget" id="searchContainer">
	<div class="header">
		Buscar Personagem
	</div>
	<div class="body">
		<div class="relative">
			<div id="name_suggestion"></div>
		</div>
		<form class="searchForm" action="characterprofile.php" method="get">
			<label for="src_name">Nome: </label>
			<input autocomplete="off" type="text" name="name" id="src_name" class="search form-control" placeholder="Digite o nome...">
		</form>
		<?php
		$cache = new Cache('engine/cache/characterNames');
		if ($cache->hasExpired()) {
			$names_sql = mysql_select_multi("SELECT `Name` AS `name` FROM `Characters` WHERE `Deleted` = 0 ORDER BY `Name` ASC;");
			$names = array();
			if ($names_sql !== false) {
				foreach ($names_sql as $name) {
					$names[] = $name['name'];
				}
			}
			$cache->setContent($names);
			$cache->save();
		} else {
			$names = $cache->load();
		}
		?>
		<script type="text/javascript">
			window.searchNames = <?php echo json_encode($names ? $names : array()); ?>;
			$(function() {
				if (window.searchNames && window.searchNames.length > 0) {
					$('#src_name').keyup(function(e) {
						$('#name_suggestion').html('');
						var search = $(this).val().toLowerCase();
						var results = new Array();
						if (search.length > 0) {
							for (var i = 0; i < window.searchNames.length && results.length < 10; i++) {
								if (window.searchNames[i].toLowerCase().indexOf(search) > -1) {
									results.push(window.searchNames[i]);
								}
							}
						}
						if (results.length > 0) {
							var search_html = "";
							for (var j = 0; j < results.length; j++) {
								search_html += '<div class="sname"><a href="characterprofile.php?name=' + encodeURIComponent(results[j]) + '">' + results[j] + '</a></div>';
							}
							$('#name_suggestion').addClass('show').html(search_html);
						} else {
							$('#name_suggestion.show').removeClass('show');
						}
					});
				}
			});
		</script>
	</div>
</div>
