<?php
// upload_response.php

// Obtener el nombre del archivo
$filename = $argv[1]; // El nombre del archivo viene como argumento

// Generar la respuesta HTML con CSS en línea
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
    p {
        font-size: 1.2em;
    }
    #contador {
        font-weight: bold;
        color: #FF5722;
    }
</style></head><body>";

echo "<h1><strong>$filename</strong> file was successfully uploaded!</h1>";
echo "<p>You are goint to be redirected automatically in <span id='contador'>5</span> seconds...</p>";
echo "
<script>
// Configura el contador
let segundos = 5;
const contador = document.getElementById('contador');

const interval = setInterval(() => {
    segundos--;
    contador.textContent = segundos;

    if (segundos <= 0) {
        clearInterval(interval);
        window.location.href = 'upload.html'; // Cambia la URL al menú principal
    }
}, 1000);
</script>";

echo "</body></html>";
?>
