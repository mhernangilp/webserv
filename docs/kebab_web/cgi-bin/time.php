<?php

echo "<html><head><style>
    body {
        font-family: Arial, sans-serif;
        text-align: center;
        margin-top: 50px;
        color: #333;
    }
    h1 {
        color: #4CAF50;
    }
</style></head><body>";

// Establecer la zona horaria
date_default_timezone_set('Europe/Madrid'); // Cambia esto según tu zona horaria

// Mostrar la hora actual
echo "<h1><strong>La hora actual es: </strong></h1>" . date("H:i:s");
?>
