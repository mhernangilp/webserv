<?php
$filename = $argv[1];
$exist = $argv[2];

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
    a {
        padding: 10px 20px;
        background-color: #4CAF50;
        color: white;
        text-decoration: none;
        border-radius: 5px;
        margin: 10px; /* Esto separa los botones */
        display: inline-block; /* Hace que los botones estén en una línea */
    }
</style></head><body>";

if ($exist === "YES") {
    echo "<h1><strong>File '$filename' exists</strong></h1>";
} else {
    echo "<h1><strong>File '$filename' doesn't exist</strong></h1>";
}

echo '<a href="../index.html">Home</a>';
echo '<a href="../menu/menu.html">Menu</a>';
echo '<a href="../upload/upload.html">Upload</a>';

echo "</body></html>";
?>
