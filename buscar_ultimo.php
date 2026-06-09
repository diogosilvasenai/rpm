<?php
include 'connect.php';

$query = $conn->query("SELECT rpm, kph FROM monitoramento ORDER BY id DESC LIMIT 1");

if ($query && $query->num_rows > 0) {
    $row = $query->fetch_assoc();
    echo json_encode(['rpm' => $row['rpm'], 'kph' => $row['kph']]);
} else {
    echo json_encode(['rpm' => 0, 'kph' => 0]);
}
$conn->close();
?>
