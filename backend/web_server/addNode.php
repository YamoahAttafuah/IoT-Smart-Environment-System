<?php
include "config.php";

global $conn;
$type =  $_POST["request_type"];

switch ($type) {
    case 'add':
        addNode();
        break;
    case 'rename':
        renameNode();
        break;
    default:
        echo "Invalid listing type.";
}

function addNode(){
    $node_name =  $_POST["node_name"];
    if (!(isset($node_name)) or $node_name == '') {
        echo "Error: Blank node name field";
        return "Error: Blank node field";
    }
    else {
        global $conn;
        
        $sql = "INSERT INTO smart_object_table (SmartObjectName) SELECT '$node_name'
                WHERE NOT EXISTS (SELECT 1 FROM smart_object_table WHERE SmartObjectName = '$node_name')";

        $result = $conn->query($sql);
        if ($result === TRUE) {
            if ($conn->affected_rows > 0) {
                echo "New smart object, $node_name, successfully added.";
            } else {
                echo "Error: smart object, $node_name, already exists.";
            }
        }
        else {
            echo "Error: " . $conn->error;
            return "Error: " . $conn->error;
        }
    }
}

function renameNode(){
    $old_node_name =  $_POST["old_node_name"];
    $new_node_name =  $_POST["new_node_name"];

    if (!(isset($old_node_name) or isset($new_node_name)) or $new_node_name == '' or $old_node_name == '') {
        echo "Error: Blank field";
        return "Error: Blank field";
    }

    global $conn;

    $sql = "UPDATE smart_object_table
    SET SmartObjectName = '$new_node_name'
    WHERE SmartObjectName = '$old_node_name'";
    
    $result = $conn->query($sql);
    if ($result === TRUE) {
        if ($conn->affected_rows > 0) {
            echo "Smart object name changed from 
            $old_node_name to $new_node_name.";
        } else {
            echo "Error: smart object, $old_node_name, does not exist.";
        }
    }
    else {
        echo "Error: " . $conn->error;
        return "Error: " . $conn->error;
    }
}
?>