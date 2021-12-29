#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SoftwareSerial.h>


SoftwareSerial NodeMCU(D7,D8);

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "WLAN_SSID"
#define WLAN_PASS       "WLAN_PASSWORD"



/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "ADAFRUIT_IO_USERNAME"
#define AIO_KEY         "ADAFRUIT_IO_PASSWORD"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Power = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Potencia");
Adafruit_MQTT_Publish Voltage = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tensão");
Adafruit_MQTT_Publish Current = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/corrente");
Adafruit_MQTT_Publish paramDIC = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dic");
Adafruit_MQTT_Publish paramFIC = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/fic");
void MQTT_connect();

void setup() {
  Serial.begin(9600);
  NodeMCU.begin(115200);
  pinMode(D7,INPUT);
  pinMode(D8,OUTPUT);
  delay(10);
  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  String stringData = "";
  char character;

  while (NodeMCU.available()) {
       character = NodeMCU.read();
       stringData.concat(character);
  }


  int commaIndex = stringData.indexOf(',');
  int secondCommaIndex = stringData.indexOf(',', commaIndex + 1);
  int thirdCommaIndex = stringData.indexOf(',', secondCommaIndex + 1);
  int fourthCommaIndex = stringData.indexOf(',', thirdCommaIndex + 1);

  String W = stringData.substring(0, commaIndex);
  String V = stringData.substring(commaIndex + 1, secondCommaIndex);
  String I = stringData.substring(secondCommaIndex + 1, thirdCommaIndex);
  String DIC = stringData.substring(thirdCommaIndex + 1, fourthCommaIndex);
  String FIC = stringData.substring(fourthCommaIndex + 1);


  float watt = W.toFloat();
  float volt = V.toFloat();
  float amp = I.toFloat();
  float pamDIC = DIC.toFloat();
  float pamFIC = FIC.toFloat();

    
  // Publica o valor da potência no feed da AdaFruit
  Serial.print(F("\nEnviando os valores de Potência "));
  Serial.println(watt);
  Serial.print("...");

  if (! Power.publish(watt)) {
    Serial.println(F("Erro"));
  } else {
    Serial.println(F("OK!"));
  }


  // Publica o valor da tensão no feed da AdaFruit
  Serial.print(F("\nEnviando os valores de Tensão "));
  Serial.println(volt);
  Serial.print("...");

  if (! Voltage.publish(volt)) {
    Serial.println(F("Erro"));
  } else {
    Serial.println(F("OK!"));
  }


  // Publica o valor da corrente no feed da AdaFruit
  Serial.print(F("\nEnviando os valores de Corrente "));
  Serial.println(amp);
  Serial.print("...");

  if (! Current.publish(amp)) {
    Serial.println(F("Erro"));
  } else {
    Serial.println(F("OK!"));
  }


    // Publica o valor do DIC no feed da AdaFruit
  Serial.print(F("\nEnviando os valores de DIC "));
  Serial.println(pamDIC);
  Serial.print("...");

  if (! paramDIC.publish(pamDIC)) {
    Serial.println(F("Erro"));
  } else {
    Serial.println(F("OK!"));
  }


      // Publica o valor do FIC no feed da AdaFruit
  Serial.print(F("\nEnviando os valores de FIC "));
  Serial.println(pamFIC);
  Serial.print("...");

  if (! paramFIC.publish(pamFIC)) {
    Serial.println(F("Erro"));
  } else {
    Serial.println(F("OK!"));
  }
  
delay(11000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
