<?php

include('config.php');

$query = "SELECT * FROM bookmarks ORDER BY dt DESC";
$result = mysql_query($query) or die(mysql_error());

$bookmarks = array();
while ($row = mysql_fetch_assoc($result)) {
  $bookmarks[] = $row;
}

include_once("header.php");
?>
<h2>Bookmarks</h2>
<p><a href='new.php'>Create a new bookmark</a></p>

<table>
  <thead>
    <tr>
      <th><b>URL</b></th>
      <th><b>Title</b></th>
      <th><b>Date Created</b></th>
      <th>Actions</th>
    </tr>
  </thead>
  <tbody>

<?php

foreach ($bookmarks as $bookmark) {
  echo "<tr>\n";
  echo "<td valign='top'><a href=". $bookmark['url'] .">" . $bookmark['url'] ."</a></td>\n";
  echo "<td valign='top'>" . $bookmark['title'] . "</td>\n";
  echo "<td valign='top'>" . $bookmark['dt'] . "</td>\n";
  echo "<td valign='top'><a href=edit.php?id={$bookmark['id']}>Edit</a>  <a href=delete.php?id={$bookmark['id']}>Delete</a></td>\n";
  echo "</tr>\n";
}

?>

  </tbody>
</table>

<?php
include_once("footer.php");
?>
