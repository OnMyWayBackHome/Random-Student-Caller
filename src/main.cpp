//Program to randomly shuffle the names of students in a class and display a different student name
//ever time a button is pressed. Display is a 4 x 8x8 Dot matrix panel controlled by MAX7219
#include "Arduino.h"
#include <MD_Parola.h>  //Library of nice functions for the display
#include <MD_MAX72xx.h> //Driver for display
#include <SPI.h>

#define CS_PIN 3  //control pin for the display
#define CLASS_PIN 4 //pin for the pushbutton to change classes
#define NEXT_PIN 2 //Pin to get next student
#include "LowPower.h"
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  //Number of 8x8 LED matrices

const int maxClassSize = 12;  //Enter the number of students in the largest class
const int numberOfClasses = 3; //How many classes
const int numberOfStudents[] = {12, 10, 6}; //Enter the number of students in each class (I'm a lazy programmer)
const char *classNames[] = {"Geo A", "Geo D", "College Alg"};
const char *classRosters[numberOfClasses][maxClassSize] = {
                      {"Angie", "Jill", "Mairi", "Allie", "Jacey", "Jennifer", "Jojo", "Nozomi",
                        "Lillian", "Erica", "Mehar", "Ada"},
                      {"Ashley", "Guialem", "Sydney", "Sadie", "Raji", "Annie", "Tess", "Annika",
                        "Olana", "Claire"}, 
                      {"Morgan", "Grace", "Sallie", "Esme", "Sophie", "Bella"}
                    };

// Create a new instance of the MD_Parola class with hardware SPI connection:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
int shuffledIndexes[maxClassSize];
int thisClass = 0;
int thisStudent = 0;

void shuffle_students(int n) {
  for (int i = 0; i < n; i++){
    shuffledIndexes[i] = i;
  }
  for (int i = 0; i < n; i++) {
    int j = random(0, n);
    int t = shuffledIndexes[i];
    shuffledIndexes[i] = shuffledIndexes[j];
    shuffledIndexes[j] = t;
  }  
}  

void setup() {
  //Serial.begin(9600);
  pinMode(NEXT_PIN, INPUT_PULLUP); //The pushbutton pin will have an internal resistor pulling the value high when not depressed
  pinMode(CLASS_PIN, INPUT_PULLUP);
  randomSeed(analogRead(0)); //get noise from pin 0 as seed for random number generator
  shuffle_students(numberOfStudents[thisClass]);
  Serial.println("analogread ");
  Serial.println(analogRead(0));
  for (int i = 0; i < numberOfStudents[thisClass]; i++) {
    Serial.println(classRosters[thisClass][shuffledIndexes[i]]);
  }
  myDisplay.begin();  //Start the display module
  myDisplay.setIntensity(7);  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.displayClear();
  delay(100);
  myDisplay.print("Select");
  delay(500);
  myDisplay.print("Class");
  delay(500);
  myDisplay.displayClear();
/* while (digitalRead(CLASS_PIN) and digitalRead());
    thisClass++;
  } else if (not c2) {
    thisClass = 1; 
  } else {
    thisClass = 2;
  }
*/

  //Serial.println("thisClass ");
  //Serial.println(thisClass);
  

  //for (int i = 0; i < numberOfStudents[thisClass] - 1; i++) {
  //  Serial.println(shuffle[i]);
  //}
}

void wakeUp(){
}


void loop() {
  thisStudent = (thisStudent + 1) % numberOfStudents[thisClass];
  //for (int i = 0; i < numberOfStudents[thisClass]; i++) {
  int del = 0;
  for (int i = 0; i < 4 * numberOfStudents[thisClass]; i++) {
    int student = (thisStudent + i) % numberOfStudents[thisClass];
    myDisplay.print(classRosters[thisClass][student]);
    del = del + i/4;
    delay(del);
  }  
    myDisplay.setInvert(true);
    myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
    delay(200);
    myDisplay.setInvert(false);
    myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
    delay(200);
    myDisplay.setInvert(true);
    myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
    delay(200);
    myDisplay.setInvert(false);
    //Serial.println("Winner is:");
    //Serial.println(classRosters[thisClass][shuffledIndexes[currentStudent]]);
    myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
    delay(5000);
    myDisplay.displayClear();
    //low power mode
    attachInterrupt(digitalPinToInterrupt(NEXT_PIN), wakeUp, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    detachInterrupt(digitalPinToInterrupt(NEXT_PIN)); 
//   while (digitalRead(NEXT_PIN) == LOW) {}
}
