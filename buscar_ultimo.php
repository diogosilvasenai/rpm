<?php
header("Content-Type: application/json");
header("Access-Control-Allow-Origin: *");
$host    = "localhost";
$usuario = "root";
$senha   = "";
$banco   = "velocimetro_iot";
$conn = new mysqli($host, $usuario, $senha, $banco);
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(["erro" => "Falha na conexão: " . $conn->connect_error]);
    exit();
}
$sql    = "SELECT velocidade, rpm, registrado_em FROM leituras ORDER BY id DESC LIMIT 1";
$result = $conn->query($sql);

if ($result && $result->num_rows > 0) {
    $row = $result->fetch_assoc();

    echo json_encode([
        "velocidade"    => floatval($row["velocidade"]),
        "rpm"           => floatval($row["rpm"]),
        "registrado_em" => $row["registrado_em"]
    ]);
} else {
    echo json_encode([
        "velocidade"    => 0,
        "rpm"           => 0,
        "registrado_em" => null
    ]);
}

$conn->close();
?>
