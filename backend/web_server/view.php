<?php
include "config.php";

global $conn;
$type =  $_POST["request_type"];

switch ($type) {
    case 'all':
        getAll();
        break;
    case 'sensor_name':
        getSensorNameEntries();
        break;
    case 'smart_object_name':
        getSmartObjectEntries();
        break;
    case 'location':
        getLocationEntries();
        break;
    case 'date':
        getDateEntries();
        break;
    default:
        echo "Invalid listing type.";
}

function getAll(){
    global $conn;

    $sql = "SELECT COUNT(*) AS entry_count FROM main_table";
    $result = mysqli_query($conn, $sql);
    if ($result) {
        $row = mysqli_fetch_assoc($result);
        $entry_count = $row['entry_count'];
    } else {
        echo "Error: " . $sql . "<br>" . mysqli_error($conn);
        return;
    }
    
    $sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
    main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
    main_table.DateEntered
    FROM main_table 
    INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
    INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
    INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
    ORDER BY main_table.TimeEntered DESC LIMIT $entry_count";
    
    $result = $conn->query($sql);
    if ($result === false) {
        echo "Error: " . $conn->error;
    } elseif ($result->num_rows > 0) {
        printResult($result);
        $result->close();
    } else {
        echo "No records found.";
    }
    
}


function getSensorNameEntries(){
    $SensorName = $_POST['name'];
    $number_of_entries = $_POST["quantity"];

    if (!(isset($SensorName)) or $SensorName == '') {
        echo "Error: Blank sensor name field";
        return "Error: Blank sensor name field";
    }
    if (!(isset($number_of_entries))) {
        echo "Error: Blank quantity field";
        return "Error: Blank quantity field";
    }
    else if ($number_of_entries == ''){
        $number_of_entries = 15;
    }

    global $conn;
    $entry_count = countEntries("sensor_table", "SensorName", $SensorName, "SensorID");

    if ($number_of_entries > $entry_count) {
        if ($entry_count >= 15) $number_of_entries = 15;
        else $number_of_entries = $entry_count;
    }
    $sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
    main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
    main_table.DateEntered
    FROM main_table 
    INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
    INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
    INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
    WHERE sensor_table.SensorName = '$SensorName'
    ORDER BY main_table.TimeEntered DESC LIMIT $number_of_entries";
    
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        printResult($result);
        $result->close();
        return $result;
    }
    else { echo "\nNo records found."; return "No records found."; }
}

function getSmartObjectEntries(){
    $SmartObjectName = $_POST['name'];
    $number_of_entries = $_POST["quantity"];

    if (!(isset($SmartObjectName)) or $SmartObjectName == '') {
        echo "Error: Blank smart object name field";
        return "Error: Blank smart object name field";
    }
    if (!(isset($number_of_entries))) {
        echo "Error: Blank quantity field";
        return "Error: Blank quantity field";
    }
    else if ($number_of_entries == ''){
        $number_of_entries = 15;
    }

    global $conn;
    $entry_count = countEntries("smart_object_table", "SmartObjectName", $SmartObjectName, "SmartObjectID");

    if ($number_of_entries > $entry_count) {
        if ($entry_count >= 15) $number_of_entries = 15;
        else $number_of_entries = $entry_count;
    }
    $sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
    main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
    main_table.DateEntered
    FROM main_table 
    INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
    INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
    INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
    WHERE smart_object_table.SmartObjectName = '$SmartObjectName'
    ORDER BY main_table.TimeEntered DESC LIMIT $number_of_entries";
    
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        printResult($result);
        $result->close();
        return $result;
    }
    else { echo "\nNo records found."; return "No records found."; }
}

function getLocationEntries(){
    $Location = $_POST['name'];
    $number_of_entries = $_POST["quantity"];
    
    if (!(isset($Location)) or $Location == '') {
        echo "Error: Blank location field";
        return "Error: Blank location field";
    }
    if (!(isset($number_of_entries))) {
        echo "Error: Blank quantity field";
        return "Error: Blank quantity field";
    }
    else if ($number_of_entries == ''){
        $number_of_entries = 15;
    }

    global $conn;
    $entry_count = countEntries("location_table", "DeviceLocation", $Location, "LocationID");

    if ($number_of_entries > $entry_count) {
        if ($entry_count >= 15) $number_of_entries = 15;
        else $number_of_entries = $entry_count;
    }
    $sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
    main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
    main_table.DateEntered
    FROM main_table 
    INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
    INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
    INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
    WHERE location_table.DeviceLocation = '$Location'
    ORDER BY main_table.TimeEntered DESC LIMIT $number_of_entries";
    
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        printResult($result);
        $result->close();
        return $result;
    }
    else { echo "\nNo records found."; return "No records found."; }
}

function getDateEntries() {
    $DateEntered = $_POST['name'];
    $number_of_entries = $_POST["quantity"];

    if (!(isset($DateEntered)) or $DateEntered == '') {
        echo "Error: Blank date field";
        return "Error: Blank date field";
    }
    if (!(isset($number_of_entries))) {
        echo "Error: Blank quantity field";
        return "Error: Blank quantity field";
    }
    else if ($number_of_entries == ''){
        $number_of_entries = 15;
    }

    global $conn;

    $sql = "SELECT COUNT(*) AS entry_count FROM main_table WHERE DateEntered = $DateEntered";
    $result = mysqli_query($conn, $sql);
    if ($result) {
        $row = mysqli_fetch_assoc($result);
        $entry_count = $row['entry_count'];
    }
    else {    echo "Error: " . $sql . "<br>" . mysqli_error($conn);    }

    if ($number_of_entries > $entry_count) {
        if ($entry_count >= 15) $number_of_entries = 15;
        else $number_of_entries = $entry_count;
    }
    $sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
    main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
    main_table.DateEntered
    FROM main_table 
    INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
    INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
    INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
    WHERE main_table.DateEntered = '$DateEntered'
    ORDER BY main_table.TimeEntered DESC LIMIT $number_of_entries";
    
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        printResult($result);
        $result->close();
        return $result;
    }
    else { echo "\nNo records found."; return "No records found."; }
}

function countEntries($table, $column, $param, $key) {
    global $conn;
    $sql = "SELECT COUNT(*) AS entry_count FROM main_table
            WHERE $key = (SELECT $key FROM $table WHERE $column = '$param')";
    $result = mysqli_query($conn, $sql);
    if ($result) {
        $row = mysqli_fetch_assoc($result);
        $count = $row['entry_count'];
        //echo "Number of entries: " . $count;
        return $count;
    }
    else {
        echo "Error: " . $sql . "<br>" . mysqli_error($conn);
        return "Error: " . $sql . "<br>" . mysqli_error($conn);
    }
}


function printResult($result) {
    echo "<div class='container'>
    <table border='1'>
    <tr>
        <th>Date</th>
        <th>TimeStamp</th>
        <th>Sensor Name</th>
        <th>Sensor Reading</th>
        <th>Location</th>
        <th>Smart Object</th>

    </tr>";
    while ($row = mysqli_fetch_assoc($result)) {
        echo "
            <tr>
                <td>" . $row["DateEntered"] . "</td>
                <td>" . $row["TimeEntered"] . "</td>
                <td>" . $row["SensorName"] . "</td>
                <td>" . $row["SensorReading"] . "</td>
                <td>" . $row["DeviceLocation"] . "</td>
                <td>" . $row["SmartObjectName"] . "</td>
            </tr>";
    }
    echo "</table></div>";
}

?>