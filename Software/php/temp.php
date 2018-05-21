<?php
	echo "<table style='border: solid 1px black;'>";
	echo "<tr><th>Sensor</th><th>Date</th><th>Temperature</th><th>Humidity</th><th>Voltage</th><th>dBm</th></tr>";

	class TableRows extends RecursiveIteratorIterator 
	{ 
		function __construct($it) 
		{ 
			parent::__construct($it, self::LEAVES_ONLY); 
		}

		function current() 
		{
			return "<td style='width:150px;border:1px solid black;'>" . parent::current(). "</td>";
		}

		function beginChildren() 
		{ 
			echo "<tr>"; 
		} 

		function endChildren() 
		{ 
			echo "</tr>" . "\n";
		} 
	} 

	$servername = "localhost";
	$username = "username";
	$password = "password";
	$dbname = "dbname";

	try
	{
		$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
		$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		$stmt = $conn->prepare("SELECT HEX(sensor), date, temp, humidity, voltage, dBm FROM TEMP_SENSORS ORDER BY date DESC Limit 500"); 
		$stmt->execute();

		// set the resulting array to associative
		$result = $stmt->setFetchMode(PDO::FETCH_ASSOC); 
		foreach(new TableRows(new RecursiveArrayIterator($stmt->fetchAll())) as $k=>$v)
		{ 
			echo $v;
		}
	}
	catch(PDOException $e)
	{
		echo "Error: " . $e->getMessage();
	}
	$conn = null;
	echo "</table>";
?>