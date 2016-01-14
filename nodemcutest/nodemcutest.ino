/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#define LED1 5
#define LED2 4
#define PUSH1 12

#include <ESP8266WiFi.h>

const char* ssid = "Erhandroid";
const char* password = "12345678";

void GetSwitchState(WiFiClient);
void SetLEDStates(WiFiClient, String);
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  //pinMode(2, OUTPUT);
  //digitalWrite(2, 0);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(PUSH1, INPUT_PULLUP);
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

String HTTP_req;            // stores the HTTP request
void loop() 
{
  WiFiClient client = server.available();

  if (client)
  {                             // if you get a client,
    Serial.print("new client! ");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    boolean newConnection = true;     // flag for new connections
    unsigned long connectionActiveTimer;  // will hold the connection start time

    boolean currentLineIsBlank = true;
    while (client.connected())
    {       // loop while the client's connected
      if (newConnection)
      {                 // it's a new connection, so
        connectionActiveTimer = millis(); // log when the connection started
        newConnection = false;          // not a new connection anymore
      }
      if (!newConnection && connectionActiveTimer + 1000 < millis())
      { 
        // if this while loop is still active 1000ms after a web client connected, something is wrong
        break;  // leave the while loop, something bad happened
      }


      if (client.available())
      {             // if there's bytes to read from the client,    
        char c = client.read();             // read a byte, then
        // This lockup is because the recv function is blocking.
        Serial.print(c);
        HTTP_req += c;  // save the HTTP request 1 char at a time
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: keep-alive");
          client.println();
          // AJAX request for switch state
          if (HTTP_req.indexOf("ajax_switch") > -1)
          {
            // read switch state and send appropriate paragraph text
            GetSwitchState(client);
            SetLEDStates(client, HTTP_req);

          }
          else 
          {  // HTTP request for web page
            // send web page - contains JavaScript with AJAX calls
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Erhan Web Page</title>");
            client.println("<script>");
            client.println("var led1, x1, led2, x2;");
            client.println("function GetSwitchState() {");
            client.println("nocache = led1 + led2 + \"&nocache=\"\+ Math.random() * 1000000;");
            client.println("var request = new XMLHttpRequest();");
            client.println("request.onreadystatechange = function() {");
            client.println("if (this.readyState == 4) {");
            client.println("if (this.status == 200) {");
            client.println("if (this.responseText != null) {");            
            client.println("if (this.responseText.indexOf(\"LED1ON\") > -1){document.getElementById(\"LED1\").style.fill = \"yellow\";}");
            client.println("else {document.getElementById(\"LED1\").style.fill = \"black\";}");
            client.println("if (this.responseText.indexOf(\"LED2ON\") > -1){document.getElementById(\"LED2\").style.fill = \"yellow\";}");
            client.println("else {document.getElementById(\"LED2\").style.fill = \"black\";}");
            client.println("if (this.responseText.indexOf(\"S1:ON\") > -1){document.getElementById(\"SW1\").style.fill = \"red\";document.getElementById(\"text1\")\.innerHTML =\"SW1:ON\";}");
            client.println("else {document.getElementById(\"SW1\").style.fill = \"white\"; document.getElementById(\"text1\")\.innerHTML =\"SW1:OFF\";}");
            client.println("}}}}");
            client.println("request.open(\"GET\", \"ajax_switch\" + nocache, true);");
            client.println("request.send(null);");
            client.println("setTimeout('GetSwitchState()', 1000);");
            client.println("}");
            client.println("function SetLEDStates(num){");
            client.println("switch(num){");
            client.println("case 1: if(x1==1){led1=\"&LED1ON\";x1=0;}else{led1=\"&LED1OFF\";x1=1;}break;");
            client.println("case 2: if(x2==1){led2=\"&LED2ON\";x2=0;}else{led2=\"&LED2OFF\";x2=1;}break;");
            client.println("case 0: led1=\"&LED1OFF\";x1=1; led2=\"&LED2OFF\";x2=1; break;");
            client.println("}}");
            client.println("</script>");
            client.println("</head>");
            client.println("<body onload=\"GetSwitchState();SetLEDStates(0);\"><center>");
            client.println("<h1>Erhan YILMAZ ESP8266 Web Server Application!</h1>");
            client.println("<div id=\"text1\"></div>");
            client.println("<svg width=\"40\" height=\"40\"><circle id=\"SW1\" cx=\"20\" cy=\"20\" r=\"10\"stroke=\"black\" stroke-width=\"4\" fill=\"white\"/></svg>");
            client.println("<div id=\"text2\"></div>");
            client.println("<p><button type=\"L1\" onclick=\"SetLEDStates(1);\">LED1</button>&nbsp;");
            client.println("<button type=\"L2\" onclick=\"SetLEDStates(2);\">LED2</button>&nbsp;</p>");
            client.println("<p><center>");
            client.println("<svg width=\"50\" height=\"50\"><circle id=\"LED1\" cx=\"20\" cy=\"20\" r=\"10\"stroke=\"green\" stroke-width=\"4\" fill=\"black\"/></svg>");
            client.println("<svg width=\"50\" height=\"50\"><circle id=\"LED2\" cx=\"20\" cy=\"20\" r=\"10\"stroke=\"green\" stroke-width=\"4\" fill=\"black\"/></svg>");
            client.println("</center></p>");
            client.println("</center></body>");
            client.println("</html>");
          }
          // display received HTTP request on serial port
          Serial.print(HTTP_req);
          HTTP_req = "";            // finished with request, empty string
          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n')
        {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        } 
        else if (c != '\r')
        {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

// send the state of the switch to the web browser
void GetSwitchState(WiFiClient cl)
{ 
  char x=0;

  if (digitalRead(PUSH1))
    cl.println("S1:OFF");
  else
    cl.println("S1:ON");

}

void SetLEDStates(WiFiClient cl, String HTTP_req)
{

  if (HTTP_req.indexOf("LED1ON") > -1)
  {
    digitalWrite(LED1, HIGH); 
    cl.println("LED1ON");
  }
  else
  {
    digitalWrite(LED1, LOW); 
    cl.println("LED1OFF");
  }

  if (HTTP_req.indexOf("LED2ON") > -1)
  {
    digitalWrite(LED2, HIGH); 
    cl.println("LED2ON");
  }
  else
  {
    digitalWrite(LED2, LOW); 
    cl.println("LED2OFF");
  }

}

