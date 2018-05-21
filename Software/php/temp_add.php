<?php
$servername = "localhost";
$username = "username";
$password = "password";
$dbname = "dbname";
	try
	{
		$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
		// set the PDO error mode to exception
		$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

		$sensor = 0;
		if (isset($_GET['sens']))
			$sensor = intval($_GET['sens'], 0);
				
		$temp = null;
		if (isset($_GET['temp']))
			$temp = $_GET['temp'];
		
		$humidity = null;
		if (isset($_GET['hum']))
			$humidity = $_GET['hum'];
	
		$voltage = null;
		if (isset($_GET['vcc']))
			$voltage = $_GET['vcc'];

		$dBm = null;
		if (isset($_GET['dBm']))
			$dBm = intval($_GET['dBm'], 0);		

		$statement = $conn->prepare("INSERT INTO TEMP_SENSORS(sensor, temp, humidity, voltage, dBm)
			VALUES(:sensor, :temp, :humidity, :voltage, :dBm)");
		$statement->execute(array(
			"sensor" => $sensor,
			"temp" => $temp,
			"humidity" => $humidity,
			"voltage" => $voltage,
			"dBm" => $dBm
			));
		$conn = null;
	}
	catch(PDOException $e)
	{
		echo "Connection failed: " . $e->getMessage();
	}
	
	
?>