#include <RCSwitch.h>
#include <EEPROM.h>

RCSwitch mySwitch = RCSwitch();
const int MAXSTORAGE = 100;
const int EEPROMLocation = 0;
unsigned long codeArray[MAXSTORAGE]; //create Array for saved codes
unsigned long timerArray[MAXSTORAGE];
int arraylength = sizeof(codeArray) / sizeof(codeArray[0]); //The actual length of storage

const unsigned long debouncePeriod = 1000;  //the time we wait for debouncing in millis 
const unsigned long savePeriod = 5000;  //save if held for 5 seconds
const unsigned long clearPeriod = 10000;  //delete if held for 10 seconds

unsigned long pressMillis; //time when button is pressed
unsigned long holdMillis; //measures duration of the hold
unsigned long debounceMillis; //Measures the time when a key is recieved

const int buttonPin = 12;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
int buttonState = HIGH;         // variable for reading the pushbutton status
unsigned long value = 0;  //Storage for recieved keys

String message = "AT!GX APP SENDGNXMSG";
String command = "";
String key = "";

void setup() {
  Serial.begin(9600);
  Serial.println("Powered on");
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pinMode(ledPin, OUTPUT); // initialize the LED pin as an output:
  pinMode(buttonPin, INPUT_PULLUP); // initialize the pushbutton pin as an input:

  EEPROM.get( EEPROMLocation, codeArray ); //first thing we do during start up is to load EEPROM into a local array.
  
  buttonState = digitalRead(buttonPin);
  if(buttonState == LOW){
    pressMillis = millis(); //Static time that is measure when button is pressed ex)pressMillis = 0
  }
  while(buttonState == LOW){
    holdMillis = millis(); //this timer will grow after each loop until ex)holdMillis = 10000
    if(holdMillis - pressMillis >= clearPeriod){
      clearKeys();  
      break;
    }
  }
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if(buttonState == LOW){
    pressMillis = millis(); //ex)pressMillis = 0
  }
  while(buttonState == LOW){
    buttonState = digitalRead(buttonPin);
    holdMillis = millis(); //check how long is held. ex)holdMillis = 5000
    if(holdMillis - pressMillis >= savePeriod){
      saveKey();
      mySwitch.resetAvailable();
    }
  }
  if (mySwitch.available()){
    recieveKey();
    mySwitch.resetAvailable();
  }
}

void clearKeys(){
  Serial.println("now clearing keys");
  for (int i=0 ; i<30 ; i++){
    digitalWrite(ledPin, HIGH); //LED ON
    delay(50);
    digitalWrite(ledPin, LOW); //LED OFF
    delay(50);
  }
  for(int i=0; i<arraylength; i++){
    codeArray[i]=0; //clear the local array
    EEPROM.put(EEPROMLocation, codeArray);   //Saves cleared array to EEPROM
  }
}
void saveKey(){
  Serial.println("now saving keys");
  for (int i=0 ; i<3 ; i++){
    digitalWrite(ledPin, HIGH); //LED ON
    delay(750);
    digitalWrite(ledPin, LOW); //LED OFF
    delay(750);
  }
  unsigned long value = mySwitch.getReceivedValue();
  if (value == 0) {
    Serial.println("Unknown encoding");
    return;
  }
  //Check our code array to see if valid input
  for(int i=0; i < arraylength; i++){
    if(codeArray[i]==value){ //if we found it 
      Serial.println("Key already exists");
      return;
    }
  }
  for(int i=0; i<arraylength; i++){
    if(codeArray[i] == 0){
      codeArray[i] = value;   //Any time we modify the local the array it needs to be saved
      EEPROM.put(EEPROMLocation, codeArray);   //Saves the current code array into EEPROM
      Serial.println("key saved");
      return;
    }
  }
  Serial.println("Key storage is full");  
}
void serialSave(){
  Serial.println("saving key from serial input");
  if(key != NULL){
    value = key.toInt();  //handles Serial commands
  }
  if (value == 0) {
    Serial.println("Unknown encoding");
    return;
  }
  //Check our code array to see if valid input
  for(int i=0; i < arraylength; i++){
    if(codeArray[i]==value){ //if we found it 
      Serial.println("key already exists");
      return;
    }
  }
  for(int i=0; i<arraylength; i++){
    if(codeArray[i] == 0){
      codeArray[i] = value;   //Any time we modify the local the array it needs to be saved
      EEPROM.put(EEPROMLocation, codeArray);   //Saves the current code array into EEPROM
      Serial.println("key saved");
      return;
    }
  }
  Serial.println("Key storage is full");    
}
void deleteKey(){
  Serial.println("deleting key");
  if(key != NULL){
    value = key.toInt();  //handles Serial commands
  }
  if (value == 0) {
    Serial.println("Unknown encoding");
    return;
  }
  //Check our code array to see if valid input
  for(int i=0; i < arraylength; i++){
    if(codeArray[i]==value){ //if we found it 
      codeArray[i]=0;
      EEPROM.put(EEPROMLocation, codeArray);   //Saves the current code array into EEPROM
      Serial.println("key deleted");
      return;
    }
  }
  Serial.println("The selected Key doesnt exist"); 
}
void recieveKey(){
  unsigned long debounceMillis = millis();
  unsigned long value = mySwitch.getReceivedValue();
  if (value == 0) {
    Serial.println("Unknown encoding");
    } else {
    //Check our code array to see if valid input
    for(int i=0; i < arraylength; i++){
      if(codeArray[i]==value){ //if we found it 
        if(timerArray[i]==0){ //check the timer if this is first time we make a timer
          Serial.println(message + value);
          timerArray[i] = debounceMillis + 1000;
        }
        else if(debounceMillis  >= timerArray[i]){
          Serial.println(message + value);
          timerArray[i] = debounceMillis + 1000;
        }
      }
    }
  }
}
void serialEvent() {  //Logic for taking serial ATCommands
  if (Serial.available()) {
    command="";
    key = "";
    command = Serial.readStringUntil(';');  //commands are delimited by semicolon
    if(command.substring(0,7) == "ATLEARN"){
      key = command.substring(8);
      serialSave();
    }
    if(command.substring(0,7) == "ATCLEAR"){
      key = command.substring(8);
      deleteKey();
    }
    if (command.substring(0,9) == "ATFACTORY") {
      clearKeys();
    }
  }  
}
