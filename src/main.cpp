//Program to shuffle the names of students in a class and display a different student name
//every time a button is pressed. Display is a 4 x 8x8 Dot matrix panel controlled by MAX7219
#include <Arduino.h>
#include <MD_Parola.h>  //Library of nice functions for the display
#include <MD_MAX72xx.h> //Driver for display
#include <SPI.h>
#include "LowPower.h"
#include <SD.h> // SD card read/write ** MOSI - pin 11, ** MISO - pin 12, ** CLK - pin 13, ** CS - pin 10 
//#include <Wire.h>
#include <serialEEPROM.h>

#define NEXT_PIN 2 //Pin to get next student
#define CLASS_PIN 11 //pin for the pushbutton to change classes
#define CS_PIN_LED_MATRIX 4  //control pin for the display
#define CS_PIN_SD_CARD 10 //chip select for the micro SD Card
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  //Number of 8x8 LED matrices
#define DISPLAY_SCROLL_SPEED  50 //msec between frames
#define DISPLAY_SCROLL_PAUSE_TIME  0
#define DISPLAY_BRIGHTNESS 3 // The intensity (brightness) of the display (0-15):


const byte maxClassSize = 25;  //size of largest class that you anticipate
const byte maxNumberOfClasses = 6;  
const byte maxNameSize = 12;  //number of characters in the longest student name
const int addrEEPROM = 0x50; //I2C Device Address 0x50 (A0 = GND, A1 = GND)
const byte  addrVersionNumber = 0; //put code into eeprom to prompt for new data if no data is present or of data is of old format.
const byte versionNumber = 0xD7; //current version number (random number)
const byte addrDeviceName = 0x02;//Eprom address locations of 8 byte character array of the name of the device.
const byte addrNumberOfClasses = 0x0B;
const byte addrClassSizes = 0x10; //Start of array of 16 class sizes
const int addrClassNames = 0x30;  
const int addrStudentNames = 0x100; 

char deviceName[8];
byte classSizes[maxNumberOfClasses] ; //Enter the number of students in each class
char classNames[maxNumberOfClasses][8]; //Abbreviate to < 8 characters to fit on display
char studentNames[maxClassSize][maxNameSize];
byte shuffledIndexes[maxClassSize];
byte numberOfClasses = 0; 
byte thisClass = 0;
byte thisStudent = 0;
bool nextPinUp = false;
bool classPinUp = false;


void shuffleStudents(); //declare functions
void startDisplay();
void changeClass();
void getNextStudent();
void wakeUp();
void setPinModes();
void initializeSDCardReader();
void loadNewRoster();
int readClassNamesFromEprom();
int readLineFromSDWriteToEprom(File myFile, int maxLen, int addr); 
void readStudentsInClass(int classNumber);
void wakeDisplay();
void sleepDisplay();

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN_LED_MATRIX, MAX_DEVICES); //new instance of MD_Parola class with hardware SPI 
serialEEPROM myEEPROM(addrEEPROM, 32768, 64); //address; 256K bit = 32768 bytes; 64-Byte Page Write Buffer

void setup() {
  setPinModes();
  Serial.begin(9600);
  while (!Serial) {}
  Serial.print("displaying something");
  // loadNewRoster();  //FIX TO LOAD ONLY IF BUTTON IS PRESSED ON STARTUP
  Serial.println("after loadnew roster: ");
  readClassNamesFromEprom();
  Serial.println("after readClassNames: ");
  startDisplay();
  Serial.println ("after startDisplay: ");
  myDisplay.print("Derby"); //display my silly name for the random caller. "I didn't call on you--Derby did!"
  while (digitalRead(CLASS_PIN)){} //wait for the button to be pressed so I can get a random time.
  randomSeed(millis());  //initialize my random number generator
  myDisplay.displayClear();
  changeClass(); //go to the next class and display its name.
  Serial.println ("after changeClass ");

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
    Serial.println ("change Class button pressed ");
    changeClass();
  } else {
    Serial.println ("next STudent button pressed ");
    getNextStudent();
  }
}

void setPinModes(){
  pinMode(NEXT_PIN, INPUT_PULLUP); //The pushbutton pin will have an internal resistor pulling the value high when not depressed
  pinMode(CLASS_PIN, INPUT_PULLUP);
  pinMode(CS_PIN_LED_MATRIX, OUTPUT);
  pinMode(CS_PIN_SD_CARD, OUTPUT);
  digitalWrite(CS_PIN_SD_CARD, HIGH); // set Chip Select for the card reader to high to disable the SD Card Reader until needed
}

void startDisplay(){
  myDisplay.begin();
  myDisplay.setIntensity(DISPLAY_BRIGHTNESS);  
  myDisplay.setTextAlignment(PA_CENTER);
  wakeDisplay();
}

void wakeDisplay() {
  myDisplay.displayShutdown(false);
  myDisplay.displayClear();
  myDisplay.displayReset();
}

void sleepDisplay() {
  myDisplay.displayShutdown(true);
}

void changeClass() {
  thisClass = (thisClass + 1) % numberOfClasses;
  readStudentsInClass(thisClass);
  shuffleStudents();
  wakeDisplay();
  myDisplay.print(classNames[thisClass]);
  Serial.print("Class name displayed: ");
  Serial.println(classNames[thisClass]);
  delay(800);
  sleepDisplay();
}

void shuffleStudents() {
  for (byte i = 0; i < classSizes[thisClass]; i++){
    shuffledIndexes[i] = i;
  }
  for (byte i = 0; i < classSizes[thisClass]; i++) {
    byte j = random(0, classSizes[thisClass]);
    byte t = shuffledIndexes[i];
    shuffledIndexes[i] = shuffledIndexes[j];
    shuffledIndexes[j] = t;
  } 
}  

void getNextStudent() {
  wakeDisplay();
  thisStudent = (thisStudent + 1) % classSizes[thisClass];
  int del = 0;
  for (byte i = 0; i < 30; i++) {
    int student = (thisStudent + i) % classSizes[thisClass];
    Serial.print("displaying ");
    Serial.println (studentNames[student]);
    myDisplay.print(studentNames[student]);
    del = del + i/4;
    delay(del);
  }  
    myDisplay.setInvert(true);
    myDisplay.print(studentNames[shuffledIndexes[thisStudent]]);
    Serial.print("Final student ");
    Serial.println(studentNames[shuffledIndexes[thisStudent]]);
    delay(200);
    myDisplay.setInvert(false);
    if (myDisplay.getTextColumns(studentNames[shuffledIndexes[thisStudent]]) > MAX_DEVICES * 8) {
      //If the name it too long to fit, scroll the name
      myDisplay.displayClear();
      myDisplay.displayText(studentNames[shuffledIndexes[thisStudent]], PA_CENTER, DISPLAY_SCROLL_SPEED, DISPLAY_SCROLL_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      while(true) { //wait for animation to finish
        if (myDisplay.displayAnimate()) {
          myDisplay.displayReset();
          break;
        }
      }
    } else {
      myDisplay.print(studentNames[shuffledIndexes[thisStudent]]);
      delay(4000);
    }
    sleepDisplay();
}

void wakeUp(){
}

void initializeSDCardReader(){
 //Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

int readLineFromSDWriteToEprom(File myFile, int maxLen, int addr) {
  //read a line or up to maxLen characters from myFile on the SD card. Store the string to EEPROM at address addr
  byte i = 0;
  char buffer[16];
  while (myFile.available()) {
    char c = myFile.read();
    
    if (c == '\n') {
      buffer[i] = 0;  // add a null to terminate the string.
      break;
    } else if (c == '\r' || i >= maxLen - 1) {
      buffer[i] = 0; // ignore the carriage return or characters longer than the max allowed.
    } else {
      buffer[i++] = c;
    }
  }
  if (buffer[0] == 0x2A) { //check for "*" which signals end of class
    return -1;
  } else {
    myEEPROM.write(addr, (uint8_t*)buffer, i + 1);
    return i;
  }
}

void loadNewRoster(){
  initializeSDCardReader();
  File myFile;
  myFile = SD.open("roster.txt");
  if (!myFile) {
    Serial.println("error opening file on SD Card");    
    while(true) {}
  } 
  readLineFromSDWriteToEprom(myFile, 8, addrDeviceName); //read the device name Derby etc from the first line of the text file on the SD card
  int currentClass = 0;
  while (myFile.available() && currentClass < maxNumberOfClasses){  
    readLineFromSDWriteToEprom(myFile, 8, addrClassNames + currentClass * 8); // read the course name and store it in the EEPROM
    int currentStudent = 0;
    while (myFile.available() && currentStudent < maxClassSize){
      int flag = readLineFromSDWriteToEprom(myFile, maxNameSize, addrStudentNames + currentClass * maxClassSize * maxNameSize + currentStudent * maxNameSize); //read student names
      if(flag < 0){
        break;
      } else {
        currentStudent++;
      }
    }
    myEEPROM.write(addrClassSizes + currentClass, currentStudent); // store the number of students in this class
    currentClass++;
  }
  myEEPROM.write(addrNumberOfClasses, currentClass);
  myFile.close();
  digitalWrite(CS_PIN_SD_CARD, HIGH); // set Chip Select for the card reader to high to disable the SD Card Reader until needed
  return;
}

void readStudentsInClass(int classNumber){
  for (byte i = 0; i < classSizes[classNumber]; i++) {
    Serial.print("Student: ");
    myEEPROM.read(addrStudentNames + classNumber * maxClassSize * maxNameSize + i * maxNameSize, (uint8_t*)studentNames[i], maxNameSize);
    Serial.println(studentNames[i]);
  }
  return;
}

int readClassNamesFromEprom(){
  /* 
  /if (myEEPROM.read(addrVersionNumber)= versionNumber) {
    Serial.print("EEPROM has not been programmed yet."); //UPDATE. PUT THIS MESSAGE ON THE LED MATRIX.
    while(true) {}
  }
  */
  Serial.print("Device name in nEEPROM: ");
  myEEPROM.read(addrDeviceName, (uint8_t*)deviceName, 8);
  Serial.println(deviceName);

  numberOfClasses = myEEPROM.read(addrNumberOfClasses);
  thisClass = numberOfClasses;
  Serial.print("Number of Classes in EEPROM: ");
  Serial.println(numberOfClasses);
  for (int i = 0; i < numberOfClasses; i++ ) {
    Serial.print("Class: ");
    myEEPROM.read(addrClassNames + i * 8, (uint8_t*)classNames[i], 8);
    Serial.println(classNames[i]);

    classSizes[i] = myEEPROM.read(addrClassSizes + i);
    Serial.print("Students in this class: ");
    Serial.println(classSizes[i]);
  }
  return  1;
}
