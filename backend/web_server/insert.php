<?php

include 'config.php';
header('Content-Type: application/json');

$method = $_SERVER['REQUEST_METHOD'];
$data = json_decode(file_get_contents('php://input'), true);

switch ($method) {
  case 'POST':
    insertData($data);
    break;
  default:
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed']);
    break;
}

function insertData($data) {
    global $conn;

    $SensorName = $data['SensorName'];
    $SensorReading = $data['SensorReading'];
    $Location = $data['Location'];
    $SmartObject = $data['SmartObject'];

    if (empty($SensorName) || empty($SensorReading) || empty($Location) || empty($SmartObject)) {
        echo "Error: Missing required fields.";
        return "Error: Missing required fields.";
    }
    
    $SensorName = mysqli_real_escape_string($conn, $SensorName);
    $SensorReading = mysqli_real_escape_string($conn, $SensorReading);
    $Location = mysqli_real_escape_string($conn, $Location);
    $SmartObject = mysqli_real_escape_string($conn, $SmartObject);

    $sql1 = "INSERT INTO sensor_table (SensorName) SELECT '$SensorName'
            WHERE NOT EXISTS (SELECT 1 FROM sensor_table WHERE SensorName = '$SensorName')";

    $sql2 = "INSERT INTO smart_object_table (SmartObjectName) SELECT '$SmartObject'
             WHERE NOT EXISTS (SELECT 1 FROM smart_object_table WHERE SmartObjectName = '$SmartObject')";

    $sql3 = "INSERT INTO location_table (DeviceLocation) SELECT '$Location'
            WHERE NOT EXISTS (SELECT 1 FROM location_table WHERE DeviceLocation = '$Location')";

    if ($conn->query($sql1) === TRUE && $conn->query($sql2) === TRUE && $conn->query($sql3) === TRUE) { 
        // Insert data into the main_table
        $sql = "INSERT INTO main_table (SensorID, SensorReading, LocationID, SmartObjectID)
                SELECT
                (SELECT SensorID FROM sensor_table WHERE SensorName = '$SensorName'),
                '$SensorReading',
                (SELECT LocationID FROM location_table WHERE DeviceLocation = '$Location'),
                (SELECT SmartObjectID FROM smart_object_table WHERE SmartObjectName = '$SmartObject')
               FROM DUAL";
            
        if ($conn->query($sql) === TRUE) {
            echo "Data inserted successfully.";
            return "Data inserted successfully.";
        }
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
            return "Error: " . $sql . "<br>" . $conn->error;
        }
    }
    else {
        echo "Error: " . $conn->error;
        return "Error: " . $conn->error;
    }
}

?>
