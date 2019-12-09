/*********
  Original for ESP8266 -- Rui Santos
  Complete project details at http://randomnerdtutorials.com  

  Modified for NodeMCU -- Luke Rumbaugh 03 March 19

  Reworked for access by socket client -- Luke Rumbaugh 14 November 19
*********/
#include <ESP8266WiFi.h> // Library for Wi-Fi connection
#include "DHT.h"          // Library for DHT sensor

// sensor pin assignments
int PIN_PROX_SENSOR = D4;   // proximity sensor
int PIN_DHT_SENSOR = D5;    // analog humidity and temperature sensor
int PIN_TOUCH_SENSOR = D6;  // touch sensor (or water, as substitute)
int PIN_LIGHT_SENSOR = D7;  // light sensor
// actuator pin assignments
int PIN_LED_BUILTIN = D0;   // on-board LED output
int PIN_LED_OUT = D1;       // external LED output 
int PIN_BUZZER_OUT = D2;    // piezo buzzer output

// Replace with your network credentials
const char* ssid     = "GCC";
const char* password = "";

// Set web server port number to 80
WiFiServer server(80);

// Initialize DHT sensor
DHT dht(PIN_DHT_SENSOR, DHT11);

// State variables
bool ledState = LOW;

String outputIntState = "off";
String outputExtState = "off";
String inputExtState = "LOW";

// SETUP FUNCTION
void setup() {

  // Task 1: Initialize the serial link to the PC
  Serial.begin(115200);
  delay(1000);
  
  // Task 2: Configure pins as inputs & outputs, and set outputs to default values
  //  sensor pin assignments
  pinMode(PIN_DHT_SENSOR, INPUT_PULLUP);    // analog humidity and temperature sensor
  pinMode(PIN_TOUCH_SENSOR, INPUT);         // touch sensor (or water, as substitute)
  pinMode(PIN_LIGHT_SENSOR, INPUT);         // light sensor
  pinMode(PIN_PROX_SENSOR, INPUT);          // proximity sensor
  //  actuator pin assignments
  pinMode(PIN_LED_BUILTIN, OUTPUT);         // on-board LED output
  pinMode(PIN_LED_OUT, OUTPUT);             // external LED output 
  pinMode(PIN_BUZZER_OUT, OUTPUT);          // piezo buzzer output
  //   initialize actuator values
  ledState = LOW;                             // external LED state
  digitalWrite(PIN_LED_BUILTIN, LOW);         // on-board LED output
  digitalWrite(PIN_LED_OUT, ledState);        // external LED output 
  digitalWrite(PIN_BUZZER_OUT, LOW);          // piezo buzzer output

  // Task 3: Start the analog DHT sensor
  dht.begin();

  // Task 4: Print the MAC address 
  Serial.println();
  Serial.print("Device with MAC address "); Serial.print(WiFi.macAddress()); Serial.println(" reporting for duty.");
  delay(1000);
  
  // Task 5: Connect to network
  //  attempt
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("...attempting to connect to SSID ");
    Serial.print(ssid);
    Serial.println("...");
  }
  //  confirm
  Serial.println("it worked!");
  Serial.print("WiFi connected to SSID ");
  Serial.println(ssid);

  // Task 6: Print the IP address obtained
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Task 7: Start web server
  server.begin(); // on port 80
}

// LOOP FUNCTION
void loop(){

  // Task 1: Listen for client
  WiFiClient client = server.available();   // Listen for incoming clients
  Serial.println("Waiting for client connection.");

  // Task 2: Connect to client and read message from client
  String msgServerReceived; 
  String msgServerSent;
  if (client) {                             // If a new client connects,
    Serial.println("Client connection established");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        msgServerReceived += c;

        // End of HTTP message
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            msgServerSent += "HTTP/1.1 200 OK\r\nContent-type:text/html\r\nConnection: close\r\n";

            Serial.println("Message received is:");
            Serial.println(msgServerReceived);

            // Control actuator message handlers 
            
            // Message Handler 1: Control LED
            if (msgServerReceived.indexOf("PUT /led") >=0) {
              
              // Control option A: Toggle LED
              if (msgServerReceived.indexOf("/led/toggle") >= 0) {
                ledState = !ledState; // toggle state
                digitalWrite(PIN_LED_OUT, ledState); 
                Serial.print("Toggling LED to "); Serial.println(ledState);
                msgServerSent += "Toggling LED to ";
                msgServerSent += ledState;
                msgServerSent += "\r\n";
              }
              
              // Control option B: Turn LED on regardless of previous state
              else if (msgServerReceived.indexOf("/led/on") >= 0) {              
                ledState = HIGH; // force state to HIGH 
                digitalWrite(PIN_LED_OUT, ledState); 
                Serial.print("Setting LED to "); Serial.println(ledState);
                msgServerSent += "Setting LED to ";
                msgServerSent += ledState; 
                msgServerSent += "\r\n";
              }
                            
              // Control option C: Turn LED off regardless of previous state
              else if (msgServerReceived.indexOf("/led/off") >= 0) {              
                ledState = LOW; // force state to low 
                digitalWrite(PIN_LED_OUT, ledState); 
                Serial.print("Setting LED to "); Serial.println(ledState);
                msgServerSent += "Setting LED to ";
                msgServerSent += ledState;  
                msgServerSent += "\r\n";
              }

              // Control option D: Flash LED four times
              else if (msgServerReceived.indexOf("/led/flash") >= 0) {              
                for (int ii=0; ii<4; ii++){
                  ledState = !ledState; 
                  digitalWrite(PIN_LED_OUT, ledState);
                  delay(500);
                }
                ledState = LOW; // force state to low 
                digitalWrite(PIN_LED_OUT, ledState); 
                Serial.print("Flashed LED and reset it to "); Serial.println(ledState);
                msgServerSent += "Flashed LED and reset it to ";
                msgServerSent += ledState;
                msgServerSent += "\r\n";
              }
              
            } // end LED handlers

             // Message Handler 2: Control buzzer
            else if (msgServerReceived.indexOf("PUT /buzz") >=0) {
              
              // Control option A: Activate buzzer for a single 2-second pulse
              if (msgServerReceived.indexOf("/buzz/single") >= 0) {
                analogWrite(PIN_BUZZER_OUT, 64);// switch on
                delay(2000);
                analogWrite(PIN_BUZZER_OUT, 0);  // switch off
                Serial.println("Buzzed for two seconds."); 
                msgServerSent += "Buzzed for two seconds.\r\n"; 
              }

              // Control option B: Pulse buzzer for three short beeps
              if (msgServerReceived.indexOf("/buzz/beeps") >= 0) {
                for (int ii=0; ii<3; ii++) {
                  analogWrite(PIN_BUZZER_OUT, 196); 
                  delay(250); 
                  analogWrite(PIN_BUZZER_OUT, 0);
                  delay(250);
                }
                Serial.println("Sounded a series of beeps."); 
                msgServerSent += "Sounded a series of beeps.\r\n";  
              }

              // Control option C: Turn buzzer off regardless of previous state
              if (msgServerReceived.indexOf("/buzz/off") >= 0) {
                analogWrite(PIN_BUZZER_OUT, 0);
                Serial.println("Buzzer turned off."); 
                msgServerSent += "Buzzer turned off.\r\n";
              }
              
            } // end buzzer handlers

            // Monitor sensor message handlers

            // Message Handler 3: Get sensor values
            else if (msgServerReceived.indexOf("GET /sensors") >=0) {
              
              // Monitor option A: Retrieve proximity sensor values
              if (msgServerReceived.indexOf("/sensors/proximity") >= 0) {                
                int sensorValue = !digitalRead(PIN_PROX_SENSOR); // read 
                String sensorText;
                if (sensorValue) {
                  sensorText = "PRESENT";
                }
                else {
                  sensorText = "NOT PRESENT";
                }
                Serial.print("Digital proximity sensor has value "); Serial.print(sensorValue); Serial.print(" and indicates object is "); Serial.print(sensorText); Serial.println(", which will be passed to remote client."); 
                msgServerSent += "Digital proximity sensor indicates object is ";
                msgServerSent += sensorValue;
                msgServerSent += "\r\n";
              }
              
              // Monitor option B: Retrieve touch sensor values
              else if (msgServerReceived.indexOf("/sensors/touch") >= 0) {                
                int sensorValue = digitalRead(PIN_TOUCH_SENSOR); // read 
                Serial.print("Digital touch sensor value is "); Serial.print(sensorValue); Serial.println(", which will be passed to remote client."); 
                msgServerSent += "Digital touch sensor value is ";
                msgServerSent += sensorValue;
                msgServerSent += "\r\n";
              }

              // Monitor option C: Retrieve light sensor values
              else if (msgServerReceived.indexOf("/sensors/light") >= 0) {                
                int sensorValue = !digitalRead(PIN_LIGHT_SENSOR); // read 
                String sensorText;
                if (sensorValue) {
                  sensorText = "HIGH";
                }
                else {
                  sensorText = "LOW";
                }
                Serial.print("Digital sensor indicates ambient light is "); Serial.print(sensorText); Serial.println(", which will be passed to remote client."); 
                msgServerSent += "Digital sensor indicates ambient light is ";
                msgServerSent += sensorText;
                msgServerSent += "\r\n";
              }

              // Monitor option D: Retrieve temperature value
              else if (msgServerReceived.indexOf("/sensors/temperature") >= 0) {                
                int N = 10;                   // number of readings to average
                float tempValueSum = 0;       // temporary variable for sum of averages
                for (int ii=0;ii<N;ii++){        // take N readings and average them
                  tempValueSum = tempValueSum + dht.readTemperature(true);  // take each reading
                }
                float tempValue = tempValueSum / N;     // average readings
                Serial.print("Temperature value is "); Serial.print(tempValue); Serial.println(" deg F, which will be passed to remote client.");
                msgServerSent += "Temperature value is ";
                msgServerSent += tempValue;
                msgServerSent += " deg F\r\n";
              }

              // Monitor option E: Retrieve humidity value
              else if (msgServerReceived.indexOf("/sensors/humidity") >= 0) {                
                int N = 10;                   // number of readings to average
                float humidValueSum = 0;       // temporary variable for sum of averages
                for (int ii=0;ii<N;ii++){        // take N readings and average them
                  humidValueSum = humidValueSum + dht.readHumidity();  // take each reading
                }
                float humidValue = humidValueSum / N;     // average readings
                Serial.print("Humidity value is "); Serial.print(humidValue); Serial.println(" %, which will be passed to remote client.");
                msgServerSent += "Humidity value is ";
                msgServerSent += humidValue;
                msgServerSent += " %\r\n";
              }

              // Monitor option F: Retrieve all values, and return them in a JSON string
              else {
                // read digital sensors
                int proxSensorValue = !digitalRead(PIN_PROX_SENSOR); // read 
                int touchSensorValue = digitalRead(PIN_TOUCH_SENSOR); // read   
                int lightSensorValue = !digitalRead(PIN_LIGHT_SENSOR); // read 
                // read analog sensors
                int N = 10;                   // number of readings to average
                float tempValueSum = 0;       // temporary variable for sum of averages
                float humidValueSum = 0;       // temporary variable for sum of averages
                for (int ii=0;ii<N;ii++){        // take N readings and average them
                  tempValueSum = tempValueSum + dht.readTemperature(true);  // take each reading
                  humidValueSum = humidValueSum + dht.readHumidity();  // take each reading
                }
                float tempSensorValue = tempValueSum / N;     // average readings
                float humidSensorValue = humidValueSum / N;     // average readings
                // construct JSON string
                char sensorString[100];
                char tempString[10];
                char humidString[10];
                dtostrf(tempSensorValue,4,2,tempString);
                dtostrf(humidSensorValue,4,1,humidString);
                sprintf(sensorString, "{ \"proximity\": %i, \"temperature (F)\": %s, \"humidity (%)\": %s }", proxSensorValue,tempString,humidString);
                Serial.println("Sensor value JSON is as follows:"); Serial.println(sensorString); Serial.println();
                msgServerSent += "Sensor values are as follows:";
                msgServerSent += sensorString;
                msgServerSent += "\r\n";
              }
              
            } // end sensor monitoring handlers
            
            // Message Handler 4: Quit
            else if (msgServerReceived.indexOf("GET /quit") >0 ) {
              Serial.println("Disconnecting at client request."); 
              msgServerSent += "Disconnecting.\r\n";
              client.stop();
              break;
            }

            client.println(msgServerSent);  // send whole message in one HTTP message
            msgServerSent = ""; // prepare new HTTP message
            msgServerReceived = "";     // clear last HTTP message received to prepare for next message
            
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
          
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }

    }
  }

  // Task 2: Flash the built-in LED 
  delay(500);
  digitalWrite(PIN_LED_BUILTIN,1);
  delay(500);
  digitalWrite(PIN_LED_BUILTIN,0);
  
}
