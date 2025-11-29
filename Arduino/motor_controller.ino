int EN = 6;         // PWM pin to control motor speed
int IN1 = 7;        // Motor direction pin 1
int IN2 = 8;        // Motor direction pin 2
int speed = 0;      // Final PWM value (0–255)
String msgraw = ""; // Raw message from ESP8266
String dir_past = "fwd"; // Stores last direction to avoid sudden direction reversal

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);

  // Set initial motor direction (forward)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  Serial.begin(115200);   // For debugging
  Serial1.begin(9600);    // ESP8266 UART

  delay(1000);

  // ------------------------------
  // ESP8266 TCP SERVER SETUP
  // ------------------------------

  // Enable multiple connections
  Serial1.println("AT+CIPMUX=1");
  delay(500);

  // Start TCP server on port 5050
  Serial1.println("AT+CIPSERVER=1,5050");
  delay(500);

  // Disable server auto-timeout to keep connection stable
  Serial1.println("AT+CIPSTO=0");
  delay(500);
}

void loop() {

  // ------------------------------
  // READ DATA FROM ESP8266
  // ------------------------------

  if (Serial1.available()) {
    msgraw = Serial1.readString(); // Read full line from ESP
    msgraw.trim();
  }

  // Incoming TCP payload from ESP8266 begins with +IPD
  // Example: +IPD,0,7:40,fwd0,CLOSED
  // Ignore anything not starting with +IPD
  if (!msgraw.startsWith("+IPD")) return;

  // Extract the portion after ":" → actual message, e.g. "40,fwd"
  int pos = msgraw.indexOf(':');

  // Find "0,CLOSED" if present (sent when client disconnects)
  int poscls = msgraw.indexOf("0,CLOSED");
  if (poscls == -1) poscls = msgraw.length();

  // Clean message between ":" and "0,CLOSED"
  String msg = msgraw.substring(pos + 1, poscls);

  // Split into speed and direction
  int msg1 = msg.indexOf(',');
  String speed_msg = msg.substring(0, msg1);
  String dir = msg.substring(msg1 + 1);
  dir.trim();

  // Convert speed (0–100%) to PWM (0–255)
  int speedraw = speed_msg.toInt();
  speed = (speedraw * 255) / 100;

  Serial.println(dir);

  // ------------------------------
  // SAFETY: MOTOR DIRECTION LOGIC
  // ------------------------------
  // If the motor direction changes suddenly, motors and drivers can get
  // damaged due to back-EMF and mechanical stress.
  // Logic below forces motor to STOP for 1 second before switching direction.
  // dir_past stores previous direction to detect changes.

  if (dir == "fwd") {

    if (dir_past == "fwd") {
      // Continue normally in same direction
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    } else {
      // SAFE REVERSAL: stop motor, wait, THEN change direction
      analogWrite(EN, 0);
      delay(1000); // 1-second cooldown
      dir_past = dir;

      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    }
  }

  if (dir == "rev") {

    if (dir_past == "rev") {
      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
    } else {
      // SAFE SWITCH FROM FORWARD TO REVERSE
      analogWrite(EN, 0);
      delay(1000);
      dir_past = dir;

      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
    }
  }

  // Apply PWM speed
  analogWrite(EN, speed);

  delay(10);
}
