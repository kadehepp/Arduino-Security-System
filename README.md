# Arduino-Security-System
A small state based security system that uses a sound and ultrasonic(distance) sensor to detect intrusions, trigger alarms, and provide visual feedback

Goals:
1. real time user interaction and feedback with system
2. embedded system state management
3. non blocking timing logic

Hardware Requirements:
1. Arduino Uno + usb cable
2. Ultrasonic sensor module
3. Sound Sensor module
4. 5 LED's (Red, yellow, green, white, blue)
5. Resistors(330 ohm) for LED's
6. Breadboard
7. Jumperwires

Installation Instructions:
1. Copy Repository
2. Download Arduino IDE -> https://www.arduino.cc/en/software/ 
2. Load Sketch into Arduino IDE
3. Wire sensors and LED's to respective pins labeled in the sketch (pin 12 = trig, 13 = echo, A0 = sound, 2-6 = LED)
4. Select board and port to upload

Usage Instructions:
System will start locked and off, to change mode you must unlock the system and use the following pins to navigate modes:
1. Unlock = 1111    
2. Lock = 2222

The following modes are available if system is unlocked:

3. Armed = 3333
4. Silent = 4444
5. Off = 5555

The following is only available if the system has been triggered:

6. Disarm = 9999

Visual System Behavior:

Green LED = area safe

Yellow LED = object approaching

Red LED = object close
White solid = silent mode
Blue LED = Armed mode
flashing = alarm triggered
flashinbg white = sound warnings

Features:
1. Multi sensor intrusion detection
2. non-blocking timing
3. automatic saftey reset
4. Armed, Silent, and Off modes
5. Pin based system control

Technology Stack:
1. Arduino C/C++
2. Arduino IDE
3. Ultrasonic sensor
4. Sound sensor
5. Serial communication
6. Finite State Machine

Authors:
Kade Heppner - kadeheppner@gmail.com - www.linkedin.com/in/kadeheppner
(has not confirmed I can add their credentials) - 
     
  
