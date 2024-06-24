<?php
  require 'database.php';
  
  //---------------------------------------- Condition to check that POST value is not empty.
  if (!empty($_POST)) {
    //........................................ keep track POST values
    $phoneNumber = $_POST['phoneNumber'];
    $Name = $_POST['Name'];
    $rescueType = $_POST['rescueType'];
    $Longitude = $_POST['Longitude'];
    $Latitude = $_POST['Latitude'];
    //........................................
    
    //........................................ Updating the data in the table.
    $pdo = Database::connect();
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $sql = "INSERT INTO rescuerequest (phoneNumber, Name, rescueType, Longitude, Latitude) VALUES (?, ?, ?, ?, ?)";
    $q = $pdo->prepare($sql);
    $q->execute(array($phoneNumber, $Name, $rescueType, $Longitude, $Latitude));
    Database::disconnect();
    //........................................ 
  }
  //---------------------------------------- 
?>