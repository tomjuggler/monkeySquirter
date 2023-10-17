//Monkey Squirter D1 mini code - see https://circusscientist.com for details
//to use the web buttons version switch on D1 mini and go to http://<tmpIP>/

//adapted from original below:
/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/
*********/
//this actually works!!!!!

// html version with colour indicators

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

// Replace with your network credentials
const char *ssid = "YourRouterName";
const char *password = "YourRouterPassword";

ESP8266WebServer server(80);

String webPage = "";

// two options for triggering relays, you could have more or less:
int relay_pin = D1;
int sensor = D3;

int state = LOW; // by default, no motion detected
int val = LOW;     // variable to store the sensor status (value)

boolean sensing = false;
boolean playing = false;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;



IPAddress tmpGateway(192, 168, 8, 1); //change to router IP
IPAddress tmpIP(192, 168, 8, 13); //you choose this depending on your router IP and configuration
IPAddress ipSubnet(255, 255, 255, 0);

void setup(void){
  //this is the webpage, with two buttons, one line per button:

  webPage += "<html><head><style>";
  webPage += "body { text-align: center; font-family: Arial, sans-serif; }";
  webPage += "h1 { font-size: 24px; margin-bottom: 20px; }";
  webPage += "p { font-size: 18px; }";
  webPage += ".button { display: inline-block; padding: 10px 20px; font-size: 18px;";
  webPage += "border: none; border-radius: 5px; cursor: pointer; margin: 5px; }";
  webPage += ".button-on { background-color: green; color: white; }";
  webPage += ".button-squirt { background-color: blue; color: white; }";
  webPage += ".button-off { background-color: red; color: white; }";
  webPage += "@media screen and (max-width: 1000px) {"; // Adjust for mobile screens
  webPage += "h1 { font-size: 40px; }";
  webPage += "p { font-size: 36px; }";
  webPage += ".button { padding: 50px 0; font-size: 36px; width: 80%; }"; // Expand buttons to 100% width
  webPage += "}";
  webPage += "</style></head><body>";
  webPage += "<h1>MONKEY SQUIRTER</h1>";
  webPage += "<br>";
  webPage += "<p><span>SQUIRTER:</span>";
  webPage += "<a href=\"socket1On\"><button class=\"button button-squirt\">SQUIRT</button></a>";
  webPage += "<a href=\"socket1Off\"><button class=\"button button-off\">STOP</button></a></p>";
  webPage += "<br>";
  webPage += "<p><span>MOTION SENSOR:</span>";
  webPage += "<a href=\"socket2On\"><button class=\"button button-on\">ACTIVATE</button></a>";
  webPage += "<a href=\"socket2Off\"><button class=\"button button-off\">DEACTIVATE</button></a></p>";
  webPage += "</body></html>";
  
  // preparing GPIOs
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);

  pinMode(sensor, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(1000);
  Serial.begin(115200);
  WiFi.config(tmpIP, tmpGateway, ipSubnet, tmpGateway);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP()))
  {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", []()
            { server.send(200, "text/html", webPage); });
  server.on("/socket1On", []()
            {
    server.send(200, "text/html", webPage);
    digitalWrite(LED_BUILTIN, LOW);   // turn LED ON
    digitalWrite(relay_pin, HIGH);
    delay(1000);
    //turn off again after some time?!
    digitalWrite(LED_BUILTIN, HIGH);   // turn LED OFF
    digitalWrite(relay_pin, LOW); });
  server.on("/socket1Off", []()
            {
    server.send(200, "text/html", webPage);
    digitalWrite(LED_BUILTIN, HIGH);   // turn LED OFF
    digitalWrite(relay_pin, LOW);
    delay(1000); });
  server.on("/socket2On", []()
            {
    server.send(200, "text/html", webPage);
    sensing = true; });
    Serial.println("Motion Detection activated");
  server.on("/socket2Off", []()
            {
    server.send(200, "text/html", webPage);
    sensing = false;
    Serial.println("Motion Detection deactivated");
    digitalWrite(LED_BUILTIN, HIGH);   // turn LED OFF
    digitalWrite(relay_pin, LOW); });
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void)
{
  server.handleClient();
  // val = digitalRead(sensor); // read sensor value  
  if (sensing)
  {
    currentMillis = millis();
    val = digitalRead(sensor); // read sensor value    
    if (currentMillis - previousMillis >= interval)
    {
      // val = digitalRead(sensor); // read sensor value  
      previousMillis = currentMillis;
      if (val == HIGH)
      { // check if the sensor is HIGH
        Serial.println("Motion detected");
        if (!playing)        
        {
          playing = true;
          // sensing = false; //test do once only
          digitalWrite(LED_BUILTIN, LOW); // turn LED ON
          digitalWrite(relay_pin, HIGH);
          delay(interval); // delay 100 milliseconds - todo: not working?
          digitalWrite(relay_pin, LOW);
          digitalWrite(LED_BUILTIN, HIGH); // turn LED OFF
          // if (state == LOW)
          // {
          //   Serial.println("Motion detected!");
          //   state = HIGH; // update variable state to HIGH
          // }
        }
      }
      else
      {
        if(playing){
          playing = false;
          Serial.println("DETECTING.....");
        }
        digitalWrite(relay_pin, LOW);
        digitalWrite(LED_BUILTIN, HIGH); // turn LED OFF
        // delay(1000);             // delay 200 milliseconds

        // if (state == HIGH)
        // {
        //   // Serial.println("Motion stopped!");
        //   digitalWrite(relay_pin, LOW);
        //   digitalWrite(LED_BUILTIN, HIGH); // turn LED OFF
        //   state = LOW;                     // update variable state to LOW
        // }
      }
    }
    else
    {
      // Serial.print(".");
      // Serial.println("currentMillis - previousMillis < interval ");
      digitalWrite(relay_pin, LOW);
      digitalWrite(LED_BUILTIN, HIGH); // turn LED OFF
      // state = LOW;                     // update variable state to LOW
    }
  }
  else
  {
  }
} 
