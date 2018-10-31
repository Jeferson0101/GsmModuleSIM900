#include<Arduino.h>

//Biblioteca para trabalhar com json
#include<ArduinoJson.h>

#include <SoftwareSerial.h>

//Este Define tem que estar antes de dar um include na biblioteca GSM
#define TINY_GSM_MODEM_SIM900
 
//Biblioteca GSM
#include <TinyGsmClient.h>

//Biblioteca para enviar dados mqtt
#include <PubSubClient.h>

//bibliotecas dht22
//#include <DHT_U.h>
//#include <Adafruit_Sensor.h>

//#define DHTPIN            8         // pino do DHT22
//#define DHTTYPE           DHT22     // Define qual DHT estará sendo usado

// Select your modem:



//DHT_Unified dht(DHTPIN, DHTTYPE);

//uint32_t delayMS; // variável que recebe o valor mínimo de delay

SoftwareSerial SerialAT(2, 3); // RX, TX

// Criando objeto Json 
  char payload[256];
  String strPayload;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
 
// Access point do seu provedor
const char apn[]  = "";
const char user[] = "";
const char pass[] = "";

// Broker e porta mqtt
const char* broker = "pesquisa02.lages.ifsc.edu.br";
int mqttPort = 1883;

//credenciais
char deviceId = "726e2360-db99-11e8-9c73-1905eba2b69b";
char deviceToken =  "v7CE1CuQMafGKe6Pj5oG";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);


long lastReconnectAttempt = 0;

//Declarando os métodos para ficarem visíveis ao método principal
void sendDhtData();
void sendPluvioData();
void sendAnemometerData();
void sendEndDirection();
void sendData();

// variável para controlar de quanto em quanto tempo os dados serão enviados ao servidor 
int last_send;

void setup() {

  last_send=0;
  // Set console baud rate
  Serial.begin(9600);
 

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");

  // MQTT Broker setup
  mqtt.setServer(broker, mqttPort);
  
  //mqtt.setCallback(mqttCallback);
}

boolean mqttConnect() {
  Serial.println("Connecting to ");
  Serial.println(broker);
  

  // Connect to MQTT Broker
  boolean status = mqtt.connect(deviceId, deviceToken, NULL);


  if (status == false) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" OK");
  
//  mqtt.subscribe(topicLed);
  return mqtt.connected();
}

void loop() {

  if (!mqtt.connected()) {
    Serial.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }

  if(millis() - last_send > 5000){
    last_send = millis();
    sendData();
  }
  
  mqtt.loop();
  
}

  void sendEndDirection(){
    data["endDirection"]=0;
  }

  void sendAnemometerData(){
    data["Anemometer"]=30.0;
  }

  void sendPluvioData(){
    data["pluviometer"]= 1.25;
  }

  void sendDhtData(){
  
  data["temperature"] = 27;
  data["humidity"] = 70;

  
}

void sendData(){

  sendDhtData();
  sendPluvioData();
  sendAnemometerData();
  sendEndDirection();

  data.printTo(payload, sizeof(payload));
  strPayload = String(payload);

  Serial.println(strPayload.c_str() );

  mqtt.publish( "v1/devices/me/telemetry", strPayload.c_str() );
}



//
//void mqttCallback(char payload) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("]: ");
//  Serial.write(payload, len);
//  Serial.println();
//
//  // Only proceed if incoming message's topic matches
//  if (String(topic) == topicLed) {
//    ledStatus = !ledStatus;
//    digitalWrite(LED_PIN, ledStatus);
//    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
//  }
//}
