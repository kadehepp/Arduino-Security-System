//Kade and Cade 
//Final 
const int unlock = 1111; 
const int lock = 2222; 
const int armed = 3333; 
const int silent = 4444; 
const int off = 5555;
const int disarm = 9999; 
//LED Pins
const int redPin = 2;
const int yellowPin = 3;
const int greenPin = 4; 
const int bluePin = 6; //
const int whitePin = 5;

//UltraSonic Sensor 
const int trigPin = 12;
const int echoPin = 13;
long time;
float distance; 

//Sound Sensor 
const int digitalSoundPin = 5; 
const int analogSoundPin = A0; 

float voltage;
float db = 0.0;

int loudSoundCount = 0;

//time variables
unsigned long lastUnlock = 0; //for reset back to locked after time period
unsigned long lastTimeoutPrint = 0;
const unsigned long UNLOCKTIMEOUT = 15000; //15 sec interval while unlocked


unsigned long lastFlash = 0; //flash for alarm triggeres(leds)
const unsigned long FLASHINTERVAL = 500; //flash lights every 500 ms
bool flash = false; //used to swap between high and low during triggered state
unsigned long lastSilentPrint = 0;

unsigned long lastPrint = 0; //print every one second
const unsigned long PRINTINTERVAL = 1000;

unsigned long sensorReadTime = 0; //read fevery 100ms, prevent overload, sensor works better
unsigned long soundWindowTime = 0; //window only active after first 0.5 reading-> 2000ms
const unsigned long SOUNDWINDOWINTERVAL = 3000; // 2 second window
int windowCount = 0; //3+ readings in 2 sec = 1 loudSound
bool windowActive = false; //only active for two seconds


unsigned long lastSoundFlash = 0;
bool soundFlash = false;

//States 
enum SysLock {
	LOCKED,
  	UNLOCKED
};

enum SysMode {
	OFF, //do nothing
  	SILENT, //quiet alerts
  	ARMED //active alarm
};


enum AlarmState {
  ALARM_WAIT, //idle
  ALARM_TRIGGERED, //activated, flash lights
  ALARM_DISABLED //probably dont need, only used for message currently
};

//message states, easy to carry information with low data use over long strings+ints 
enum DistanceWarning {
  DIST_SAFE, //nothing in range, no issue, green light
  DIST_APPROACHING, //starting to sense, yellow
  DIST_CLOSE, //object close, red
  DIST_TRIGGER, //trigger alarm
};
enum SoundWarning{
  SOUND_SAFE,
  SOUND_ONE,
  SOUND_TWO,
  SOUND_TRIGGER //loud noise detected, maybe add a 3+ reading count of a loud noise before trigger?
  //...add more with more triggers
};

SysLock sysLock = LOCKED;
SysMode sysMode = OFF;

AlarmState alarmState = ALARM_WAIT;

DistanceWarning distanceWarning = DIST_SAFE;
SoundWarning soundWarning = SOUND_SAFE;




//SENSOR INPTU LOGIC
void runUltrasonic(){
  //read distance sensor 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  time = pulseIn(echoPin , HIGH);
  distance = time * 0.0343/2;
  //Serial.print("In FX: ");
  //Serial.println(distance);  
  
}
void getSoundReading(){
  
  int analogValue = analogRead(A0);

  // Convert the analog value to a voltage
  voltage = (float)analogValue * (5.0 / 1023.0);

  // Calculate the decibel level
  // This formula assumes a reference sound pressure level.
  // The 'quietVoltage' acts as a reference point for 0dB.
  // The factor of 20 is used for sound pressure level (SPL) in dB.
   db = 20 * log10(voltage / 2.5);

  // Print the results to the serial monitor
 
  
}
//handle sensor input, set warning states
void monitorSensors(){
  if(millis() - sensorReadTime >= 100){
    sensorReadTime = millis();
    getSoundReading();
    runUltrasonic();
  
  }
  
  

  //if off  dont use sensors or compute trigger
  if(sysMode == OFF){
    return;
  }
  

  if(distance <= 10 ){
    distanceWarning = DIST_TRIGGER;
    if(alarmState != ALARM_TRIGGERED){
      triggerAlarm();
    }
    
  }
  else if(distance <= 80){
    distanceWarning = DIST_CLOSE;
  }
  else if(distance <= 150){
    distanceWarning = DIST_APPROACHING;
  }
  else{
    distanceWarning = DIST_SAFE;
  }

//read too fast, 1 single noise still produces 3 readings->auto trigger at one real sound
  if(db >= 0.5 ){
    unsigned long currentTime = millis();
    Serial.print("sound recieved: ");
    Serial.println(db);
    if(!windowActive){
      windowActive = true;
      windowCount = 1;
      soundWindowTime = currentTime; 

    }
    else if (millis() - soundWindowTime <= SOUNDWINDOWINTERVAL){ //new reading within window
      windowCount++;
      Serial.print("readings within window: ");
      Serial.println(windowCount);
    }
    else{ //window expired, but new reading entered
      windowActive = true;
      windowCount = 1;
      soundWindowTime = currentTime;
      Serial.println("previous window expired, starting new window");
    }
    if(windowCount >= 3){
      loudSoundCount++;

      if(loudSoundCount == 1)
      {
        soundWarning = SOUND_ONE;

      }
      else if(loudSoundCount == 2){

        soundWarning = SOUND_TWO;
      }
      else if (loudSoundCount >= 3){
        soundWarning = SOUND_TRIGGER;
        if(alarmState != ALARM_TRIGGERED){
          triggerAlarm();
        }
      
      }
      //reset window variables
      windowActive = false;
      windowCount = 0;
      Serial.println("sound event recorded");
    }
    
    
   
  }
  //must check reset(window only) here as well, not handled if windowCount < 3
  if(windowActive && millis() - soundWindowTime >= SOUNDWINDOWINTERVAL){
    windowActive = false;
    windowCount = 0;
  }
}


//CHECK USER INPUT FUNCTIONS
void checkInput(){ 
  if(Serial.available() > 0){ 
    String incomingString = Serial.readString(); 
    incomingString.trim(); 
    if(incomingString.length() != 4){ 
      Serial.println("incorrect pin length"); 
    }  
    for(int i=0; i< incomingString.length(); i++){ 
      if(!isDigit(incomingString.charAt(i))){ 
        Serial.println("pin must be all digits"); 
      } 
    } 
    int inputPin = incomingString.toInt(); 
    checkPin(inputPin); 
   
  } 
} 
//check valid 4 digit pin
void checkPin(int pin){ 
//if alarm triggered disable it first if correct pin is input
  if(alarmState == ALARM_TRIGGERED && pin == disarm){
    disableAlarm();
    return;
  }
  //change lock/unlock state 

  if(pin == unlock){ 
    sysLock = UNLOCKED; 
    lastUnlock = millis();
    Serial.println("system unlocked"); 
    return;
  } 
  else if(pin == lock){ 
    sysLock = LOCKED; 
    Serial.println("system locked");
    return;
  } 
  //only if system is unlocked take other pins to change substates 
  if(sysLock == LOCKED){
    Serial.println("System Locked, please enter unlock pin to access controls"); 
    return;
  }
  switch(pin){
    case armed:
      sysMode = ARMED; 
      alarmState = ALARM_WAIT;
      resetLed();
      Serial.println("armed mode on");
      break;
    case silent:  
      sysMode = SILENT; 
      alarmState = ALARM_WAIT;
      resetLed();
      Serial.println("silent mode on"); 
      break;
    case off:
      sysMode = OFF;
      alarmState = ALARM_WAIT;  
      resetLed();
      Serial.println("OFF mode"); 
      break;
    default:
      Serial.println("invalid pin");  
  }  
} 


//reset to locked state after 15 second interval, prevent system left unlocked
void checkUnlockTimeout(){
  
  if(sysLock == UNLOCKED){
    unsigned long currentTime = millis();

    if(currentTime - lastUnlock >= UNLOCKTIMEOUT){
      sysLock = LOCKED;
      Serial.println("timeout reached, system locked");
    }
    else{
      unsigned long timeLeft = UNLOCKTIMEOUT - (currentTime - lastUnlock);


      if(currentTime - lastTimeoutPrint >= 1000){ //print once a second, 
        lastTimeoutPrint = currentTime;
        Serial.print("time before lock: ");
        Serial.println(timeLeft);
      }
    }
  }
}

//ALARM FUNCTIONS
//set alarm state to triggered, set time variable for alarm flash(LEDs)
void triggerAlarm(){
  alarmState = ALARM_TRIGGERED;
  resetLed();
  if(distanceWarning == DIST_TRIGGER){
    Serial.println("distance alarm triggered");
  }
  else if(soundWarning == SOUND_TRIGGER){
    Serial.println("sound alarm triggered");
  }
  
  //
  if(sysMode == SILENT){
    Serial.println("silent alarm, notifying authorities");
    return;
  }
  lastFlash = millis();
}

//handle flash
void handleAlarm(){
  if(alarmState != ALARM_TRIGGERED){
    return;
  }
//only flash if system armed, else silent msg
  unsigned long currentTime = millis();
  if(sysMode == ARMED){
    
    if(currentTime - lastFlash >= FLASHINTERVAL){
      flash = !flash; //swap true false, high low
      digitalWrite(redPin, flash);
      digitalWrite(yellowPin, flash);
      digitalWrite(greenPin, flash);
      lastFlash = currentTime;
    }
  }
  else if(sysMode == SILENT){
    unsigned long timeSince = millis() - lastFlash; 
    if(millis()- lastSilentPrint >= 5000){
      lastSilentPrint = currentTime;
      Serial.print("Time since authorities contacted: ");
      Serial.println(timeSince);
    }
      
  }
}
//disable alarm if triggered+correct pin input
void disableAlarm(){
  //alarmState = ALARM_DISABLED;
  distanceWarning = DIST_SAFE;
  soundWarning = SOUND_SAFE;
  loudSoundCount = 0;
  windowCount = 0;
  windowActive = false;
  soundWindowTime = 0;


  resetLed();
  Serial.println("Alarm DISARMED");

  //maybe print to user what happened to trigger alarm

  alarmState = ALARM_WAIT;

}
  
//handle led logic(besides alrm trig)
void updateLED(){

//dont update normally if triggered, off
  if(alarmState == ALARM_TRIGGERED || sysMode == OFF) {
    return;
  }
  

//show which active mode you are in

//very slight flicker but keeps white blink intact and not noticibly bad
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, LOW);

  switch(sysMode){
    case SILENT: 
      digitalWrite(whitePin, HIGH);
      break;  
    case ARMED:
      digitalWrite(bluePin, HIGH);
      if(alarmState == ALARM_WAIT){
        //handle warning lights
        switch(distanceWarning){ 
          case DIST_SAFE:
            
            digitalWrite(greenPin, HIGH);
            break;
          case DIST_APPROACHING:
            
            digitalWrite(yellowPin, HIGH);
            break;
          case DIST_CLOSE:
            digitalWrite(redPin, HIGH);
            break;  

        }
        switch(soundWarning){
          case SOUND_SAFE:


            break;
          case SOUND_ONE:
            if(millis() - lastSoundFlash >= 500){ //flash normal
              lastSoundFlash = millis();
              soundFlash = !soundFlash;
              digitalWrite(whitePin, soundFlash);
            }
            break;
          case SOUND_TWO:    
            if(millis() - lastSoundFlash >= 250){ //fast flash
              lastSoundFlash = millis();
              soundFlash = !soundFlash;
              digitalWrite(whitePin, soundFlash);
            }
            break;
        }
      }
      break;
    default:
      break;

  }
  
    
}
void resetLed(){ //only call on state change(off, armed, silent, disarm)
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(whitePin, LOW);
}

//handle all prints
void printData(){ 
//only print once a second
  if(millis() - lastPrint >= PRINTINTERVAL){
    lastPrint = millis();

    //sensor data, remove in final version most likely
    Serial.print("distance: ");
    Serial.print(distance);
    Serial.print("cm | sound: ");

    Serial.print(db);
    Serial.print(" db, voltage: ");
    Serial.print(voltage);
    Serial.print(" V ");

    
    //print sysMode
    Serial.print("| Mode: ");
    switch(sysMode){
      case OFF: 
        Serial.print("OFF");
        break;
      case ARMED:
        Serial.print("ARMED");
        break;
      case SILENT:
        Serial.print("SILENT");  
        break;  
    }

//print senosr warnings (all good, approaching, close, triggered, sounds),maybe states

    Serial.print(" |Dist: ");
    switch(distanceWarning){
      case DIST_SAFE:
        Serial.print("all good");
        break;
      case DIST_APPROACHING:
        Serial.print("Someone approaching");
        break;
      case DIST_CLOSE:
        Serial.print("Warning, someone very close");
        break;
      case DIST_TRIGGER:
        Serial.print("distance sensor trigger ");    
        break;  
    }
    Serial.print("|Sound: ");
    switch(soundWarning){
      case SOUND_SAFE:
        Serial.print("sound normal");
        break;
      case SOUND_ONE:
        Serial.print("One loud sound detected");
        break;
      case SOUND_TWO:
        Serial.print("Two loud sounds detected");    
        break;
      case SOUND_TRIGGER:
        Serial.print("sound Trigger, ");
        Serial.print(loudSoundCount);
        Serial.print(" loud sounds recieved");
        break;  

    }

    //print lock status
    Serial.print(" |Lock: ");
    if(sysLock == UNLOCKED){ Serial.print("unlocked");}
    else if(sysLock == LOCKED){ Serial.print("locked");}

    //print alarm status
    Serial.print(" |Alarm: ");
    switch(alarmState){
      case ALARM_WAIT:
        Serial.print("Wait");
        break;
      case ALARM_TRIGGERED:
        Serial.print("Triggered");  
        break;
      case ALARM_DISABLED:
        Serial.print("Disabled");
        break;  
    }
    Serial.println();

  }
  
}




//REQUIRED FUNCTIONS
void setup(){
 	Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(whitePin, OUTPUT); //silent
  pinMode(echoPin, INPUT);
  pinMode(A0, INPUT);

  Serial.println("SYSTEM STARTED");
  Serial.println("Enter 1111 to unlock");
}

void loop(){
  checkInput();
  checkUnlockTimeout();
  monitorSensors();
  updateLED();
  handleAlarm();
  printData();

}