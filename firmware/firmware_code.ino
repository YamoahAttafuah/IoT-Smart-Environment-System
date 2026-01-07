// relevant libraries
#include <DHT22.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <String.h>
#include "LCDIC2.h"
#include "FS.h"
#include "SPIFFS.h"
#include "html.h"

// macro definitions
#define pinDATA (18)
#define FAN (19)
#define LDR 32
#define HEART (23)
#define MSG_BUFFER_SIZE  (50)

// global objects
DHT22 dht22(pinDATA);
LCDIC2 lcd(0x27, 16, 2);
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// global variables
// time variables
uint32_t currTime;
uint32_t last_temp_light_read_time = 0;
uint32_t last_humidity_read_time = 0;
uint32_t last_heartbeat = 0;
uint32_t lcd_print_time = 0;
uint32_t last_save_time = 0;
uint32_t heart_rate = 2000;
uint32_t ip_time = 0;
uint32_t last_wifi_check = 0;
uint32_t last_post_time = 0;

// sensor readings
float t, h;
int temp_entries = 0;
int hum_entries = 0;
float LDRreading = 0;

// network variables
String ssidAP = "YOUR_AP_SSID"; // modify
String passwordAP = "";
String ssid = "YOUR_WIFI_SSID";
String password = "";
String serverName = "http://172.20.10.4/iot/insert.php";
//String serverName = "http://192.168.2.2/iot/insert.php";
IPAddress local_ip(192,168,2,1); 
IPAddress gateway(192,168,2,1); 
IPAddress subnet(255,255,255,0); 

//---- MQTT Broker settings
const char* mqtt_server = "mqtt-dashboard.com";
const int mqtt_port =8884;
char msg[MSG_BUFFER_SIZE];
const char* ldr_topic= "fp/ldr";
const char* temperature_topic= "fp/temperature";
const char*  humidity_topic= "fp/humidity";
const char*  control_topic= "fp/control";
const char*  protocol_topic= "fp/protocol";


// arrays for data. Store entries for  past one minute
float humidity[20];
float temps[10];
float light_intensity[10];
String split_temp[10];
String split_hum[20];
String split_lit[10];
String configs[5];

// array pointer position
uint8_t temps_ptr = 0, hum_ptr = 0;

// control variables
uint8_t wifi_mode = 0; // 0 for STA, 1 for AP
uint8_t auto_fan = 0;
uint8_t trigger_temperature = 26;
uint8_t post = 1; // 0 - MQTT, 1 - HTTP

// smart object config info
String son = "ESP32ALLOY"; // S-smart, O-Object, N-name
String location = "HangOut";

//================================================
// SPIFFS Fubctions
String readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    String tmp = "";
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return String("error");
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
        tmp += file.read();
    }
    file.close();
    return tmp;
}

//================================================
// write message to file in SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

//====================================================================
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

//================================================
// read light and temperature every 6 seconds
void read_light_and_temperature() {
  if (currTime - last_temp_light_read_time >= 6000) {
    // read temperature
    // store temp reading in flash
    // post to database
    t = dht22.getTemperature();
    temps[temps_ptr] = t;
    if (post){
      post_to_db(json_format(String(t), String("Temperature"))); 
    }
    else {
      publishMessage(temperature_topic,String(mqtt_format(String(t), String("Temperature"))),true);
    }

    float resistance = 0, lux = 0;
    LDRreading = analogRead(LDR);
    resistance = (4095*33000/(LDRreading))-33000;
    lux = (2.5 * pow(10,7)) * pow(resistance,-1.4059);
    light_intensity[temps_ptr] = lux;
    Serial.println(LDRreading);

    // update last read time
    if (post){
      post_to_db(json_format(String(lux), String("LDR"))); 
    }
    else {
      publishMessage(ldr_topic,String(mqtt_format(String(lux), String("LDR"))),true);
    }
    last_temp_light_read_time = currTime;
    lcd_print("Tem:", t, 0, 0);
    lcd_print("LDR:", lux, 8, 0);
    temps_ptr = (temps_ptr >= 9)?0:temps_ptr+1;
    temp_entries++;
  }
}

//================================================
// read humidity every 3 seconds and publish to database
void read_humidity() {
  if (currTime - last_humidity_read_time >= 3000) {
    // read humidity
    // store temp reading in flash
    // post to database
    h = dht22.getHumidity();
    humidity[hum_ptr] = h;
    hum_ptr = (hum_ptr >= 19)?0:hum_ptr+1;
    lcd_print("Hum: ", h, 0, 1);
    last_humidity_read_time = currTime;
    hum_entries++;
    if (post){
      post_to_db(json_format(String(h), String("Humidity"))); 
    }
    else {
      publishMessage(humidity_topic,String(mqtt_format(String(h), String("Humidity"))),true);
    }
  }
}

//================================================
// format as json
String json_format(String reading, String sensorName){
  String tmp;
  tmp = "{\"SensorName\":\"";
  tmp += "\""+sensorName+"\",";
  tmp += "\"SensorReading\":\"";
  tmp += String(reading) + "\",";
  tmp += "\"Location\":\"";
  tmp += location + "\",";
  tmp += "\"SmartObject\":\"";
  tmp += son + "\"}";
  return tmp;
}

//================================================
// blink LED every 2 seconds
void heart_beat() {
  if (currTime - last_heartbeat >= heart_rate) {
    digitalWrite(HEART, !digitalRead(HEART));
    last_heartbeat = currTime;
  }
}

//================================================
// update heart rate
void heart_rate_change(){
  if (server.hasArg("heartRate")){
    Serial.println(server.arg("heartRate"));
    server.send(200, "text/plain", server.arg("heartRate"));
    heart_rate = server.arg("heartRate").toInt();
    writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
  }
  else{
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

//================================================
void updateFanTrigger(){
  if (server.hasArg("temp")){
    Serial.println(server.arg("temp"));
    server.send(200, "text/plain", server.arg("temp"));
    trigger_temperature = server.arg("temp").toInt();
    //    writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
  }
  else {
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

//================================================
// turn fan ON if temperature exceeds 26
void fanAuto() {
  (t > trigger_temperature) ? digitalWrite(FAN, 1) : digitalWrite(FAN, 0);
}

//================================================
// manual fan control from webpage
void fanON(){
  digitalWrite(FAN, 1);
  server.send(200, "text/plain","FAN ON");
}

void fanOFF(){
  digitalWrite(FAN, 0);
  server.send(200, "text/plain","FAN OFF");
}

//================================================
// switch fan control from auto to manual and vice versa
void fan_control_switch(){
  Serial.println("Fan control switch");
  auto_fan = !(auto_fan);
  (auto_fan)?server.send(200, "text/plain","Fan Mode: AUTO"):server.send(200, "text/plain","Fan Mode: MANUAL");
}

//================================================
void lcd_print(char* label, float reading, int row, int col) {
  lcd.setCursor(row, col);
  lcd.print(label);
  lcd.setCursor(row+4, col);
  lcd.print(String(reading)); // 4 chars
  lcd_print_time = currTime;
}

//================================================
// store to flash
void store_to_flash(){
  if (currTime - last_save_time >= 60000){
    String save = "Temperature:";
    for (int i = 10; i > 0; i--){
      save += String(temps[i-1]);
      save = (i != 1)? save+",":save+"\n";
    }
    save += "Humidity:";
    for (int i = 20; i > 0; i--){
      save += String(humidity[i-1]);
      save = (i != 1)? save+",":save+"\n";
    }
    save += "Light Intensity:";
    for (int i = 10; i > 0; i--){
      save += String(light_intensity[i-1]);
      save = (i != 1)? save += ",":save+'\n';
    }
    writeFile(SPIFFS, "/sensorData.txt", save.c_str());
    last_save_time = currTime;
  }
}

//================================================
// change smart object name
void change_son(){
  if (server.hasArg("son")){
    Serial.println("New smart object name: " + server.arg("son"));
    son = server.arg("son");
    server.send(200, "text/plain", son);
    writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
  }
  else{
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

void change_location(){
  if (server.hasArg("location")){
    Serial.println("New Location: " + server.arg("location"));
    location = server.arg("location");
    server.send(200, "text/plain", location);
    writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
  }
  else{
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

//================================================
// display IP when in AP mode
void display_ip(){
  if (currTime - ip_time >= 10000){
    lcd_print("IP: ", WiFi.softAPIP(), 0, 0); 
  }
}

//================================================
// nwtwork and web functions
void base(){
  server.send(200, "text/html", home_page);
}

//================================================
// display readings page
void display_readings(){
  server.send(200, "text/html", display_page);
}

//================================================
// load settings page
void settings(){
  Serial.println("Settings clicked");
  server.send(200, "text/html", settings_page);
}

//================================================
// handle wifi connection from client-provided credentials
void handle_reconnection(){
  Serial.println("Handling connection");
  if (server.hasArg("ssid") && server.hasArg("pass")){
    Serial.println(server.arg("ssid"));
    Serial.println(server.arg("pass"));
    ssid = server.arg("ssid");
    password = server.arg("pass");
    writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
    if (station_mode() == 1){
      server.send(200, "text/plain", "Connected!");
    }
    else server.send(400, "text/plain", "Connection Failed!");
  }
  else{
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

//================================================
void updateWiFiCredentials(){
  Serial.println("Editing Credentials");
  if (server.hasArg("ssid") && server.hasArg("pass")){
    Serial.println(server.arg("ssid"));
    Serial.println(server.arg("pass"));
    ssidAP = server.arg("ssid");
    passwordAP = server.arg("pass");
    if (station_mode() == 1){
      server.send(200, "text/plain", "Connected!");
      writeFile(SPIFFS, "/config.txt", (String("ssid:")+ssid+String("\npassword:")+password+String("\nSON:")+son+String("\nHeartrate:")+String(heart_rate)+String("\nLocation:")+location).c_str());
    }
    else server.send(400, "text/plain", "Credentials Edited!");
  }
  else{
    server.send(400, "text/plain", "Incomplete form. Enter all data");
  }
}

//================================================
// return last five entries as html text for client
void last_five(){
  String lastFive = "<tr><td>Temperature</td><td>Humidity</td><td>Light Intensity</td></tr>";
  if (temp_entries < 5){
    for (int i = temp_entries; i >0; i++){
      lastFive += "<tr><td>" + String(temps[i-1]) + "</td>";
      lastFive += "<td>" + String(humidity[(i*2)-1]) + "</td>";
      lastFive += "<td>" + String(light_intensity[i-1]) + "</td></tr>";
    }
  }
  else{
    int ct = 0;
    int tmp_ctr = temps_ptr;
    int hm_ctr = hum_ptr;
    while(ct < 5){
      lastFive += "<tr><td>" + String(temps[tmp_ctr-1]) + "</td>";
      lastFive += "<td>" + String(humidity[(hm_ctr)-1]) + "</td>";
      lastFive += "<td>" + String(light_intensity[tmp_ctr-1]) + "</td></tr>";
      ct++;
      tmp_ctr = (tmp_ctr <= 0)?(sizeof(temps)/sizeof(float))-1:(tmp_ctr-1);
      hm_ctr = (hm_ctr <= 0)?(sizeof(humidity)/sizeof(float))-1:(hm_ctr-1);
    }
  }
  server.send(200, "text/plain", lastFive);
}

//================================================
// return last data minute from flash
//void last_min(){
//  retrieve_from_flash();
//  String response = "<tr>";
//  response += "<td>Temperature</td>";
//  response += "<td>Light Intensity</td>";
//  response += "<td>Humidity</td>";
//  response += "<td>More Humidty</td>";
//  response += "</tr>";
//  for (int i = 0; i < 10; i++){
//    response += "<tr>";
//    response += "<td>" + split_temp[i] + "</td>";
//    response += "<td>" + split_lit[i] + "</td>";
//    response += "<td>" + split_hum[2*i] + "</td>";
//    response += "<td>" + split_hum[2*i+1] + "</td>";
//    response += "</tr>";
//  }
//  server.send(200, "text/html", response);
//}

//================================================
void last_min(){
  retrieve_from_flash();

  // Initialize response string
  String response = "<table>"; // Start of table

  // Add table header row
  response += "<tr>";
  response += "<th>Temperature</th>";
  response += "<th>Light Intensity</th>";
  response += "<th>Humidity 1</th>";
  response += "<th>Humidity 2</th>";
  response += "</tr>";

  // Add table rows for each entry
  for (int i = 0; i < 10; i++){
    // Check if data is available at index i
    if (i < 10 && i < 10 && (2*i+1) < 20) {
      response += "<tr>";
      response += "<td>" + split_temp[i] + "</td>";
      response += "<td>" + split_lit[i] + "</td>";
      response += "<td>" + split_hum[2*i] + "</td>";
      response += "<td>" + split_hum[2*i+1] + "</td>";
      response += "</tr>";
    }
  }

  response += "</table>"; // End of table

  // Send HTTP response with the table
  server.send(200, "text/html", response);
}

//================================================
// configure smart object as station
int station_mode(){
  // STA mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int num_connection_attempts = 0;
  Serial.println("In station");
  while (WiFi.status() != WL_CONNECTED)
  {
    num_connection_attempts++;
    delay(1000);
    Serial.print(".");
    if (num_connection_attempts > 19){
      // start softAP mode
      ap_mode();
      return -1;
    }
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  wifi_mode = 0;
  return 1;
}

//================================================
// setup ESP32 as Access Point
void ap_mode(){
  WiFi.mode(WIFI_AP);
  delay(1000);
  WiFi.softAP(ssidAP, passwordAP); 
  WiFi.softAPConfig(local_ip, gateway, subnet);  // initialise Wi-Fi 
  server.begin();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", base);
  server.on("/reconnect", HTTP_GET, handle_reconnection);
  server.on("/fanmode", fan_control_switch);
  server.on("/heartpulse", HTTP_GET, heart_rate_change);
  server.on("/son", HTTP_GET, change_son);
  server.on("/settings", settings);
  server.on("/last_five", last_five);
  server.on("fanON", fanON);
  server.on("fanOFF", fanOFF);
  server.on("/temperature", updateFanTrigger);
  server.on("/display", display_readings);
  server.on("/last_min", last_min);
  server.on("/editESPWiFi",HTTP_GET, updateWiFiCredentials);
  server.on("/post", setPost);
  server.on("/mqtt", setMqtt);
  server.on("/location", HTTP_GET, change_location);
  wifi_mode = 1;
}

//================================================
void split(String str_temp, String str_hum, String str_lit){
  String tmp;
  int arr_idx = 0;
  for (int i = 0; i < str_temp.length(); i++){
    tmp = "";
    while(str_temp[i] != ','){
      if (i == str_temp.length()) break;
      tmp += str_temp[i];
      i++;
    }
    split_temp[arr_idx] = tmp;
    arr_idx++;
  }

  arr_idx = 0;
  for (int i = 0; i < str_hum.length(); i++){
    tmp = "";
    while(str_hum[i] != ','){
      if (i == str_hum.length()) break;
      tmp += str_hum[i];
      i++;
    }
    split_hum[arr_idx] = tmp;
    arr_idx++;
  }
  
  arr_idx = 0;
  for (int i = 0; i < str_lit.length(); i++){
    tmp = "";
    while(str_lit[i] != ','){
      if (i == str_lit.length()) break;
      tmp += str_lit[i];
      i++;
    }
    split_lit[arr_idx] = tmp;
    arr_idx++;
  }
}

//================================================
void retrieve_from_flash(){
  File file = SPIFFS.open("/sensorData.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  String file_data = file.readString();
  Serial.println("File Content:");
  Serial.println(file_data);
  file.close();
  // keys and values
  String keys[3];
  String values[3];
  // iterator indexes
  int k = 0, l = 0;
  
  for (int i = 0; i < file_data.length(); i++){
    String tmp = "";
    int j = i;
    while (file_data[j] != ':'){
      tmp += file_data[j];
      j++;
    }
    keys[k] = tmp;
    Serial.println(keys[k]);
    j = j+1;
    tmp = "";
    while (file_data[j] != '\n'){
      tmp += file_data[j];
      j++;
    }
    values[k] = tmp;
    Serial.println(values[k]);
    k++;
    i = j;
  }
  // split last 1 minute data into strings
  split(values[0], values[1], values[2]);
}

//================================================
// collect and format data from flash
void json_format_and_post(){
  retrieve_from_flash();
  String tmp_t;
  String tmp_h1;
  String tmp_h2;
  String tmp_l;
  for (int i = 0; i < 10; i++){
    tmp_t = "{\"SensorName\":\"";
    tmp_t += "Temperature\",";
    tmp_t += "\"SensorReading\":\"";
    tmp_t += split_temp[i] + "\",";
    tmp_t += "\"Location\":\"";
    tmp_t += location + "\",";
    tmp_t += "\"SmartObject\":\"";
    tmp_t += son + "\"}";
    
    tmp_l = "{\"SensorName\":\"";
    tmp_l += "LDR\",";
    tmp_l += "\"SensorReading\":\"";
    tmp_l += split_lit[i]  +"\",";
    tmp_l += "\"Location\":\"";
    tmp_l += location + "\",";
    tmp_l += "\"SmartObject\":\"";
    tmp_l += son + "\"}";
    
    tmp_h1 = "{\"SensorName\":\"";
    tmp_h1 += "Humidity\",";
    tmp_h1 += "\"SensorReading\":\"";
    tmp_h1 += split_hum[2*i]  +"\",";
    tmp_h1 += "\"Location\":\"";
    tmp_h1 += location + "\",";
    tmp_h1 += "\"SmartObject\":\"";
    tmp_h1 += son + "\"}";
    
    tmp_h2 = "{\"SensorName\":\"";
    tmp_h2 += "Humidity\",";
    tmp_h2 += "\"SensorReading\":\"";
    tmp_h2 += split_hum[2*i+1]  +"\",";
    tmp_h2 += "\"Location\":\"";
    tmp_h2 += location + "\",";
    tmp_h2 += "\"SmartObject\":\",";
    tmp_h2 += son + "\"}";
    Serial.println(tmp_h1);
    Serial.println(tmp_h2);
    Serial.println(tmp_t);
    Serial.println(tmp_l);
    // post all to database
    post_to_db(tmp_h1);
    post_to_db(tmp_h2);
    post_to_db(tmp_t);
    post_to_db(tmp_l);
  }
} 

String mqtt_format(String reading, String sensorName){
  String tmp;
  tmp = "SensorName:";
  tmp += sensorName + ",";
  tmp += "SensorReading:";
  tmp += String(reading) + ",";
  tmp += "Location:";
  tmp += location + ",";
  tmp += "SmartObject:";
  tmp += son;
  return tmp;
}

//================================================
// post to database
void post_to_db(String string){
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
//    http.begin(client, serverName);
      http.begin(serverName);
      
      // For HTTP request with a content type: application/json:
      http.addHeader("Content-Type", "application/json");      
      int httpResponseCode = http.POST(string);
      
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

//================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);  //you could make this static
    // Attempt to connect
    if (client.connect(clientId.c_str())){//, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(control_topic);   // subscribe the topics here
      client.subscribe(protocol_topic);   // subscribe the topics here
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
    return;
  }
}

//================================================
void setPost(){
  Serial.println("POST MODE");
  post = 1;
  server.send(200, "text/plain", "POST MODE");
}

//================================================
void setMqtt(){
  Serial.println("POST MODE");
  post = 0;
  server.send(200, "text/plain", "MQTT MODE");
}

//=======================================  
// This void is called every time we have a message from the broker
//
void callback(char* topic, byte* payload, unsigned int length) {
  String incomingMessage = "";
  for (int i = 0; i < length; i++) incomingMessage += (char)payload[i];
  Serial.println("Message arrived ["+String(topic)+"]"+incomingMessage);
 
 //--- check the incomming message
  if(strcmp(topic,control_topic) == 0) {
    if (incomingMessage.equals("0")) { // AP
      if (wifi_mode != 0) ap_mode();
    } 
    else if (incomingMessage.equals("1")) { // Station
      if (wifi_mode != 1) ap_mode();
    }
 }
  if(strcmp(topic,protocol_topic) == 0) {
    if (incomingMessage.equals("0")) { // MQTT
      if (post != 0) post = 0;
    } 
    else if (incomingMessage.equals("1")) { // HTTP
      if (post != 1) post = 1;
    }
 }
}

//================================================
// publish as string
void publishMessage(const char* topic, String payload , boolean retained){
  if (!client.connected()) reconnect();
  else client.loop();
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}

// read config file and update esp variables
void configure(){
  File file = SPIFFS.open("/config.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  String conf = file.readString();
  
  if (conf == "error"){
    Serial.println("Error opening configuration file");
    return;
  }
  if (conf == ""){
    Serial.println("No config data yet. Update using web interface");
    return;
  }
  // writeFile(SPIFFS, "/config.txt", 
// ( String("ssid:")+ssid+
//   String("\npassword:")+password+
//   String("\nSON:")+son+String("\nHeartrate:")+
  // String(heart_rate)+
//   String("\nLocation:")+location).c_str());
  for (int i = 0; i < conf.length(); i++){
    String tmp = "";
    static int j = 0;
    if (conf[i] == ':'){
      i++;
      while(conf[i] != '\n'){
        if (i >= conf.length()) break;
        tmp += conf[i];
        i++;
      } 
      configs[j] = tmp;
      Serial.println(configs[j]);
      j++;
      i++;
    }
  }
  Serial.println("SSID");
  ssid = configs[0];
  Serial.println("Password");
  password = configs[1];
  Serial.println("Son");
  son = configs[2];
  Serial.println("Heart rate");
  heart_rate = configs[3].toInt();
  if (configs[4] != "") location = configs[4];
  Serial.println("Done configuring");
}

//================================================
void setup() {
  // Initialise serial monitor
  Serial.begin(115200);

  // pin initialisations
  pinMode(LDR, INPUT);
  pinMode(HEART, OUTPUT);
  pinMode(FAN, OUTPUT);
  
  lcd.begin();
  if (lcd.begin()) Serial.println("Hello, World!");

  // SPIFFS setup
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  // configure here
  configure();
  // attempt connecting to wifi
  station_mode();
  
  // OTA Init
  ArduinoOTA.setHostname("NNA_ESP32");
  ArduinoOTA.setPassword("pass");
  ArduinoOTA.begin();   // initialise ArduinoOTA
  
  // MQTT
  client.setServer(mqtt_server, 1883);//mqtt_port
  client.setCallback(callback);
  
  currTime = millis();
  
}

//================================================
void loop() {
  LDRreading = analogRead(LDR);
  ArduinoOTA.handle();
  server.handleClient();
  // put your main code here, to run repeatedly:
  currTime = millis();
  read_light_and_temperature();
  read_humidity();
  heart_beat();
  if (auto_fan) fanAuto();
  store_to_flash();
  if (wifi_mode) display_ip();
}
