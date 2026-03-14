#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <time.h>

WebServer server(80);

const char* DB_HOST = "192.168.164.122";
const uint16_t DB_PORT = 8000;
const char* DB_PATH = "/insert";

typedef struct struct_message {
  unsigned long ms;
  int raw;
  bool bright;
} struct_message;

struct_message incomingData;

int currentRaw = 0;
bool currentBright = false;
String currentState = "-";
String lastTimestamp = "-";

volatile bool dbSendPending = false;
int pendingRaw = 0;
bool pendingBright = false;
String pendingTimestamp = "-";

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "-";
  }

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

bool testTcpConnection() {
  WiFiClient client;
  bool ok = client.connect(DB_HOST, DB_PORT);
  if (ok) {
    client.stop();
    return true;
  }
  return false;
}

void sendToDB(int raw, String timestamp, bool bright) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("DB: WLAN nicht verbunden");
    return;
  }

  if (!testTcpConnection()) {
    Serial.println("DB: Server/Port nicht erreichbar");
    return;
  }

  // URL-Encoding fuer Leerzeichen
  timestamp.replace(" ", "%20");

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(DB_HOST) + ":" + String(DB_PORT) + String(DB_PATH);
  url += "?sensor=" + String(raw);
  url += "&timestamp=" + timestamp;
  url += "&bright=" + String(bright ? 1 : 0);

  Serial.print("DB URL: ");
  Serial.println(url);

  http.setConnectTimeout(5000);
  http.setTimeout(5000);

  if (!http.begin(client, url)) {
    Serial.println("HTTP begin fehlgeschlagen");
    return;
  }

  int code = http.GET();

  Serial.print("DB HTTP Code: ");
  Serial.println(code);

  if (code > 0) {
    Serial.print("DB Antwort: ");
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP Fehler: ");
    Serial.println(http.errorToString(code));
  }

  http.end();
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingDataBytes, int len) {
  if (len != sizeof(struct_message)) {
    Serial.println("Falsche Datenlaenge empfangen");
    return;
  }

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  currentRaw = incomingData.raw;
  currentBright = incomingData.bright;
  currentState = currentBright ? "HELL" : "DUNKEL";
  lastTimestamp = getTimeString();

  pendingRaw = currentRaw;
  pendingBright = currentBright;
  pendingTimestamp = lastTimestamp;
  dbSendPending = true;

  Serial.println("=== Neue ESP-NOW Daten ===");
  Serial.print("RAW: ");
  Serial.println(currentRaw);
  Serial.print("Status: ");
  Serial.println(currentState);
  Serial.print("Zeit: ");
  Serial.println(lastTimestamp);
}

String getHtmlPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Sensorstation</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:30px;background:#f4f4f4;}";
  html += ".box{background:white;padding:20px;border-radius:12px;max-width:450px;box-shadow:0 0 10px rgba(0,0,0,0.1);}";
  html += "h1{margin-top:0;}";
  html += "</style>";
  html += "</head><body>";

  html += "<div class='box'>";
  html += "<h1>Sensorstation</h1>";
  html += "<p><b>Lichtwert:</b> " + String(currentRaw) + "</p>";
  html += "<p><b>Status:</b> " + currentState + "</p>";
  html += "<p><b>Zeit:</b> " + lastTimestamp + "</p>";
  html += "<p><b>IP:</b> " + WiFi.localIP().toString() + "</p>";
  html += "</div>";

  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHtmlPage());
}

void handleApiData() {
  String json = "{";
  json += "\"timestamp\":\"" + lastTimestamp + "\",";
  json += "\"sensor\":" + String(currentRaw) + ",";
  json += "\"bright\":" + String(currentBright ? 1 : 0) + ",";
  json += "\"status\":\"" + currentState + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleApiSensor() {
  String json = "{";
  json += "\"timestamp\":\"" + lastTimestamp + "\",";
  json += "\"sensor\":" + String(currentRaw);
  json += "}";

  server.send(200, "application/json", json);
}

void handleApiBright() {
  String json = "{";
  json += "\"timestamp\":\"" + lastTimestamp + "\",";
  json += "\"bright\":" + String(currentBright ? 1 : 0);
  json += "}";

  server.send(200, "application/json", json);
}

void testServer() {
  WiFiClient client;
  HTTPClient http;

  String testUrl = "http://" + String(DB_HOST) + ":" + String(DB_PORT) + "/";
  Serial.print("Teste Server: ");
  Serial.println(testUrl);

  http.begin(client, testUrl);
  int code = http.GET();

  Serial.print("Root Code: ");
  Serial.println(code);

  if (code > 0) {
    Serial.println(http.getString());
  } else {
    Serial.println(http.errorToString(code));
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  bool res = wm.autoConnect("ESP32-Setup");

  if (!res) {
    Serial.println("WLAN Verbindung fehlgeschlagen");
    ESP.restart();
  }

  Serial.println("WLAN verbunden");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Kanal: ");
  Serial.println(WiFi.channel());

  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");

  server.on("/", handleRoot);
  server.on("/api/data", handleApiData);
  server.on("/api/data/sensor", handleApiSensor);
  server.on("/api/data/bright", handleApiBright);
  server.begin();

  testServer();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Fehler");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Empfaenger bereit");
}

void loop() {
  server.handleClient();

  if (dbSendPending) {
    dbSendPending = false;
    sendToDB(pendingRaw, pendingTimestamp, pendingBright);
  }
}
