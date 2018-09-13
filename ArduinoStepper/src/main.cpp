/*
1 amarillo
2 marron
3 gris
4 negro
*/

/*
 Stepper Motor Control - one revolution

 This program drives a unipolar or bipolar stepper motor.
 The motor is attached to digital pins 8 - 11 of the Arduino.

 The motor should revolve one revolution in one direction, then
 one revolution in the other direction.


 Created 11 Mar. 2007
 Modified 30 Nov. 2009
 by Tom Igoe

 */
#include <Arduino.h>
#include <AccelStepper.h>

const uint16_t STEPS = 3000;
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
AccelStepper stepper;

void setup() {
  // set the speed at 60 rpm:
  stepper.setSpeed(60);
  stepper.setMaxSpeed(1000);

  // initialize the serial port:
  Serial.begin(9600);
}

void loop() {
//   // step one revolution  in one direction:
//   Serial.println("clockwise");
//   stepper.step(STEPS);
// //   delay(1000);

//   // step one revolution in the other direction:
//   Serial.println("counterclockwise");
//   stepper.step(-STEPS);
// //   delay(1000);
    stepper.runSpeed();
}