//Program to randomly shuffle the names of students in a class and display a different student name
//ever time a button is pressed. Display is a 4 x 8x8 Dot matrix panel controlled by MAX7219
#include "Arduino.h"
#include <MD_Parola.h>  //Library of nice functions for the display
#include <MD_MAX72xx.h> //Driver for display
#include <SPI.h>
#include "LowPower.h"

#define CS_PIN 3  //control pin for the display
#define CLASS_PIN 4 //pin for the pushbutton to change classes
#define NEXT_PIN 2 //Pin to get next student
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  //Number of 8x8 LED matrices

//Edit these variables to match your situation.
const int maxClassSize = 12;  //Enter the number of students in the largest class
const int numberOfClasses = 3; //How many classes
const int numberOfStudents[] = {12, 10, 6}; //Enter the number of students in each class
const char *classNames[] = {"Geo A", "Geo D", "C-Alg"};
const char *classRosters[numberOfClasses][maxClassSize] = {
                      {"Angie", "Jill", "Mairi", "Allie", "Jacey", "Jennifer", "Jojo", "Nozomi",
                        "Lillian", "Erica", "Mehar", "Ada"},
                      {"Ashlyn", "Guialem", "Sydney", "Sadie", "Raji", "Annie", "Tess", "Annika",
                        "Olana", "Claire"}, 
                      {"MorganANA", "GraceBEHERE", "Sallie", "Esme", "Sophie", "Bella"},
                    };

void shuffleStudents(); //so I can put function after loop()
void changeClass();
void getNextStudent();

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); // A new instance of the MD_Parola class with hardware SPI connection:
int shuffledIndexes[maxClassSize];
int thisClass = numberOfClasses - 1;
int thisStudent = 0;
int nextPinUp = false;
int classPinUp = false;

void setup() {
   //Serial.begin(9600);
  pinMode(NEXT_PIN, INPUT_PULLUP); //The pushbutton pin will have an internal resistor pulling the value high when not depressed
  pinMode(CLASS_PIN, INPUT_PULLUP);
  pinMode(CS_PIN, OUTPUT);
  randomSeed(analogRead(0)); //get noise from pin 0 as seed for random number generator

  myDisplay.begin();  //Start the display module
  myDisplay.setIntensity(7);  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.displayClear();
  delay(100);
  changeClass(); //go to the next class and display its name.
}

void wakeUp(){
}

void loop() {
  do {
    classPinUp = digitalRead(CLASS_PIN);
    nextPinUp = digitalRead(NEXT_PIN);
  } while (classPinUp and nextPinUp);
  if (!classPinUp) {
    changeClass();
  } else {
    getNextStudent();
  }

/* 
    //low power mode
    attachInterrupt(digitalPinToInterrupt(NEXT_PIN), wakeUp, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    detachInterrupt(digitalPinToInterrupt(NEXT_PIN)); 
//   while (digitalRead(NEXT_PIN) == LOW) {}
*/
}

void changeClass() {
  thisClass = (thisClass + 1) % numberOfClasses;
  myDisplay.print(classNames[thisClass]);
  delay(800);
  myDisplay.displayClear();
  shuffleStudents();
}

void shuffleStudents() {
  for (int i = 0; i < numberOfStudents[thisClass]; i++){
    shuffledIndexes[i] = i;
  }
  for (int i = 0; i < numberOfStudents[thisClass]; i++) {
    int j = random(0, numberOfStudents[thisClass]);
    int t = shuffledIndexes[i];
    shuffledIndexes[i] = shuffledIndexes[j];
    shuffledIndexes[j] = t;
  } 
  /*  for (int i = 0; i < numberOfStudents[thisClass]; i++) {
      Serial.print(classRosters[thisClass][shuffledIndexes[i]]);
      Serial.print(" ");
     Serial.println(strlen(classRosters[thisClass][shuffledIndexes[i]]));
  }
  */ 
}  

void getNextStudent() {
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
    if (myDisplay.getTextColumns(classRosters[thisClass][shuffledIndexes[thisStudent]]) > MAX_DEVICES * 8) {
      myDisplay.displayClear();
      myDisplay.displayScroll(classRosters[thisClass][shuffledIndexes[thisStudent]], PA_LEFT, PA_SCROLL_LEFT, 0);
    } else {
      myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
    }
    delay(4000);
    myDisplay.displayClear();
}