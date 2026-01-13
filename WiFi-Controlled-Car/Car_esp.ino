#include <AFMotor.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

void setup() {
  Serial.begin(9600);

  motor1.setSpeed(200); 
  motor2.setSpeed(200);
  motor3.setSpeed(200);
  motor4.setSpeed(200);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '\n' || cmd == '\r') return;

    Serial.print("Received: ");
    Serial.println(cmd);

    if (cmd == 'F') { 
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }
    else if (cmd == 'B') { 
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
    }
    else if (cmd == 'L') { 
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }
    else if (cmd == 'R') { 
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
    }
    else if (cmd == 'S') { 
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      motor3.run(RELEASE);
      motor4.run(RELEASE);
    }
  }
}
