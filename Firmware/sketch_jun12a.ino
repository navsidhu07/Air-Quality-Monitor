#include <DHT.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WebServer.h>

// Web Server Credentials:

const char* ssid = "SpectrumSetup-83";
const char* password = "greentrain069";

// Outputs

String tempReading = "";
String humReading = "";
String ppmReading = "";
String qualityReading = "";

// Web server object

WebServer server(80);

// HTML + CSS + JavaScript dashboard

void handleRoot()
{
  String html = "<!DOCTYPE html>"
    "<html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Air Quality Monitor</title>"
    "<style>"
      "body{"
        "font-family: Arial, sans-serif;"
        "background:#f2f2f2;"
        "text-align:center;"
        "padding:30px;"
      "}"
      ".card{"
        "background:#fff;"
        "max-width:360px;"
        "margin:0 auto;"
        "padding:20px;"
        "border-radius:10px;"
        "box-shadow:0 2px 8px rgba(0,0,0,0.15);"
      "}"
      "h2{margin-top:0;}"
      "#clock{"
        "font-size:16px;"
        "color:#555;"
        "margin-bottom:20px;"
      "}"
      ".reading{"
        "font-size:18px;"
        "margin:10px 0;"
        "text-align:left;"
        "padding-left:20px;"
      "}"
      ".value{"
        "font-weight:bold;"
      "}"
      "#quality{"
        "font-weight:bold;"
        "padding:6px;"
        "border-radius:6px;"
        "display:inline-block;"
      "}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='card'>"
      "<h2>ESP32 Air Quality Monitor</h2>"
      "<div id='clock'>--</div>"
      "<div class='reading'>Temperature: <span class='value'><span id='temp'>--</span> &deg;C</span></div>"
      "<div class='reading'>Humidity: <span class='value'><span id='hum'>--</span> %</span></div>"
      "<div class='reading'>Gas Level (PPM): <span class='value' id='ppm'>--</span></div>"
      "<div class='reading'>Air Quality: <span id='quality'>--</span></div>"
    "</div>"
    "<script>"
    "function updateClock(){"
      "document.getElementById('clock').innerText = new Date().toLocaleString();"
    "}"
    "setInterval(updateClock, 1000);"
    "updateClock();"

    "function updateColor(q){"
      "var el = document.getElementById('quality');"
      "el.innerText = q;"
      "if(q === 'GOOD'){ el.style.background = '#c8f7c5'; el.style.color = '#1a7a1a'; }"
      "else if(q === 'MODERATE'){ el.style.background = '#fff3b0'; el.style.color = '#8a6d00'; }"
      "else if(q === 'BAD'){ el.style.background = '#f7c5c5'; el.style.color = '#a10000'; }"
      "else { el.style.background = '#eee'; el.style.color = '#555'; }"
    "}"

    "setInterval(function(){"
      "fetch('/data')"
      ".then(r => r.json())"
      ".then(d => {"
        "document.getElementById('temp').innerText = d.temp;"
        "document.getElementById('hum').innerText = d.hum;"
        "document.getElementById('ppm').innerText = d.ppm;"
        "updateColor(d.quality);"
      "});"
    "}, 3000);"
    "</script>"
    "</body></html>";
  server.send(200, "text/html", html);
}

// JSON data endpoint

void handleData()
{
  String json = "{";
  json += "\"temp\":" + tempReading + ",";
  json += "\"hum\":" + humReading + ",";
  json += "\"ppm\":" + ppmReading + ",";
  json += "\"quality\":\"" + qualityReading + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

////////////////////////////////////////////////////////////////////

// DHT pin# and type
#define DHTPIN 23
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LCD pin assignments
const int rs = 19, en = 18, d4 = 17, d5 = 16, d6 = 4, d7 = 21;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Boolean flag

bool error = false;

// LED pins
#define RED_PIN 25
#define YELLOW_PIN 26
#define GREEN_PIN 27

// MQ135
#define MQ 36
String quality = "";

// Reprinting LCD

void rePrint()
{
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
}

// Assign a level to PPM range

String airQuality(int &num, String &line)
{
  if(num < 5000)
  {
    line = "GOOD";
    // LED GREEN
  }
  else if (num >= 5000 && num <= 8000)
  {
    line = "MODERATE";
    // LED YELLOW
  }
  else if (num > 8000)
  {
    line = "BAD";
    // LED RED
  }

  return line;
}

// LED control

void ledControl(String &quality)
{
  // LED control - LEDs stay on for whole program

  if (quality == "GOOD")
  {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
  }

  if (quality == "MODERATE")
  {
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }

  if (quality == "BAD")
  {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  }
}

void setup() 
{
  // initialize serial monitor

  Serial.begin(115200);

  // Connect to WiFi network

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Web Server Setup

  server.on("/", handleRoot);
  server.on("/data", handleData);

  // Start the web server

  server.begin();
  Serial.println("HTTP server started");

  // initialize LCD

  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");

  // initialize DHT

  dht.begin();

  // initialize LEDS

  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  
}

void loop() 
{
  server.handleClient();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  tempReading = String(temp, 1);
  humReading = String(hum, 1);

  int gasLevel = analogRead(MQ);
  ppmReading = String(gasLevel);
  Serial.println(gasLevel);

  // verify valid temp and humidity

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Invalid temperature or humidity");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Error");
    error = true;
    return;
  }

  if (error)
  {
    rePrint();
  }
  error = false;

  // Call airQuality for air quality reading

  airQuality(gasLevel, quality);
  qualityReading = quality;

  // LED control - at least 1 led stays on for whole program

  ledControl(quality);

  // Display temp and humidity on LCD for 3 seconds

  lcd.setCursor(7, 0);
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print ("C");

  lcd.setCursor(7, 1);
  lcd.print(hum);
  lcd.print("%");
  delay(3000);
  
  // Display air quality reading for 3 seconds

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Air Quality:");
  lcd.setCursor(7,1);
  lcd.print(quality);
  delay(3000);
  lcd.clear();
  rePrint();
}