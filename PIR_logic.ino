// Using ESP32
// Controlling an LED at pin 5 based on PIR sensors
#define pir1 32 // Inside PIR sensor
#define pir2 33 // Outside PIR sensor
#define ledPin 5 // LED pin 

int state = 0; // Tracks LED state (0 = OFF, 1 = ON)
bool pir1_triggered = false;
bool pir2_triggered = false;
unsigned long pir1_time = 0;
unsigned long pir2_time = 0;
const unsigned long timeout = 3000; // 3 seconds timeout for PIR sequence

void setup() {
  pinMode(pir1, INPUT);
  pinMode(pir2, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Initialize LED as OFF
  Serial.begin(115200);
  Serial.println("System Initialized");
}

void handlePIRLogic() {
  unsigned long now = millis();

  // PIR1 senses first
  if (digitalRead(pir1) == HIGH && !pir1_triggered) {
    pir1_triggered = true;
    pir1_time = now;
    Serial.print("time1 : ");
    Serial.println(pir1_time);
    Serial.println("PIR1 triggered");
  }

  // PIR2 senses first
  if (digitalRead(pir2) == HIGH && !pir2_triggered) {
    pir2_triggered = true;
    pir2_time = now;
    Serial.print("time2 : ");
    Serial.println(pir2_time);
    Serial.println("PIR2 triggered");
  }

  // Entry: PIR1 then PIR2 within timeout
  if (pir1_triggered && pir2_triggered && pir2_time > pir1_time && pir2_time - pir1_time <= timeout) {
    if (state == 0) {
      state = 1;
      digitalWrite(ledPin, HIGH); // Turn LED ON
      Serial.println("Entry Detected → LED ON");
    }
    resetTriggers();
    delay(1000); // Prevent immediate retriggering
  }

  // Exit: PIR2 then PIR1 within timeout
  if (pir1_triggered && pir2_triggered && pir1_time > pir2_time && pir1_time - pir2_time <= timeout) {
    if (state == 1) {
      state = 0;
      digitalWrite(ledPin, LOW); // Turn LED OFF
      Serial.println("Exit Detected → LED OFF");
    }
    resetTriggers();
    delay(1000); // Prevent immediate retriggering
  }

  // Timeout auto-reset
  if (pir1_triggered && now - pir1_time > timeout) pir1_triggered = false;
  if (pir2_triggered && now - pir2_time > timeout) pir2_triggered = false;

  // LED status feedback
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 1000) {
    Serial.print("LED Status: ");
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

void loop() {
  handlePIRLogic(); 
}
