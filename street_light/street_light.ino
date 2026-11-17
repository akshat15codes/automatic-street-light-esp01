#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* AP_NAME = "StreetLight";
const char* AP_PASS = "12345678";      // minimum 8 characters

#define LDR_PIN 3     // GPIO3 (RX): LDR module DO
#define OUT_PIN 2     // GPIO2: relay / LED

#define DARK_LEVEL HIGH          // DO HIGH = dark on this module
#define OUTPUT_ACTIVE_LOW true   // GPIO2 LOW = light ON

enum Mode { AUTO, FORCE_ON, FORCE_OFF };
Mode mode = AUTO;
bool lightOn = false;

ESP8266WebServer server(80);

void applyLight(bool on) {
  if (on == lightOn) return;
  lightOn = on;
  bool pinHigh = OUTPUT_ACTIVE_LOW ? !on : on;
  digitalWrite(OUT_PIN, pinHigh ? HIGH : LOW);
  Serial.println(on ? "Light ON" : "Light OFF");
}

String modeName() {
  if (mode == AUTO) return "AUTO (follows LDR)";
  if (mode == FORCE_ON) return "FORCED ON";
  return "FORCED OFF";
}

void sendPage() {
  String html =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<meta http-equiv='refresh' content='3'>"
    "<style>body{font-family:sans-serif;text-align:center;margin-top:30px}"
    "a{display:inline-block;margin:8px;padding:14px 28px;background:#2a6;"
    "color:#fff;text-decoration:none;border-radius:8px;font-size:18px}"
    "</style></head><body>"
    "<h2>Street Light</h2>"
    "<p>Mode: <b>" + modeName() + "</b></p>"
    "<p>Sensor: <b>" + String(digitalRead(LDR_PIN) == DARK_LEVEL ? "Dark" : "Bright") + "</b></p>"
    "<p>Light: <b>" + String(lightOn ? "ON" : "OFF") + "</b></p>"
    "<a href='/auto'>AUTO</a><a href='/on'>ON</a><a href='/off'>OFF</a>"
    "</body></html>";
  server.send(200, "text/html", html);
}

void redirectHome() {
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);  // keeps RX (GPIO3) free
  pinMode(LDR_PIN, INPUT);
  pinMode(OUT_PIN, OUTPUT);
  digitalWrite(OUT_PIN, OUTPUT_ACTIVE_LOW ? HIGH : LOW);  // light OFF at start

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NAME, AP_PASS);
  Serial.println("\nHotspot started");
  Serial.print("WiFi name: "); Serial.println(AP_NAME);
  Serial.print("Open: http://"); Serial.println(WiFi.softAPIP());  // 192.168.4.1

  server.on("/", sendPage);
  server.on("/auto", []() { mode = AUTO; redirectHome(); });
  server.on("/on",   []() { mode = FORCE_ON; redirectHome(); });
  server.on("/off",  []() { mode = FORCE_OFF; redirectHome(); });
  server.begin();
}

void loop() {
  server.handleClient();

  bool isDark = (digitalRead(LDR_PIN) == DARK_LEVEL);
  if (mode == AUTO) applyLight(isDark);
  else if (mode == FORCE_ON) applyLight(true);
  else applyLight(false);

  delay(10);
}
