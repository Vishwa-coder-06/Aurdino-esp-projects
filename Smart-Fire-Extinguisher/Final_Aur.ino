#include <Servo.h>

#define FLAME1 2
#define FLAME2 5
#define FLAME3 6

#define TRIG1 A0
#define ECHO1 A1
#define TRIG2 A2
#define ECHO2 A3
#define TRIG3 A4
#define ECHO3 A5

#define PUMP1 12
#define PUMP2 13

Servo servoX;
Servo servoY;

void setup() {
  Serial.begin(9600);

  pinMode(FLAME1, INPUT);
  pinMode(FLAME2, INPUT);
  pinMode(FLAME3, INPUT);

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);

  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);

  servoX.attach(9);
  servoY.attach(10);

  servoX.write(0);
  servoY.write(0);
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH,30000);
  long distance = duration * 0.034 / 2;
  return distance;
}

void sprayUntilOff(int flamePin, int x, int y) {
  
  servoX.write(x);
  servoY.write(y);
  delay(500);

  while (digitalRead(flamePin) == LOW) {
    digitalWrite(PUMP1, HIGH);
    digitalWrite(PUMP2, LOW);
    delay(200); 
    Serial.println("spraying water");
  }

  digitalWrite(PUMP1, LOW);
  digitalWrite(PUMP2, LOW);
   
  servoX.write(0);
  servoY.write(0);
}


void loop() {
  int flame1 = digitalRead(FLAME1);
  int flame2 = digitalRead(FLAME2);
  int flame3 = digitalRead(FLAME3);

  if (flame1 == LOW ) {
    Serial.println("Fire:Row 1");
      long dist1 = getDistance(TRIG1, ECHO1);

    if (dist1 > 19 && dist1 < 28)      { sprayUntilOff(FLAME1, 1, 57); }
    else if (dist1 > 13 && dist1 < 19)  { sprayUntilOff(FLAME1, 20, 58); }
    else if(dist1 > 3 && dist1 < 13)    { sprayUntilOff(FLAME1, 30, 67); }
  }

  else if (flame2 == LOW ) {
    Serial.println("Fire:Row 2");
      long dist2 = getDistance(TRIG2, ECHO2);
        
    if (dist2 > 19 && dist2 < 27)      { sprayUntilOff(FLAME2, 3, 45); }
    else if (dist2 > 13 && dist2 < 19)  { sprayUntilOff(FLAME2, 28, 50); }
    else if(dist2 > 3 && dist2 < 13)    { sprayUntilOff(FLAME2, 37, 55); }
  }

  else if (flame3 == LOW ) {
    Serial.println("Fire:Row 3");
      long dist3 = getDistance(TRIG3, ECHO3);
            
    if (dist3 > 19 && dist3 < 24)      { sprayUntilOff(FLAME3, 10, 30); }
    else if (dist3 > 14 && dist3 < 19)  { sprayUntilOff(FLAME3, 40, 35); }
    else if (dist3 > 3 && dist3 < 14)   { sprayUntilOff(FLAME3, 65, 55); }
  }

  else {
    Serial.println("No flame"); 
  }

  delay(500);
}