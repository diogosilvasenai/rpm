<?php
// Define que a resposta será JSON
header("Content-Type: application/json");

// Permite requisições do mesmo servidor (CORS local)
header("Access-Control-Allow-Origin: *");

// Configurações de conexão com o banco de dados
$host    = "localhost";
$usuario = "root";
$senha   = "";
$banco   = "velocimetro_iot";

// Conecta ao banco de dados
$conn = new mysqli($host, $usuario, $senha, $banco);

// Verifica se a conexão falhou
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(["erro" => "Falha na conexão: " . $conn->connect_error]);
    exit();
}

// Busca o registro mais recente da tabela
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
    // Retorna zeros quando ainda não há dados
    echo json_encode([
        "velocidade"    => 0,
        "rpm"           => 0,
        "registrado_em" => null
    ]);
}

$conn->close();
?>
