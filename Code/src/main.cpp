/*
Smart meter optical readout to MQTT over WLAN

Author:
Dominik Künne

Used libraries:
https://github.com/olliiiver/sml_parser
https://github.com/plerup/espsoftwareserial/
https://github.com/adafruit/Adafruit_MQTT_Library
*/


// Includes
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <sml.h>
#include "WiFi.h"
#include "Adafruit_MQTT_Client.h"

// ########## WiFi START ##########
WiFiClient wifiClient;
#define WLAN_SSID "xxxxx"
#define WLAN_PASS "xxxxx"
// ########## WiFi END ##########

// ########## MQTT START ##########
#define MQTT_SERVER      "XXX.XXX.XXX.XXX"
#define MQTT_SERVERPORT  1883

Adafruit_MQTT_Client mqtt(&wifiClient, MQTT_SERVER, MQTT_SERVERPORT);
Adafruit_MQTT_Publish mqttPubSensors = Adafruit_MQTT_Publish(&mqtt, "sensors/stromzaehler/<LOCATION>/<SERIAL>");
Adafruit_MQTT_Publish mqttPubDebug = Adafruit_MQTT_Publish(&mqtt, "sensors/debug");
// ########## MQTT END ##########


HardwareSerial irLesekopf(0);

sml_states_t currentState;
unsigned char currentChar = 0;
char buffer[50];
char floatBuffer[20];
double ValBezugTotal = -2, ValVerbrauchMomentan = -2, ValGeliefertTotal = -2, ValSpannungL1 = -2, ValSpannungL2 = -2, ValSpannungL3 = -2, ValStromL1 = -2, ValStromL2 = -2, ValStromL3 = -2, ValPhasenwinkel1 = -2, ValPhasenwinkel2 = -2, ValPhasenwinkel3 = -2, ValPhasenwinkel4 = -2, ValPhasenwinkel5 = -2, ValFrequenz = -2;

typedef struct {
  const unsigned char OBIS[6];
  void (*Handler)();
} OBISHandler;


void MsgBezugTotal() {smlOBISWh(ValBezugTotal);}
void MsgVerbrauchMomentan() {smlOBISW(ValVerbrauchMomentan);}
void MsgGeliefertTotal() {smlOBISWh(ValGeliefertTotal);}
void MsgSpannungL1() {smlOBISVolt(ValSpannungL1);}
void MsgSpannungL2() {smlOBISVolt(ValSpannungL2);}
void MsgSpannungL3() {smlOBISVolt(ValSpannungL3);}
void MsgStromL1() {smlOBISAmpere(ValStromL1);}
void MsgStromL2() {smlOBISAmpere(ValStromL2);}
void MsgStromL3() {smlOBISAmpere(ValStromL3);}
void MsgPhasenwinkel1() {smlOBISDegree(ValPhasenwinkel1);}
void MsgPhasenwinkel2() {smlOBISDegree(ValPhasenwinkel2);}
void MsgPhasenwinkel3() {smlOBISDegree(ValPhasenwinkel3);}
void MsgPhasenwinkel4() {smlOBISDegree(ValPhasenwinkel4);}
void MsgPhasenwinkel5() {smlOBISDegree(ValPhasenwinkel5);}
void MsgFrequenz() {smlOBISHertz(ValFrequenz);}

// clang-format off
OBISHandler OBISHandlers[] = {
    {{0x01, 0x00, 0x01, 0x08, 0x00, 0xff}, &MsgBezugTotal},     /* OBIS Bezug Total 1.8.0 */
    {{0x01, 0x00, 0x10, 0x07, 0x00, 0xFF}, &MsgVerbrauchMomentan},      /* OBIS Momentanverbrauch 16.7.0 */
    {{0x01, 0x00, 0x02, 0x08, 0x00, 0xFF}, &MsgGeliefertTotal},      /* OBIS Geliefert Total 2.8.0 */
    {{0x01, 0x00, 0x20, 0x07, 0x00, 0xFF}, &MsgSpannungL1},      /* Spannung L1 */
    {{0x01, 0x00, 0x34, 0x07, 0x00, 0xFF}, &MsgSpannungL2},      /* Spannung L2 */
    {{0x01, 0x00, 0x48, 0x07, 0x00, 0xFF}, &MsgSpannungL3},      /* Spannung L3 */
    {{0x01, 0x00, 0x1F, 0x07, 0x00, 0xFF}, &MsgStromL1},      /* Strom L1 */
    {{0x01, 0x00, 0x33, 0x07, 0x00, 0xFF}, &MsgStromL2},      /* Strom L2 */
    {{0x01, 0x00, 0x47, 0x07, 0x00, 0xFF}, &MsgStromL3},      /* Strom L3 */
    {{0x01, 0x00, 0x51, 0x07, 0x01, 0xFF}, &MsgPhasenwinkel1},      /* Winkel 1 */
    {{0x01, 0x00, 0x51, 0x07, 0x02, 0xFF}, &MsgPhasenwinkel2},      /* Winkel 2 */
    {{0x01, 0x00, 0x51, 0x07, 0x04, 0xFF}, &MsgPhasenwinkel3},      /* Phasenwinkel L1 */
    {{0x01, 0x00, 0x51, 0x07, 0x0F, 0xFF}, &MsgPhasenwinkel4},      /* Phasenwinkel L2 */
    {{0x01, 0x00, 0x51, 0x07, 0x1A, 0xFF}, &MsgPhasenwinkel5},      /* Phasenwinkel L3 */
    {{0x01, 0x00, 0x0E, 0x07, 0x00, 0xFF}, &MsgFrequenz},      /* Frequenz */
    {{0}, 0}
};
// clang-format on

bool continueSearch = true;



void sendData()
{

  dtostrf(ValBezugTotal, 10, 1, floatBuffer);
  sprintf(buffer, "ValBezugTotal: %s", floatBuffer);
  String jsonValBezugTotal = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Bezug_Total_1.8.0\",\"value\":" + String(floatBuffer) + "}";
  jsonValBezugTotal.replace(" ","");
  char bufValBezugTotal[jsonValBezugTotal.length()+1];
  jsonValBezugTotal.toCharArray(bufValBezugTotal, jsonValBezugTotal.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValVerbrauchMomentan, 6, 0, floatBuffer);
  sprintf(buffer, "ValVerbrauchMomentan: %s", floatBuffer);
  String jsonValVerbrauchMomentan = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Momentanverbrauch_16.7.0\",\"value\":" + String(floatBuffer) + "}";
  jsonValVerbrauchMomentan.replace(" ","");
  char bufValVerbrauchMomentan[jsonValVerbrauchMomentan.length()+1];
  jsonValVerbrauchMomentan.toCharArray(bufValVerbrauchMomentan, jsonValVerbrauchMomentan.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValGeliefertTotal, 10, 1, floatBuffer);
  sprintf(buffer, "ValGeliefertTotal: %s", floatBuffer);
  String jsonValGeliefertTotal = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Geliefert_Total_2.8.0\",\"value\":" + String(floatBuffer) + "}";
  jsonValGeliefertTotal.replace(" ","");
  char bufValGeliefertTotal[jsonValGeliefertTotal.length()+1];
  jsonValGeliefertTotal.toCharArray(bufValGeliefertTotal, jsonValGeliefertTotal.length()+1);
  Serial.println(buffer);


  dtostrf(ValSpannungL1, 4, 1, floatBuffer);
  sprintf(buffer, "ValSpannungL1: %s", floatBuffer);
  String jsonValSpannungL1 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Spannung_L1\",\"value\":" + String(floatBuffer) + "}";
  jsonValSpannungL1.replace(" ","");
  char bufValSpannungL1[jsonValSpannungL1.length()+1];
  jsonValSpannungL1.toCharArray(bufValSpannungL1, jsonValSpannungL1.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValSpannungL2, 4, 1, floatBuffer);
  sprintf(buffer, "ValSpannungL2: %s", floatBuffer);
  String jsonValSpannungL2 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Spannung_L2\",\"value\":" + String(floatBuffer) + "}";
  jsonValSpannungL2.replace(" ","");
  char bufValSpannungL2[jsonValSpannungL2.length()+1];
  jsonValSpannungL2.toCharArray(bufValSpannungL2, jsonValSpannungL2.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValSpannungL3, 4, 1, floatBuffer);
  sprintf(buffer, "ValSpannungL3: %s", floatBuffer);
  String jsonValSpannungL3 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Spannung_L3\",\"value\":" + String(floatBuffer) + "}";
  jsonValSpannungL3.replace(" ","");
  char bufValSpannungL3[jsonValSpannungL3.length()+1];
  jsonValSpannungL3.toCharArray(bufValSpannungL3, jsonValSpannungL3.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValStromL1, 6, 2, floatBuffer);
  sprintf(buffer, "ValStromL1: %s", floatBuffer);
  String jsonValStromL1 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Strom_L1\",\"value\":" + String(floatBuffer) + "}";
  jsonValStromL1.replace(" ","");
  char bufValStromL1[jsonValStromL1.length()+1];
  jsonValStromL1.toCharArray(bufValStromL1, jsonValStromL1.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValStromL2, 6, 2, floatBuffer);
  sprintf(buffer, "ValStromL2: %s", floatBuffer);
  String jsonValStromL2 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Strom_L2\",\"value\":" + String(floatBuffer) + "}";
  jsonValStromL2.replace(" ","");
  char bufValStromL2[jsonValStromL2.length()+1];
  jsonValStromL2.toCharArray(bufValStromL2, jsonValStromL2.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValStromL3, 6, 2, floatBuffer);
  sprintf(buffer, "ValStromL3: %s", floatBuffer);
  String jsonValStromL3 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Strom_L3\",\"value\":" + String(floatBuffer) + "}";
  jsonValStromL3.replace(" ","");
  char bufValStromL3[jsonValStromL3.length()+1];
  jsonValStromL3.toCharArray(bufValStromL3, jsonValStromL3.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValPhasenwinkel1, 3, 0, floatBuffer);
  sprintf(buffer, "ValPhasenwinkel1: %s", floatBuffer);
  String jsonValPhasenwinkel1 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Phasenwinkel_1\",\"value\":" + String(floatBuffer) + "}";
  jsonValPhasenwinkel1.replace(" ","");
  char bufValPhasenwinkel1[jsonValPhasenwinkel1.length()+1];
  jsonValPhasenwinkel1.toCharArray(bufValPhasenwinkel1, jsonValPhasenwinkel1.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValPhasenwinkel2, 3, 0, floatBuffer);
  sprintf(buffer, "ValPhasenwinkel2: %s", floatBuffer);
  String jsonValPhasenwinkel2 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Phasenwinkel_2\",\"value\":" + String(floatBuffer) + "}";
  jsonValPhasenwinkel2.replace(" ","");
  char bufValPhasenwinkel2[jsonValPhasenwinkel2.length()+1];
  jsonValPhasenwinkel2.toCharArray(bufValPhasenwinkel2, jsonValPhasenwinkel2.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValPhasenwinkel3, 3, 0, floatBuffer);
  sprintf(buffer, "ValPhasenwinkel3: %s", floatBuffer);
  String jsonValPhasenwinkel3 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Phasenwinkel_3\",\"value\":" + String(floatBuffer) + "}";
  jsonValPhasenwinkel3.replace(" ","");
  char bufValPhasenwinkel3[jsonValPhasenwinkel3.length()+1];
  jsonValPhasenwinkel3.toCharArray(bufValPhasenwinkel3, jsonValPhasenwinkel3.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValPhasenwinkel4, 3, 0, floatBuffer);
  sprintf(buffer, "ValPhasenwinkel4: %s", floatBuffer);
  String jsonValPhasenwinkel4 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Phasenwinkel_4\",\"value\":" + String(floatBuffer) + "}";
  jsonValPhasenwinkel4.replace(" ","");
  char bufValPhasenwinkel4[jsonValPhasenwinkel4.length()+1];
  jsonValPhasenwinkel4.toCharArray(bufValPhasenwinkel4, jsonValPhasenwinkel4.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValPhasenwinkel5, 3, 0, floatBuffer);
  sprintf(buffer, "ValPhasenwinkel5: %s", floatBuffer);
  String jsonValPhasenwinkel5 = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Phasenwinkel_5\",\"value\":" + String(floatBuffer) + "}";
  jsonValPhasenwinkel5.replace(" ","");
  char bufValPhasenwinkel5[jsonValPhasenwinkel5.length()+1];
  jsonValPhasenwinkel5.toCharArray(bufValPhasenwinkel5, jsonValPhasenwinkel5.length()+1);
  Serial.println(buffer);

  
  dtostrf(ValFrequenz, 3, 1, floatBuffer);
  sprintf(buffer, "ValFrequenz: %s", floatBuffer);
  String jsonValFrequenz = "{\"source\":\"weather\",\"location\":\"Keller_2\",\"sensor\":\"Frequenz\",\"value\":" + String(floatBuffer) + "}";
  jsonValFrequenz.replace(" ","");
  char bufValFrequenz[jsonValFrequenz.length()+1];
  jsonValFrequenz.toCharArray(bufValFrequenz, jsonValFrequenz.length()+1);
  Serial.println(buffer);


 // Connect MQTT
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  int maxMQTTRetries = 0;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
      // If too many retries go to deep sleep for 10 seconds
      if(maxMQTTRetries == 10){
        WiFi.disconnect();
        ESP.deepSleep(10e6);
      }
      maxMQTTRetries++;
  }
  Serial.println("MQTT Connected!");

  // Publish via MQTT
  mqttPubSensors.publish(bufValBezugTotal);
  mqttPubSensors.publish(bufValVerbrauchMomentan);
  mqttPubSensors.publish(bufValGeliefertTotal);
  mqttPubSensors.publish(bufValSpannungL1);
  mqttPubSensors.publish(bufValSpannungL2);
  mqttPubSensors.publish(bufValSpannungL3);
  mqttPubSensors.publish(bufValStromL1);
  mqttPubSensors.publish(bufValStromL2);
  mqttPubSensors.publish(bufValStromL3);
  mqttPubSensors.publish(bufValPhasenwinkel1);
  mqttPubSensors.publish(bufValPhasenwinkel2);
  mqttPubSensors.publish(bufValPhasenwinkel3);
  mqttPubSensors.publish(bufValPhasenwinkel4);
  mqttPubSensors.publish(bufValPhasenwinkel5);
  mqttPubSensors.publish(bufValFrequenz);
  delay(1000); // Troubleshooting delay

  // Disconnect MQTT
  mqtt.disconnect();
  // ########## END - Publish MQTT ##########

  
}



void readByte()
{
  unsigned int iHandler = 0;
  currentState = smlState(currentChar);
  if (currentState == SML_START) {
    /* reset local vars */
    ValBezugTotal = -3;
    ValVerbrauchMomentan = -3;
    ValGeliefertTotal = -3;
    ValSpannungL1 = -3;
    ValSpannungL2 = -3;
    ValSpannungL3 = -3;
    ValStromL1 = -3;
    ValStromL2 = -3;
    ValStromL3 = -3;
    ValPhasenwinkel1 = -3;
    ValPhasenwinkel2 = -3;
    ValPhasenwinkel3 = -3;
    ValPhasenwinkel4 = -3;
    ValPhasenwinkel5 = -3;
    ValFrequenz = -3;
  }
  if (currentState == SML_LISTEND) {
    /* check handlers on last received list */
    for (iHandler = 0; OBISHandlers[iHandler].Handler != 0 &&
                       !(smlOBISCheck(OBISHandlers[iHandler].OBIS));
         iHandler++)
      ;
    if (OBISHandlers[iHandler].Handler != 0) {
      OBISHandlers[iHandler].Handler();
    }
  }
  if (currentState == SML_UNEXPECTED) {
    //Serial.print(F(">>> Unexpected byte!\n"));
  }
  if (currentState == SML_FINAL) {
    sendData();
    Serial.print(F(">>> Successfully received a complete message!\n"));
  }
}



void setup()
{

  delay(5000);
  // ########## START - UART init ##########
  Serial.begin(115200);

  // Configure irLesekopf on pins RX=20 and TX=21 (-1, -1 means use the default)
  // This port is also used for ESP32 debug msg @ 115200 so it looks like it sends garbage at every boot.
  // We don't want to send anyways.
  irLesekopf.begin(9600, SERIAL_8N1, 20, 21);
  // ########## END - UART init ##########


  // ########## START - WiFi init ##########
  int maxWifiRetries = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // If too many retries go to deep sleep for 10 seconds
    if(maxWifiRetries == 10){
      WiFi.disconnect();
      delay(500);
      //ESP.deepSleep(10e6);
    }
    maxWifiRetries++;
  }

  ///* WiFi Debug
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //*/
  // ########## END - WiFi init ##########

}



void getData()
{

  delay(500);
  while (irLesekopf.available() > 0 && continueSearch == true)
  {

    currentChar = irLesekopf.read();
    readByte();

    continueSearch = false;

  }
}



void loop()
{

  //getData();

  ///* --- DEBUG ONLY ---
  while (irLesekopf.available() > 0 && continueSearch == true)
  {
    currentChar = irLesekopf.read();
    readByte();
  }
  //*/

  // ########## START - Deep Sleep ##########
  //Serial.println("Deep sleep for 60 seconds");
  //ESP.deepSleep(50e6); //10s + 50s = 60s
  // ########## END - Deep Sleep ##########
  //*/
}
