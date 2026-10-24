#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(12, 13); // RX = pin 12 (from ESP32 GPIO 17), TX = pin 13
Servo servo;

int servoPin = 8;
int initAngle = 75;
int steerRange = 30; // max deviation from center, in degrees
int Rmax = initAngle + steerRange; // Max angle for right turn
int Lmin = initAngle - steerRange; // Min angle for left turn

int motorL[2] = {3, 11};
int motorR[2] = {5, 6};

// Watchdog: the web page repeats the current command every 150 ms while the car
// moves. If nothing arrives for this long (link lost, tab closed, bridge reset),
// the car stops by itself.
const unsigned long WATCHDOG_MS = 500;
// Below this duty cycle the motors stall, so analog speeds start here.
const int MIN_PWM = 70;

unsigned long lastCmdMs = 0;
bool moving = false;

char lineBuf[16];
byte lineLen = 0;
bool inLine = false;

// speed: -255 (full reverse) .. 255 (full forward), applied to both motors
void setMotors(int speed) {
  speed = constrain(speed, -255, 255);
  int duty = abs(speed);
  if (speed >= 0) {
    analogWrite(motorL[0], duty); analogWrite(motorL[1], 0);
    analogWrite(motorR[0], duty); analogWrite(motorR[1], 0);
  } else {
    analogWrite(motorL[0], 0); analogWrite(motorL[1], duty);
    analogWrite(motorR[0], 0); analogWrite(motorR[1], duty);
  }
  moving = (duty != 0);
}

// steer: -100 (full left) .. 100 (full right)
void setSteering(int steer) {
  steer = constrain(steer, -100, 100);
  servo.write(initAngle + (long)steer * steerRange / 100);
}

void stopCar() {
  setMotors(0);
  setSteering(0);
}

// Analog command from the joystick: "J<speed>,<steer>" with speed -100..100
// and steer -100..100 (percent).
void handleAnalog(const char* s) {
  int speedPct = 0, steerPct = 0;
  if (sscanf(s, "%d,%d", &speedPct, &steerPct) != 2) return;
  speedPct = constrain(speedPct, -100, 100);
  int duty = 0;
  if (speedPct != 0) duty = MIN_PWM + (long)(255 - MIN_PWM) * abs(speedPct) / 100;
  setMotors(speedPct < 0 ? -duty : duty);
  setSteering(steerPct);
}

void handleChar(char bt) {
  if (bt == 'f')      { setMotors(255);  setSteering(0); }
  else if (bt == 'b') { setMotors(-255); setSteering(0); }
  else if (bt == 'l') { setMotors(255);  setSteering(-100); }
  else if (bt == 'r') { setMotors(255);  setSteering(100); }
  else if (bt == 's') { stopCar(); }
  else return; // unknown character: ignore, do not feed the watchdog
  lastCmdMs = millis();
}

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  for(int i=0; i<2; i++){
    pinMode(motorR[i], OUTPUT);
    pinMode(motorL[i], OUTPUT);
  }
  servo.attach(servoPin);
  stopCar();
}

void loop() {
  while (BTSerial.available()) {
    char c = BTSerial.read();
    if (inLine) {
      if (c == '\n' || c == ';') {
        lineBuf[lineLen] = '\0';
        handleAnalog(lineBuf);
        lastCmdMs = millis();
        inLine = false;
      } else if (lineLen < sizeof(lineBuf) - 1) {
        lineBuf[lineLen++] = c;
      } else {
        inLine = false; // too long: drop it
      }
    } else if (c == 'J') {
      inLine = true;
      lineLen = 0;
    } else {
      Serial.println(c); // debugging on the Arduino's serial monitor
      handleChar(c);
    }
  }

  if (moving && millis() - lastCmdMs > WATCHDOG_MS) {
    stopCar();
    Serial.println("Watchdog: no command, stopped");
  }
}
