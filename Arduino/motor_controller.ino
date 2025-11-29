// --------------------------------------------------------------------
// MOTOR DRIVER PINS
// --------------------------------------------------------------------
int EN = 6;         // PWM pin to control motor speed
int IN1 = 7;        // Motor direction pin 1
int IN2 = 8;        // Motor direction pin 2

// --------------------------------------------------------------------
// VARIABLES
// --------------------------------------------------------------------
int speed = 0;              // Final PWM value (0–255)
String msgraw = "";         // Raw message received from ESP8266
String dir_past = "fwd";    // Store last direction to prevent sudden reversal


void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);

  // Default motor direction = forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  Serial.begin(115200);     // USB Serial (for debugging)
  Serial1.begin(9600);      // UART connection to ESP8266

  delay(1000);

  // ------------------------------------------------------------------
  // ESP8266 TCP SERVER SETUP (AT commands)
  // ------------------------------------------------------------------

  Serial1.println("AT+CIPMUX=1");     // Allow multiple clients
  delay(500);

  Serial1.println("AT+CIPSERVER=1,5050");  // Start TCP server on port 5050
  delay(500);

  Serial1.println("AT+CIPSTO=0");     // Disable auto-timeout (keeps link alive)
  delay(500);
}


void loop() {

  // ------------------------------------------------------------------
  // READ INCOMING TCP PACKET FROM ESP8266
  // ------------------------------------------------------------------
  if (Serial1.available()) {
    msgraw = Serial1.readString(); // Read raw input
    msgraw.trim();
  }

  // Valid TCP packets start with +IPD
  // Example: +IPD,0,7:40,fwd0,CLOSED
  if (!msgraw.startsWith("+IPD")) return;

  // Extract payload between ":" and "0,CLOSED"
  int pos = msgraw.indexOf(':');
  int poscls = msgraw.indexOf("0,CLOSED");

  if (poscls == -1)
    poscls = msgraw.length();

  String msg = msgraw.substring(pos + 1, poscls);

  // msg format → "speed,direction"  e.g. "40,fwd"
  int msg1 = msg.indexOf(',');
  String speed_msg = msg.substring(0, msg1);
  String dir = msg.substring(msg1 + 1);
  dir.trim();

  // Convert speed % (0–100) → PWM (0–255)
  int speedraw = speed_msg.toInt();
  speed = (speedraw * 255) / 100;

  Serial.println(dir);

  // ------------------------------------------------------------------
  // SAFETY: DIRECTION CONTROL WITH COOL-DOWN DELAY
  // ------------------------------------------------------------------
  // Prevents instant direction reversal (dangerous for motors & drivers).
  // If direction changes, motor is stopped for 1 second before switching.

  if (dir == "fwd") {

    if (dir_past == "fwd") {
      // Already moving forward → normal operation
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    } else {
      // SAFE CHANGE → stop motor first
      analogWrite(EN, 0);
      delay(1000);
      dir_past = dir;

      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    }
  }


  if (dir == "rev") {

    if (dir_past == "rev") {
      // Already moving reverse
      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
    } else {
      // SAFE CHANGE FROM FWD → REV
      analogWrite(EN, 0);
      delay(1000);
      dir_past = dir;

      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
    }
  }

  // ------------------------------------------------------------------
  // APPLY PWM SPEED
  // ------------------------------------------------------------------
  analogWrite(EN, speed);

  delay(10);  // Small stability delay
}
