
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 -1

HardwareSerial Serial2(1);
StaticJsonDocument<200> doc;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "wifi2.4G_B9D228";
const char* password = "GMK170401007939";

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>BME680 Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #4B1D3F; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #0e7c7b; }
    .card.humidity { color: #17bebb; }
    .card.pressure { color: #3fca6b; }
    .card.gas { color: #d62246; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>BME280 AND MQ135 WEB SERVER</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURE</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> RAW MQ135</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span></span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-angle-double-down"></i> PRESSURE</h4><p><span class="reading"><span id="pres">%PRESSURE%</span> hPa</span></p>
      </div>
      <div class="card gas">
        <h4><i class="fas fa-wind"></i> PM2.5</h4><p><span class="reading"><span id="gas">%GAS%</span> K&ohm;</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i>RZERO</h4><p><span class="reading"><span id="rzero">%RZERO%</span></span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-angle-double-down"></i> ALTITUDE</h4><p><span class="reading"><span id="altitude">%PRESSURE%</span>m</span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);
 
 source.addEventListener('gas', function(e) {
  console.log("gas", e.data);
  document.getElementById("gas").innerHTML = e.data;
 }, false);
 
 source.addEventListener('rzero', function(e) {
  console.log("rzero", e.data);
  document.getElementById("rzero").innerHTML = e.data;
 }, false);

 source.addEventListener('altitude', function(e) {
  console.log("altitude", e.data);
  document.getElementById("altitude").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {

    if(Serial2.available() > 0){
      Serial.print("Data received");
      deserializeJson(doc, Serial2.readString());
      Serial.println(doc["raw"].as<int>());
      Serial.println(doc["rzero"].as<float>());
      Serial.println(doc["pm25"].as<float>());
      
      char tempResult[8];
      char pressResult[8];
      char pm25Result[8];
      char rzeroResult[8];
      char altitudeResult[8];
      char str[8];
      
      int raw = doc["raw"].as<int>();
      float rzero = doc["rzero"].as<float>();
      float pm25 = doc["pm25"].as<float>();

      float temperature = doc["temperature"].as<float>();
      float pressure = doc["pressure"].as<float>();
      float altitude = doc["altitude"].as<float>();

      events.send(dtostrf(temperature, 6, 2, tempResult),"temperature",millis());
      events.send(itoa(raw, str, 10),"humidity",millis());
      events.send(dtostrf(pressure, 6, 2, pressResult) ,"pressure",millis());
      events.send(dtostrf(pm25, 6, 2, pm25Result),"gas",millis());
      events.send(dtostrf(rzero, 6, 2, rzeroResult),"rzero",millis());
      events.send(dtostrf(altitude, 6, 2, altitudeResult),"altitude",millis());
    }
//    delay(10000);
  
}
