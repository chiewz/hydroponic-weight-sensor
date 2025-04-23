What this code does:
	•	Connects ESP8266 board to Wi-Fi.
	•	Connects to a secure MQTT broker (cloud server).
	•	Reads weight from a load cell via HX711 sensor.
	•	Sends weight data in JSON format to the cloud every 10 seconds.
 
 Included Libraries:
	•	Arduino.h – Basic Arduino functions.
	•	ESP8266WiFi.h – Enables Wi-Fi on ESP8266.
	•	WiFiClientSecure.h – Allows secure (SSL) connections.
	•	PubSubClient.h – For MQTT communication.
	•	HX711.h – For using the HX711 weight sensor module.

 Wi-Fi and MQTT Setup
	•	ssid and password: Your Wi-Fi network credentials.
	•	mqtt_server: Address of the MQTT server.
	•	mqttClient_id: Unique client ID for MQTT connection.
	•	mqtt_username & mqtt_password: Used to authenticate with MQTT server.
	•	mqtt_port: Port 443 is used for secure (SSL) MQTT communication.

  HX711 Load Cell Settings
	•	LOADCELL_DOUT_PIN = 2: Data output pin from HX711.
	•	LOADCELL_SCK_PIN = 14: Clock input pin to HX711.
	•	calibration_factor = 21000.0: Converts raw data to real weight.

 Objects and Global Variables
	•	HX711 scale;: Object to interact with the load cell.
	•	WiFiClientSecure espClient;: Secure network client.
	•	PubSubClient mqttClient(espClient);: MQTT client using secure client.
	•	bool isWiFiConnected = false;: Keeps track of Wi-Fi connection status.
	•	unsigned long measurementInterval = 10000;: Send data every 10 seconds.

 Wi-Fi Connection (manageWiFi function)
	•	Checks if Wi-Fi is already connected.
	•	If not:
	•	Tries to connect to the Wi-Fi using WiFi.begin().
	•	Waits up to 20 seconds, printing . every half second.
	•	Sets isWiFiConnected = true if successful.
	•	Prints IP address upon success.
	•	If it fails, prints an error message.

 MQTT Connection (manageMQTT function)
	•	Checks if MQTT is connected.
	•	If not:
	•	Sets the server and port.
	•	Tries 3 times to connect using mqttClient.connect().
	•	If successful, prints “MQTT connected.”
	•	If failed, shows error and retries after 5 seconds.

 Read Weight (getAverageWeight function)
	•	Reads the weight value multiple times.
	•	Adds all readings together.
	•	Divides by the number of samples to get the average.
	•	Returns the average weight.

 Send Data to Cloud (sendWeightOverMQTT function)
	•	Checks if both Wi-Fi and MQTT are connected.
	•	If yes:
	•	Gets the average weight.
	•	Formats data into JSON string like: {"weight": 123.45}.
	•	Sends the data using mqttClient.publish().
	•	Prints confirmation to serial monitor.
	•	If not connected:
	•	Tries to reconnect.

 Setup Function (setup)
	•	Runs once when device powers on.
	•	Starts serial communication.
	•	Initializes load cell with pins and calibration.
	•	Tares (resets) the scale to zero.
	•	Calls setInsecure() to ignore SSL certificate.
	•	Calls manageWiFi() to connect to Wi-Fi.
	•	If Wi-Fi is connected, calls manageMQTT() to connect to MQTT.

 Loop Function (loop)
	•	Repeatedly checks:
	•	If Wi-Fi is connected (calls manageWiFi() if not).
	•	If MQTT is connected (calls manageMQTT() if not).
	•	Keeps MQTT running using mqttClient.loop().
	•	Every 10 seconds:
	•	Calls sendWeightOverMQTT() to send latest weight.
	•	Updates lastMeasurementTime.

 Key Concepts
	•	millis() – Returns how many milliseconds the board has been running.
	•	Serial.print() – Displays messages in the Serial Monitor (for debugging).
	•	scale.set_scale() – Sets conversion factor from raw units to real-world weight.
	•	scale.tare() – Resets the scale to 0.
