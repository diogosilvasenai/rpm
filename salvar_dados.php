<?php
include 'connect.php';

$kph = $_POST['velocidade'] ?? 0;
$rpm = $_POST['rpm'] ?? 0;

$stmt = $conn->prepare("INSERT INTO monitoramento (rpm, kph) VALUES (?, ?)");
$stmt->bind_param("dd", $rpm, $kph);
$stmt->execute();
$stmt->close();
$conn->close();
?>
