<?php
include "config.php";

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

$limit = 10;
$total_rows = $entry_count;
$total_pages = ceil($total_rows / $limit);
$current_page = isset($_POST['page']) ? $_POST['page'] : 1;
$offset = ($current_page - 1) * $limit;


$sql = "SELECT main_table.ID, sensor_table.SensorName, main_table.SensorReading, 
main_table.TimeEntered, location_table.DeviceLocation, smart_object_table.SmartObjectName,
main_table.DateEntered
FROM main_table 
INNER JOIN sensor_table ON main_table.SensorID = sensor_table.SensorID
INNER JOIN location_table ON main_table.LocationID = location_table.LocationID
INNER JOIN smart_object_table ON main_table.SmartObjectID = smart_object_table.SmartObjectID
LIMIT $limit OFFSET $offset";

$result = $conn->query($sql);
if ($result === false) {
    echo "Error: " . $conn->error;
} elseif ($result->num_rows > 0) {
    printResult($result);
    $result->close();
} else {
    echo "No records found.";
}

echo '<form action="browse.php" method="post">';
echo '<input type="hidden" name="page" value="' . $current_page . '">';

if ($current_page > 1) {
    echo '<button type="submit" name="prev_page" value="true">Previous</button>';
}
if ($current_page < $total_pages) {
    echo '<button type="submit" name="next_page" value="true">Next</button>';
}
else if ($current_page == $total_pages) echo '<button type="submit" name="next_page" value="false">Next</button>';
echo '</form>';

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