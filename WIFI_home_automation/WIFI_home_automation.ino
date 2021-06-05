#include <ESP8266WiFi.h>

// WIFI credential
const char* ssid     = "polopolo";
const char* password = "qwertyytrewq";

// Setting web server port number to default 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// variables to store the current output state
String output5State = "off";
String output4State = "off";

//output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

// Current time
unsigned long currentTime = millis();

// Previous time
unsigned long previousTime = 0; 

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  
  // output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  
  // Set outputs to LOW as default
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with given SSID and password
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
  // Listening incoming clients
  WiFiClient client = server.available();   

  if (client) {
    // make a String to hold incoming data from the client
    String currentLine = "";                
    currentTime = millis();
    previousTime = currentTime;
     // loop while the client's connected
     
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();         
       
       // if there's bytes to read from the client,
      if (client.available()) {            
        
        // read a byte, then
        char c = client.read();             
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          
          if (currentLine.length() == 0) {
            // predefault HTTP request
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // power outlet control
            // NB as we know if we use a relay then at ON condition at GPIO  pin the relay will OFF and vice versa
            // from below code u can easily understand
            
            // for GPIO5 PIN
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, HIGH);
            }
            
            // for GPIO4 PIN -> D2 PIN
            else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, LOW);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, HIGH);
            }
            
            // UI RENDERING
            // for below all HTML code all html & CSS codes are converted at http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #e63946; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 1px; cursor: pointer;}");
            client.println(".button2 {background-color: #06d6a0;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1><u>Controlling home power outlet using Node MCU and Relay</u></h1>");
            client.println("<body><h3>18BEE0070</h3>");
            client.println("<body><h3>18BEE0378</h3>");
            client.println("<body><h3>18BEE0179</h3>");
            
            // UI render of current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            // If the output5State is off, it displays the ON button       
            
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            
            
            // UI render of current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            client.println();
            
            
            // Break out of the while loop
            break;
          } else { 
            // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  
          // if you got anything else but a carriage return character, add it to the end of the currentLine
          currentLine += c;      
        }
      }
    }
    // it,s the else part, when client get disconnected
    // Clear the header variable
    header = "";
    // Terminate the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
