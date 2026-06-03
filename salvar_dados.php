<?php
$host     = "localhost";
$usuario  = "root";
$senha    = "";
$banco    = "velocimetro_iot";
$conn = new mysqli($host, $usuario, $senha, $banco);
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(["erro" => "Falha na conexão: " . $conn->connect_error]);
    exit();
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(["erro" => "Método não permitido. Use POST."]);
    exit();
}
if (isset($_POST['velocidade']) && isset($_POST['rpm'])) {
    $velocidade = floatval($_POST['velocidade']);
    $rpm        = floatval($_POST['rpm']);
    $stmt = $conn->prepare("INSERT INTO leituras (velocidade, rpm) VALUES (?, ?)");
    $stmt->bind_param("dd", $velocidade, $rpm);

    if ($stmt->execute()) {
        echo json_encode(["status" => "ok", "mensagem" => "Dados salvos com sucesso."]);
    } else {
        http_response_code(500);
        echo json_encode(["erro" => "Falha ao inserir: " . $stmt->error]);
    }

    $stmt->close();

} else {
    http_response_code(400);
    echo json_encode(["erro" => "Parâmetros 'velocidade' e 'rpm' são obrigatórios."]);
}

$conn->close();
?>
