/**********************************************************************************
  Smart Home Automation using ESP32 with Blynk 2.0 and PIR Sensors

  Author : ANURAG KAR
  Also Credits to : Soumyajit Bhowal,Subhajit Khatua
**********************************************************************************/
#define BLYNK_TEMPLATE_ID "TMPL3wC674Utu"
#define BLYNK_TEMPLATE_NAME "ESP32 Smart Home"
#define BLYNK_AUTH_TOKEN "vnOBInfsfOS1znhQjPnjTeyaF5y-ZGG0"

char ssid[] = "Anurag";
char pass[] = "12345678";
int personCount = 0; // Keeps track of number of persons in the room

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>

Preferences pref;

#define RelayPin1 23
#define RelayPin2 22
#define wifiLed 2
#define pir1 32 // Inside PIR sensor
#define pir2 33 // Outside PIR sensor

#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2
#define VPIN_BUTTON_ON V3
#define VPIN_BUTTON_OFF V5

bool toggleState_1 = LOW;
bool toggleState_2 = LOW;
int state = 0; // Tracks PIR state (0: OFF, 1: ON)
bool pir1_triggered = false;
bool pir2_triggered = false;
unsigned long pir1_time = 0;
unsigned long pir2_time = 0;
const unsigned long timeout = 3000; // 3 seconds for PIR sequence

int wifiFlag = 0;

char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, !toggleState_1);
  pref.putBool("Relay1", toggleState_1);
  state = toggleState_1 || toggleState_2; // Update PIR state based on relays
  Serial.println("BLYNK: Relay 1 set to " + String(toggleState_1));
}

BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_2 = param.asInt();
  digitalWrite(RelayPin2, !toggleState_2);
  pref.putBool("Relay2", toggleState_2);
  state = toggleState_1 || toggleState_2; // Update PIR state based on relays
  Serial.println("BLYNK: Relay 2 set to " + String(toggleState_2));
}

BLYNK_WRITE(VPIN_BUTTON_ON) {
  all_SwitchOn();
}

BLYNK_WRITE(VPIN_BUTTON_OFF) {
  all_SwitchOff();
}

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

BLYNK_CONNECTED() {
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  Blynk.syncVirtual(VPIN_BUTTON_1, VPIN_BUTTON_2);
}

void all_SwitchOn() {
  toggleState_1 = 1; digitalWrite(RelayPin1, LOW); pref.putBool("Relay1", toggleState_1); Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); delay(100);
  toggleState_2 = 1; digitalWrite(RelayPin2, LOW); pref.putBool("Relay2", toggleState_2); Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2); delay(100);
  state = 1; // Update PIR state
  Serial.println("All Switches ON");
}

void all_SwitchOff() {
  toggleState_1 = 0; digitalWrite(RelayPin1, HIGH); pref.putBool("Relay1", toggleState_1); Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); delay(100);
  toggleState_2 = 0; digitalWrite(RelayPin2, HIGH); pref.putBool("Relay2", toggleState_2); Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2); delay(100);
  state = 0; // Update PIR state
  Serial.println("All Switches OFF");
}

void getRelayState() {
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

  state = toggleState_1 || toggleState_2; // Initialize PIR state
}

void pirControl() {
  unsigned long now = millis();

  // PIR1 triggers first (inside)
  if (digitalRead(pir1) == HIGH && !pir1_triggered) {
    pir1_triggered = true;
    pir1_time = now;
    Serial.print("time1 : ");
    Serial.println(pir1_time);
    Serial.println("PIR1 triggered");
  }

  // PIR2 triggers first (outside)
  if (digitalRead(pir2) == HIGH && !pir2_triggered) {
    pir2_triggered = true;
    pir2_time = now;
    Serial.print("time2 : ");
    Serial.println(pir2_time);
    Serial.println("PIR2 triggered");
  }

  // Entry: PIR1 then PIR2 within timeout
if (pir1_triggered && pir2_triggered && pir2_time > pir1_time && pir2_time - pir1_time <= timeout) {
  personCount++;
  Serial.println("Entry Detected → Person Count: " + String(personCount));
  if (state == 0) {
    all_SwitchOn();
    Serial.println("Appliances ON");
  }
  resetTriggers();
  delay(1000); 
}

// Exit: PIR2 then PIR1 within timeout
if (pir1_triggered && pir2_triggered && pir1_time > pir2_time && pir1_time - pir2_time <= timeout) {
  if (personCount > 0) personCount--;
  Serial.println("Exit Detected → Person Count: " + String(personCount));
  if (personCount == 0 && state == 1) {
    all_SwitchOff();
    Serial.println("Appliances OFF");
  }
  resetTriggers();
  delay(1000); 
}


  // Timeout auto-reset
  if (pir1_triggered && now - pir1_time > timeout) pir1_triggered = false;
  if (pir2_triggered && now - pir2_time > timeout) pir2_triggered = false;

  // Relay status feedback
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 1000) {
    Serial.print("Relay Status: ");
    Serial.println(state ? "ON" : "OFF");
    lastPrint = now;
  }
}

void resetTriggers() {
  pir1_triggered = false;
  pir2_triggered = false;
  pir1_time = 0;
  pir2_time = 0;
}

void setup() {
  Serial.begin(115200);
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(pir1, INPUT);
  pinMode(pir2, INPUT);

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
  pirControl();
  delay(10);
}