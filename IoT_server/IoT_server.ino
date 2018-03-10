#include <ESP8266mDNS.h>

#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>

#include <WiFiServerSecure.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiSTA.h>



#include <SPI.h>

#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>


// Definition of COLORS
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0

/*Connections to NodeMCU
  LED to 3V3
  SCK to D5
  SDA to D7
  A0/DC to D1
  RST to D2
  CS to D4
  GND to GND
  VCC to 3V3
*/

#define CS D4
#define DC D2
#define RST D1

// Declare an instance of the ILI9163
TFT_ILI9163C tft = TFT_ILI9163C(CS, RST, DC);

MDNSResponder mdns;

ESP8266WebServer server(80);

// WiFi parameters to be configured
const char* ssid = "...";
const char* password = "...";

//Check if header is present and correct
bool is_authentified(){
  Serial.println("Enter is authentified");
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin(){
  String msg;
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "root" ) // enter ur username and password you want
    {
      server.sendHeader("Location","/");
      server.sendHeader("Cache-Control","no-cache");
      server.sendHeader("Set-Cookie","ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;

      }

  msg = "Wrong username/password! try again.";
  Serial.println("Log in Failed");
  }
  String content = "<html><body style='background-color:MediumAquaMarine'><form action='/login' method='POST'><p  align ='center' style='font-size:300%;'><u><b><i>  Log In  </i></b></u></p><br>";
  content += " <p   align ='center' style='font-size:160%'><b> UserName:<input type='text' name='USERNAME' placeholder='user name' required></b></p><br>";
  content += "<p  align ='center' style='font-size:160%'><b>Password:<input type='password' name='PASSWORD' placeholder='password' required></b></p><br>";
  content += "<p  align ='center' style='font-size:160%'><input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "</p><br> </body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok
void handleRoot(){
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()){
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.send(301);
    return;
  }
  String content =  "<body style='background: #80c6f7'><h1 align ='center'><b><u><i><strong>HOME AUTOMATION</strong></i></u></b></h1><br><p align ='center'>Switch #1 <a href=\"switch1On\"><button>ON</button></a>&nbsp;<a href=\"switch1Off\"><button>OFF</button></a></p>";
  content += "<br><p  align ='center'>Switch #2 <a href=\"switch2On\"><button>ON</button></a>&nbsp;<a href=\"switch2Off\"><button>OFF</button></a></p>";
  content += "<br><p  align ='center'>Switch #3 <a href=\"switch3On\"><button>ON</button></a>&nbsp;<a href=\"switch3Off\"><button>OFF</button></a></p>";
  content += "<br><p><marquee direction='right'>Developed by   RUCHIR SHARMA </marquee></p>";
  content += "<br><br><br><br> <footer><p>Posted by: Ruchir Sharma</p><p><a href=\"mailto:someone@example.com\">ruchir1677@gmail.com</a>.</p></footer></body>"; 
  
   if (server.hasHeader("User-Agent")){
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
    
    
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

//no need authentification
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void setup() {
  tft.begin();
  tft.fillScreen();

  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works without need of authentification");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );


  server.on("/",[](){
   //
  });

  server.begin();
  Serial.println("HTTP server started");


  
}

void loop() {
  testText();

  server.handleClient();
  
  delay(500);
}

unsigned long testText() {

  tft.setCursor(10, 40);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.println("Fuck you bitch");
  tft.setCursor(10, 63);
  tft.println(WiFi.localIP());
}
