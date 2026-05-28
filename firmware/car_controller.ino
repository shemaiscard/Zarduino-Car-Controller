#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(12, 13); //블루투스 설정 BTSerial(Tx, Rx)
Servo servo;

int servoPin = 8;
int initAngle = 75;
// Reduced the angle deviation from 51 to 30 for less aggressive turns
int Rmax = initAngle + 30; // Max angle for right turn
int Lmin = initAngle - 30; // Min angle for left turn

int motorL[2] = {3, 11};
int motorR[2] = {5, 6};

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600); //블루투스 통신 시작
  for(int i=0; i<2; i++){
    pinMode(motorR[i], OUTPUT);
    pinMode(motorL[i], OUTPUT);
  }
  servo.attach(servoPin);
  servo.write(initAngle);
}

void loop() {
  delay(10); 
  if(BTSerial.available()) {
    char bt;
    bt = BTSerial.read();
    Serial.println(bt); // Print received character to Arduino's Serial Monitor for debugging
    if(bt == 'f') {
      analogWrite(motorL[0], 255);
      analogWrite(motorL[1], 0);
      analogWrite(motorR[0], 255);
      analogWrite(motorR[1], 0);
      servo.write(initAngle); // Ensure servo is centered when moving straight
    }
    else if(bt == 'b') {
      analogWrite(motorL[0], 0);
      analogWrite(motorL[1], 255);
      analogWrite(motorR[0], 0);
      analogWrite(motorR[1], 255);
      servo.write(initAngle); // Ensure servo is centered when moving straight
    }
    else if(bt == 'l') {
      servo.write(Lmin);
      analogWrite(motorL[0], 255);
      analogWrite(motorL[1], 0);
      analogWrite(motorR[0], 255);
      analogWrite(motorR[1], 0);
    }
    else if(bt == 'r') {
      servo.write(Rmax);
      analogWrite(motorL[0], 255);
      analogWrite(motorL[1], 0);
      analogWrite(motorR[0], 255);
      analogWrite(motorR[1], 0);
    }
    else if(bt == 's') { // Stop command
      analogWrite(motorL[0], 0);
      analogWrite(motorL[1], 0);
      analogWrite(motorR[0], 0);
      analogWrite(motorR[1], 0);
      servo.write(initAngle); // Center servo when stopped
    }
  }
}
