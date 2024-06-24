<?php
  include 'database.php';
  
  //---------------------------------------- Condition to check that POST value is not empty.
  if (!empty($_POST)) {
    // keep track post values
    $phoneNumber = $_POST['phoneNumber'];
    
    $myObj = (object)array();
    
    //........................................ 
    $pdo = Database::connect();
    $sql = 'SELECT * FROM rescuerequest WHERE phoneNumber="' . $phoneNumber . '"';
    foreach ($pdo->query($sql) as $row) {
      $myObj->phoneNumber = $row['phoneNumber'];
      
      $myJSON = json_encode($myObj);
      
      echo $myJSON;
    }
    Database::disconnect();
    //........................................ 
  }
  //---------------------------------------- 
?>