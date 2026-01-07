<?php

$servername= "localhost";
$username="root";
$password="";
$dbname="final_project_db";

$conn = mysqli_connect($servername,$username,$password,$dbname) or die ("could not connect database");

if ($conn->connect_error) {  die("Connection failed: " . $conn->connect_error); }

?>