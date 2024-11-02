<?php
// upload_response.php

// Obtener el nombre del archivo
$filename = $argv[1]; // El nombre del archivo viene como argumento

// Generar la respuesta HTML
echo "<html><body>";
echo "<h1><strong>$filename</strong> file was successful</h1>";
echo "</body></html>";
?>
