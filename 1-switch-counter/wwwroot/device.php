<?php
// Save as device.php

// Remove in production: show errors for debugging
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$db = new SQLite3('/tmp/device.db');

function saveToDb($table, $device, $switches) {
    global $db;
    $date = date('c');
    $stmt = $db->prepare("INSERT INTO $table VALUES (:date, :device, :switches)");
    $stmt->bindParam(':date', $date, SQLITE3_TEXT);
    $stmt->bindParam(':device', $device, SQLITE3_TEXT);
    $stmt->bindParam(':switches', json_encode($switches), SQLITE3_TEXT);
    $stmt->execute();
}

function outputJsonResponse($code, $object) {
    header('Content-Type: application/json');
    http_response_code($code);
    echo json_encode($object);
}

if (@$_GET['resource'] == "switches") {
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
        if (@$_POST['device']) {
            // HTML create press (x-www-form-urlencoded POST)
            saveToDb("device_switches", $_POST["device"], $_POST["switches"]);

            echo "Saved. <a href='?resource=switches'>Return</a>";
            return;
        } else {
            // JSON create press (HTTP body POST)
            $json = file_get_contents('php://input');
            $obj = json_decode($json, true); // decode as array
            if (!$obj || !$obj['device']) {
                // return 400 BAD REQUEST
                outputJsonResponse(400, ["success"=>false, "status"=>"No JSON value for device provided."]);
                return;
            }

            saveToDb("device_switches", $obj["device"], $obj["switches"]);

            // return 201 CREATED
            outputJsonResponse(201, ["success"=>true, "status"=>"Saved.", "obj" => $obj]);
            return;
        }
    } else if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        if ($_SERVER['HTTP_ACCEPT'] == 'application/json') {
            // JSON list switches
            $res = $db->query("SELECT * FROM device_switches ORDER BY date DESC");
            $out = [];
            while (($row = $res->fetchArray(SQLITE3_ASSOC))) {
                $out[] = $row;
            }
            header('Content-Type: application/json');
            echo json_encode($out);
            return;
        } else {
            // HTML list switches
            $result = $db->query("SELECT * FROM device_switches ORDER BY date DESC");

            $out = "";
            if ($result) {
                $out .= "<table border=1><thead><tr><th>Date</th><th>Device</th><th>Switches</th></tr></thead><tbody>";
                while (($row = $result->fetchArray(SQLITE3_ASSOC))) {
                    $out .= "<tr><td>".$row['date']."</td><td>".htmlspecialchars($row['device'])."</td><td>".htmlspecialchars($row['switches'])."</td></tr>";
                }
                $out .= "</tbody></table>";
            } else {
                $out = "No results.";
            }

            echo '<form action="" method="POST">
            <h3>Add New Entry:<h3> Device: <input name="device"><br/> Switches: <input name="switches"><br/>
            <input type="submit" value="Submit">
            </form>';

            echo $out;
        }
    } else {
        // return 400 BAD REQUEST
        outputJsonResponse(400, ["success"=>false, "status"=>"Incorrect HTTP method."]);
        return;
    }
} else if (@$_GET['resource'] == "setup") {
    // Remove in production: allow easy database setup
    if (@$_POST['action'] == "Do Setup") {
        // do the recreation of the DB table (POST update)
        $db->exec("DROP TABLE IF EXISTS device_switches;");
        $db->exec("CREATE TABLE IF NOT EXISTS device_switches (date TEXT, device TEXT, switches TEXT)");
        echo "Setup complete. <a href='?'>Return</a>";
        return;
    } else {
        // present user with the option (GET read)
        echo '<form action="?resource=setup" method="POST">
        Are you sure you want to clear the database and reset to defaults?
        <input type="submit" name="action" value="Do Setup">
        </form>';
        return;
    }
} else {
    // no resource selected, present options (GET read)
    echo 'Welcome to the Device Switches app. <br/>';
    echo '<a href="?resource=switches">View Device Switches</a> <br/>';
    echo '<a href="?resource=setup">Setup Database</a>';
    return;
}