XYTable Readme
Tyler Brauhn
Created 21NOV12

Updated 14FEB13

Function

This will provide a platform that can move in the x, y, and z directions controlled by an encoder for each direction. An LCD screen will display the target and current positions for each axis.
Pressing the "right" button moves the table to 
the target position. One of two encoder knobs 
controls each x and y axis target positions. 
Pressing the up and down buttons adjusts the z 
target position. The display can be "zeroed" by 
resetting the control board.

Purpose

This purpose of this project was not to create something extremely useful, but rather to learn about programming in Arduino, constructing circuits, and controlling stepper motors. The project is meant to be low-cost and provide a platform for testing out code while developing other projects, such as 3D printers and CNC machines. This manual control setup is mainly to verify that the hardware is working correctly. This is why each button press currently has no function, but are tested by displaying a character on the LCD screen.

Construction

The main hardware is constructed from two old flatbed scanners (purchased at Goodwill) torn down to the bottom half of the base and the scanning mechanisms that are driven by bipolar stepper motors. One is then bolted on top of the other at 90 degrees, creating two opposite axes of movement. A platform is then bolted onto the top scanner. The electronics in my setup consist of an Arduino Mega ADK, two Pololu A4988 stepper driver boards, two click encoders, a 16x2 LCD screen, and four push buttons. 

Configuration

The following variables will need to be configured based on each individual construction:
1. Pin assignments when using something other than the RepRap Gen7 1.4.1.
1. The number of steps per rotation for each motor
2. The maximum speed of each motor (currently restricted by slow code)
3. The total inches of movement of the platform per 100 steps of each motor

Libraries 

This code uses only one library: LiquidCrystal included with the Arduino IDE.