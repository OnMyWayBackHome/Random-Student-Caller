//Program to randomly shuffle the names of students in a class and display a different student name
//every time a button is pressed. Display is a 4 x 8x8 Dot matrix panel controlled by MAX7219
#include "Arduino.h"
#include <MD_Parola.h>  //Library of nice functions for the display
#include <MD_MAX72xx.h> //Driver for display
#include <SPI.h>
#include "LowPower.h"
#define CLASS_PIN 3 //pin for the pushbutton to change classes
#define NEXT_PIN 2 //Pin to get next student
#define CS_PIN 4  //control pin for the display
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  //Number of 8x8 LED matrices
#define DISPLAY_SCROLL_SPEED  50 //msec between frames
#define DISPLAY_SCROLL_PAUSE_TIME  0
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); //new instance of MD_Parola class with hardware SPI

//Edit these variables to match your situation.
const int maxClassSize = 12;  //Enter the number of students in the largest class
const int numberOfClasses = 3; //How many classes
const int numberOfStudents[] = {12, 10, 6}; //Enter the number of students in each class
const char *classNames[] = {"Geo A", "Geo D", "C-Alg"}; //Abbreviate to < 8 characters to fit on display
const char *classRosters[numberOfClasses][maxClassSize] = {
            {"Angie", "Jill", "Mairi", "Allie", "Jacey", "Jennifer", "Jojo", "Nozomi",
              "Lillian", "Erica", "Mehar", "Ada"},
            {"Ashlyn", "Guialem", "Sydney", "Sadie", "Raji", "Annie", "Tess", "Annika",
              "Olana", "Claire"}, 
            {"Morgan", "Grace", "Sallie", "Esme", "Sophie", "Bella"},
          };

void shuffleStudents(); //declare functions
void changeClass();
void getNextStudent();
void wakeUp();

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
  myDisplay.displayReset();
  changeClass(); //go to the next class and display its name.
}

void loop() {
  //go into low power mode and wait for a button to be pushed.
  attachInterrupt(digitalPinToInterrupt(NEXT_PIN), wakeUp, LOW); //enable interrupts
  attachInterrupt(digitalPinToInterrupt(CLASS_PIN), wakeUp, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  detachInterrupt(digitalPinToInterrupt(CLASS_PIN));  //disable interrupts until after you deal with the first.
  detachInterrupt(digitalPinToInterrupt(NEXT_PIN)); 
  do { //read both buttons to see which has been pushed
    classPinUp = digitalRead(CLASS_PIN);
    nextPinUp = digitalRead(NEXT_PIN);
  } while (classPinUp and nextPinUp);
 
  if (!classPinUp) {
    changeClass();
  } else {
    getNextStudent();
  }
}

void changeClass() {
  thisClass = (thisClass + 1) % numberOfClasses;
  shuffleStudents();
  myDisplay.displayShutdown(false);
  myDisplay.print(classNames[thisClass]);
  delay(800);
  myDisplay.displayClear();
  myDisplay.displayShutdown(true); //put into low power mode
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
  myDisplay.displayShutdown(false); //wake up display from low power mode
  thisStudent = (thisStudent + 1) % numberOfStudents[thisClass];
  int del = 0;
  for (int i = 0; i < 40; i++) {
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
      //If the name it too long to fit, scroll the name
      myDisplay.displayClear();
      myDisplay.displayText(classRosters[thisClass][shuffledIndexes[thisStudent]], PA_CENTER, DISPLAY_SCROLL_SPEED, DISPLAY_SCROLL_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      while(true) { //wait for animation to finish
        if (myDisplay.displayAnimate()) {
          myDisplay.displayReset();
          break;
        }
      }
    } else {
      myDisplay.print(classRosters[thisClass][shuffledIndexes[thisStudent]]);
      delay(4000);
    }
    myDisplay.displayClear();
    myDisplay.displayShutdown(true); //put into low-power mode
}

void wakeUp(){
}
