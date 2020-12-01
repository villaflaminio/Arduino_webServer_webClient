
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

//------------------ PIN & MEMORY -------------------
// memorizzo lo stato attuale dei pin
String output5State = "off";
String output4State = "off";

// Assegno un nome da associare ai pin di arduino
const int output5 = 13;
const int output4 = 4;

int timeSend = 1000;
unsigned long time_now = 0;
 
//------------------- NETWORK-------------------
//network credentials
const char* ssid     = "Org";
const char* password = "flaminioroom";
//static ip address
IPAddress local_IP(192, 168, 10, 130);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//-----------------------Section Time-----------------------
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
//------------------------END DICHIARAZIONI---------------------------

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);
  
  //----------------------------- inizialize wifi-------------------------
  if (!WiFi.config(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
 
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if(millis() >= time_now + timeSend){
        time_now += timeSend;
        sendValue();
    }
    
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
//--------- EFFETTUO LA DECODIFICA DELL' URL, MODIFICANDO LO STATO DEI PIN ----------------------------------------------//            
            modPin();
            
            client.println("http://192.168.10.116/setValues");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


//------ INVIO IL FILE JASON 
void sendValue() { 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
    JSONencoder["s1"] = 5154;
    JSONencoder["s2"] = "5";
    JSONencoder["s3"] = "5";
    
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
 
    HTTPClient http;    //Declare object of class HTTPClient
 
    http.begin("http://192.168.10.107:5000/setValues");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
 
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();//Get the response payload
 
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
 
    http.end();  //Close connection
 
  } else { 
    Serial.println("Error in WiFi connection");
 
  } 
}
void modPin(){
  if (header.indexOf("GET /5/on") >= 0) {
    Serial.println("GPIO 5 on");
    output5State = "on";
    digitalWrite(output5, HIGH);
  } else if (header.indexOf("GET /5/off") >= 0) {
    Serial.println("GPIO 5 off");
    output5State = "off";
    digitalWrite(output5, LOW);
  } else if (header.indexOf("GET /4/on") >= 0) {
    Serial.println("GPIO 4 on");
    output4State = "on";
    digitalWrite(output4, HIGH);
  } else if (header.indexOf("GET /4/off") >= 0) {
    Serial.println("GPIO 4 off");
    output4State = "off";
    digitalWrite(output4, LOW);
  }
  
}
