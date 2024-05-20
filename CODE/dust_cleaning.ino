#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3Ig0vOgqV"
#define BLYNK_TEMPLATE_NAME "AutomateClean1"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <NewPing.h>

// Blynk auth token
char auth[] = "NLNeSMceeDxDit9ZlhVwrJxqwQ3iU0tg";

// WiFi credentials
char ssid[] = "Sreena";
char pass[] = "crayon305";

// Motor control pins
const int motor1A = 4;
const int motor1B = 5;
const int motor2A = 18;
const int motor2B = 19;

// Ultrasonic sensor pins
const int lftrigPin1 = 12; // Adjusted to avoid conflicts with motors
const int lfechoPin1 = 13; // Adjusted to avoid conflicts with motors
const int rgtrigPin2 = 27;
const int rgechoPin2 = 14;
const int mtrigPin3 = 26;
const int mechoPin3 = 25;

// IR sensor pin
const int irPin = 2;

// Ultrasonic sensor object definitions
NewPing sonarLeft(lftrigPin1, lfechoPin1, 200);
NewPing sonarRight(rgtrigPin2, rgechoPin2, 200);
NewPing sonarMid(mtrigPin3, mechoPin3, 200);

bool automationMode = false;
bool robotOn = false;
bool manualOverride = false;
int manualDirection = 0; // 0: stop, 1: forward, 2: backward, 3: right, 4: left

// Function to control motor movement
void move(int motor1A_state, int motor1B_state, int motor2A_state, int motor2B_state) {
  digitalWrite(motor1A, motor1A_state);
  digitalWrite(motor1B, motor1B_state);
  digitalWrite(motor2A, motor2A_state);
  digitalWrite(motor2B, motor2B_state);
  Serial.print("Motor states: ");
  Serial.print(motor1A_state);
  Serial.print(" ");
  Serial.print(motor1B_state);
  Serial.print(" ");
  Serial.print(motor2A_state);
  Serial.print(" ");
  Serial.println(motor2B_state);
}

// Motor control states
#define MOTOR_STOP LOW, LOW
#define MOTOR_FORWARD HIGH, LOW
#define MOTOR_BACKWARD LOW, HIGH

// Move functions for convenience
void moveStop() {
  move(MOTOR_STOP, MOTOR_STOP);
}

void moveForward() {
  move(MOTOR_FORWARD, MOTOR_FORWARD);
}

void moveBackward() {
  move(MOTOR_BACKWARD, MOTOR_BACKWARD);
}

void turnRight() {
  move(MOTOR_FORWARD, MOTOR_BACKWARD); // Left motor forward, right motor backward
}

void turnLeft() {
  move(MOTOR_BACKWARD, MOTOR_FORWARD); // Left motor backward, right motor forward
}

// Blynk virtual pin handlers
BLYNK_WRITE(V0) {
  int state = param.asInt();
  Serial.println("V0 pressed");
  if (state == 1) {
    Serial.println("Entering automation mode");
    automationMode = true;
    manualOverride = false;
    robotOn = true;
  } else {
    Serial.println("Exiting automation mode");
    automationMode = false;
    robotOn = false;
    moveStop(); // Stop
  }
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    Serial.println("Manual forward");
    automationMode = false;
    manualOverride = true;
    manualDirection = 1; // Forward
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    Serial.println("Manual backward");
    automationMode = false;
    manualOverride = true;
    manualDirection = 2; // Backward
  }
}

BLYNK_WRITE(V3) {
  if (param.asInt() == 1) {
    Serial.println("Manual right");
    automationMode = false;
    manualOverride = true;
    manualDirection = 3; // Right
  }
}

BLYNK_WRITE(V4) {
  if (param.asInt() == 1) {
    Serial.println("Manual left");
    automationMode = false;
    manualOverride = true;
    manualDirection = 4; // Left
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(motor1A, OUTPUT);
  pinMode(motor1B, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(motor2B, OUTPUT);
  pinMode(irPin, INPUT);

  Serial.println("Setup completed");

  // Print initial sensor readings
  Serial.print("Initial distanceLeft: ");
  Serial.println(sonarLeft.ping_cm());
  Serial.print("Initial distanceRight: ");
  Serial.println(sonarRight.ping_cm());
  Serial.print("Initial distanceMid: ");
  Serial.println(sonarMid.ping_cm());
  Serial.print("Initial IR value: ");
  Serial.println(digitalRead(irPin));
}

void loop() {
  Blynk.run();

  if (automationMode && robotOn) {
    int distanceLeft = sonarLeft.ping_cm();
    int distanceRight = sonarRight.ping_cm();
    int distanceMid = sonarMid.ping_cm();
    int irValue = digitalRead(irPin);

    Serial.print("Distances - Left: ");
    Serial.print(distanceLeft);
    Serial.print(" cm, Right: ");
    Serial.print(distanceRight);
    Serial.print(" cm, Mid: ");
    Serial.print(distanceMid);
    Serial.print(" cm, IR: ");
    Serial.println(irValue);

    if (distanceMid < 20 || distanceLeft < 15 || distanceRight < 15 || irValue == LOW) {
      Serial.println("Obstacle detected, stopping");
      moveStop();
      delay(200);
      moveBackward(); // Stop if obstacle detected
    } else {
      Serial.println("No obstacles, moving forward");
      moveForward(); // Move forward
    }
  } else if (manualOverride) {
    switch (manualDirection) {
      case 0:
        Serial.println("Manual mode: Stop");
        moveStop(); // Stop
        break;
      case 1:
        Serial.println("Manual mode: Forward");
        moveForward(); // Move forward
        break;
      case 2:
        Serial.println("Manual mode: Backward");
        moveBackward(); // Move backward
        break;
      case 3:
        Serial.println("Manual mode: Right");
        turnRight(); // Turn right
        break;
      case 4:
        Serial.println("Manual mode: Left");
        turnLeft(); // Turn left
        break;
    }
  }
}
