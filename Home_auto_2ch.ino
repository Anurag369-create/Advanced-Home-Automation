/**********************************************************************************
  Smart Home Automation using ESP32 with Blynk 2.0 & EEPROM

**********************************************************************************/
#define BLYNK_TEMPLATE_ID "TMPL3wC674Utu"
#define BLYNK_TEMPLATE_NAME "ESP32 Smart Home"
#define BLYNK_AUTH_TOKEN "vnOBInfsfOS1znhQjPnjTeyaF5y-ZGG0"


char ssid[] = "IOTA";
char pass[] = "1234abc";

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>

Preferences pref;

#define RelayPin1 23
#define RelayPin2 22
#define wifiLed 2

#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2
#define VPIN_MAIN_SWITCH V3

bool toggleState_1 = LOW;
bool toggleState_2 = LOW;
bool mainSwitchState = LOW; // Tracks Main Switch state

int wifiFlag = 0;

char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt(); // this line updates the togglestate when command is given by phone or web server
  digitalWrite(RelayPin1, !toggleState_1);
  pref.putBool("Relay1", toggleState_1);
  Serial.println("BLYNK: Relay 1 set to " + String(toggleState_1));
}

BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_2 = param.asInt();
  digitalWrite(RelayPin2, !toggleState_2);
  pref.putBool("Relay2", toggleState_2);
  Serial.println("BLYNK: Relay 2 set to " + String(toggleState_2));
}

BLYNK_WRITE(VPIN_MAIN_SWITCH) {
  mainSwitchState = param.asInt();
  if (mainSwitchState) {
    all_SwitchOn();
  } else {
    all_SwitchOff();
  }
}
// This function checks whether Blynk is connected with ESP or Not
void checkBlynkStatus() {
  bool isconnected = Blynk.connected();
  if (!isconnected) {
    wifiFlag = 1;
    digitalWrite(wifiLed, LOW);
    Serial.println("Blynk Not Connected");
  } else {
    wifiFlag = 0;
    digitalWrite(wifiLed, HIGH);
    Serial.println("Blynk Connected");
  }
}
// Updates the Blynk app with the current states of toggleState_1, toggleState_2, and mainSwitchState using virtualWrite.
BLYNK_CONNECTED() {
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  Blynk.virtualWrite(VPIN_MAIN_SWITCH, mainSwitchState);
  Blynk.syncVirtual(VPIN_BUTTON_1, VPIN_BUTTON_2, VPIN_MAIN_SWITCH);
}

void all_SwitchOn() {
  toggleState_1 = 1; digitalWrite(RelayPin1, LOW); pref.putBool("Relay1", toggleState_1); Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); delay(100);
  toggleState_2 = 1; digitalWrite(RelayPin2, LOW); pref.putBool("Relay2", toggleState_2); Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2); delay(100);
  mainSwitchState = 1;
  Blynk.virtualWrite(VPIN_MAIN_SWITCH, mainSwitchState);
  Serial.println("All Switches ON");
}

void all_SwitchOff() {
  toggleState_1 = 0; digitalWrite(RelayPin1, HIGH); pref.putBool("Relay1", toggleState_1); Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); delay(100);
  toggleState_2 = 0; digitalWrite(RelayPin2, HIGH); pref.putBool("Relay2", toggleState_2); Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2); delay(100);
  mainSwitchState = 0;
  Blynk.virtualWrite(VPIN_MAIN_SWITCH, mainSwitchState);
  Serial.println("All Switches OFF");
}

void getRelayState() {
  // Retrieves the state of Relay1 from NVS(EEPROM) when new powercycle started
  toggleState_1 = pref.getBool("Relay1", 0);
  Serial.println("Relay1 state from NVS: " + String(toggleState_1));
  digitalWrite(RelayPin1, !toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(200);

  toggleState_2 = pref.getBool("Relay2", 0);
  Serial.println("Relay2 state from NVS: " + String(toggleState_2));
  digitalWrite(RelayPin2, !toggleState_2);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  delay(200);

  mainSwitchState = pref.getBool("MainSwitch", 0);
  Blynk.virtualWrite(VPIN_MAIN_SWITCH, mainSwitchState);
}

void setup() {
  Serial.begin(115200);
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  digitalWrite(RelayPin1, !toggleState_1);
  digitalWrite(RelayPin2, !toggleState_2);
  digitalWrite(wifiLed, LOW);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    digitalWrite(wifiLed, HIGH);
  } else {
    Serial.println("\nWiFi Connection Failed");
  }

  timer.setInterval(2000L, checkBlynkStatus);
  Blynk.config(auth);
  delay(1000);

  getRelayState();
  Serial.println("System Initialized");
}

void loop() {
  Blynk.run();
  timer.run();
  delay(10);
}
