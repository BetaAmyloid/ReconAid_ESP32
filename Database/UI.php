<?php
$esp32_ip = "192.168.254.179"; // IP address of the ESP32
$esp32_port = 80; // Port where the ESP32 server is running

$message = $_POST['message']; // Simple message

$data = array(
    'message' => $message
);

$options = array(
    'http' => array(
        'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
        'method'  => 'POST',
        'content' => http_build_query($data),
    ),
);

$context  = stream_context_create($options);
$result = file_get_contents("http://$esp32_ip:$esp32_port/alert", false, $context);

if ($result === FALSE) {
    // Handle error
}

echo $result;
?>
