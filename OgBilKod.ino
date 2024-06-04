#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// WiFi and MQTT settings
const char* WIFI_SSID = "ABBgym_2.4";  // Your WiFi SSID
const char* WIFI_PASSWORD = "mittwifiarsabra";  // Your WiFi password
const char* MQTT_SERVER = "maqiatto.com";  // MQTT server address
const int MQTT_PORT = 1883;  // MQTT server port
const char* MQTT_USERNAME = "olle.lindgren@hitachigymnasiet.se";  // MQTT username
const char* MQTT_PASSWORD = "123456";  // MQTT password
const char* CONTROL_TOPIC = "olle.lindgren@hitachigymnasiet.se/cornybig"; // MQTT topic for control messages
String clientId = String(ESP.getChipId());  // Unique client ID based on ESP chip ID

// Motor control pins
#define motorPin1  D1 // GPIO 5 for motor control
#define motorPin2  D3 // GPIO 4 for motor control

#define SERVO_PIN D5 // Pin for servo control

WiFiClient espClient;
PubSubClient client(espClient); // MQTT client
Servo steeringServo; // Servo object

// Function to set up WiFi connection
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Start WiFi connection

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Print the local IP address
}

// Callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  static char message[256]; // Static buffer for payload
  // Copy payload to static buffer and null-terminate
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  Serial.println(message);

  // Control motor and servo based on message
  if (strcmp(message, "up") == 0) {
    analogWrite(motorPin1, 1000); // Motor moving up with PWM signal
    digitalWrite(motorPin2, LOW); // Motor direction control
    Serial.println("Motor moving up");
  } else if (strcmp(message, "down") == 0) {
    analogWrite(motorPin1, 100); // Motor moving down with PWM signal
    digitalWrite(motorPin2, HIGH); // Motor direction control
    Serial.println("Motor moving down");
  } else if (strcmp(message, "stop") == 0) {
    analogWrite(motorPin1, 0); // Stop the motor
    digitalWrite(motorPin2, LOW); // Ensure motor direction is low
    Serial.println("Motor stopped");
  } else if (strcmp(message, "left") == 0) {
    // Move servo to the left position
    steeringServo.write(0); // Adjust the angle as needed for your servo
    Serial.println("Servo moving left");
  } else if (strcmp(message, "right") == 0) {
    // Move servo to the right position
    steeringServo.write(180); // Adjust the angle as needed for your servo
    Serial.println("Servo moving right"); // Servo moving to right
  }
}

// Function to handle reconnection to MQTT server
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(CONTROL_TOPIC); // Subscribe to control topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

// Arduino setup function
void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
  setup_wifi(); // Connect to WiFi
  client.setServer(MQTT_SERVER, MQTT_PORT); // Set MQTT server
  client.setCallback(callback); // Set callback function

  // Set the motor control pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  // Attach the servo
  steeringServo.attach(SERVO_PIN);
}

// Arduino loop function
void loop() {
  if (!client.connected()) {
    reconnect(); // Reconnect to MQTT if disconnected
  }
  client.loop(); // Process MQTT messages
}
