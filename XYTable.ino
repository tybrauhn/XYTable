/* 
   XYTable
   
   This is a sketch designed to allow manual operation of an 
   X-Y table built with old scanners. The controls consist of
   two click encoders, 4 buttons, and an LCD screen. 
   Pin schematic is found below in setup.
   
   Created 21 November 2012
   by Tyler Brauhn
 */
 
 #include <LiquidCrystal.h>
 #include <Encoder.h>
 
 // function prototypes
 void lcdUpdate(void);
 
 // Set up all pins
 int motor_X_step_pin = 20;
 int motor_X_dir_pin = 21;
 int motor_Y_step_pin = 4;
 int motor_Y_dir_pin = 5;
 int up_pin = 7;
 int down_pin = 9;
 int left_pin = 10;
 int right_pin = 12;
 LiquidCrystal lcd(19,18,17,16,15,14);
 int left_encoder_pin_a = 2; //A0 //should be interrupt
 int left_encoder_pin_b = 55; //A1
 int right_encoder_pin_a = 3; //A2 //should be interrupt
 int right_encoder_pin_b = 57; //A3
 
 // Set up global variables
 int motorXSteps = 50; //steps per revolution of the motor
 int motorYSteps = 200;
 int motorXSpeed = 500; //stepper motor speed in RPM
 int motorYSpeed = 500;
 float XInchesPer100Steps = .17; //inches of total table travel per 100 motor steps
 float YInchesPer100Steps = .03;
 float XTotalInchesOfTravel = 12;
 float YTotalInchesOfTravel = 6;
 const int potReadingCount = 1000; //number of readings used in rolling average
 unsigned long lcdDelay = 500;
 int measurementSteps = 100;
 
 // initialize remaining variables
 unsigned long XDelayPerStep, YDelayPerStep, halfXDelayPerStep, halfYDelayPerStep;
 float XInchesPerStep, YInchesPerStep;
 unsigned long XTotalStepsOfTravel, YTotalStepsOfTravel;
 unsigned long XstepsPerAnalog, YstepsPerAnalog;
 long XTargetStep, YTargetStep;
 long XCurrentStep = 0;
 long YCurrentStep = 0;
 float XCurrentInches, YCurrentInches;
 unsigned long lastLcdUpdateTime = 0;
 int lEncoderCount = 0;
 int rEncoderCount = 0;
 int upButtonStatus, downButtonStatus, leftButtonStatus, rightButtonStatus;
 int XStepFlag = 0;
 int YStepFlag = 0;
 
 // create encoder objects
 Encoder lEncoder(left_encoder_pin_a,left_encoder_pin_b);
 Encoder rEncoder(right_encoder_pin_a,right_encoder_pin_b);
 
void setup() {
  
  // calculate delay in microseconds between steps for each motor
  XDelayPerStep = 60000000 * ( 1 / (motorXSteps * motorXSpeed));
  halfXDelayPerStep = XDelayPerStep / 2;
  YDelayPerStep = 60000000 * ( 1 / (motorYSteps * motorYSpeed));
  halfYDelayPerStep = YDelayPerStep / 2;
  
  // calculate inches per step
  XInchesPerStep = XInchesPer100Steps/100;
  YInchesPerStep = YInchesPer100Steps/100;
  
  // calculate total number of steps of travel
  XTotalStepsOfTravel = XTotalInchesOfTravel / XInchesPerStep;
  YTotalStepsOfTravel = YTotalInchesOfTravel / YInchesPerStep;
  
  // set up potentiometer operation
  XstepsPerAnalog = XTotalStepsOfTravel/1024;
  YstepsPerAnalog = YTotalStepsOfTravel/1024;
      
  // set up button pin modes
  pinMode(up_pin, INPUT);
  digitalWrite(up_pin, HIGH);
  pinMode(down_pin, INPUT);
  digitalWrite(down_pin, HIGH);
  pinMode(left_pin, INPUT);
  digitalWrite(left_pin, HIGH);
  pinMode(right_pin, INPUT);
  digitalWrite(right_pin, HIGH);
  
  pinMode(motor_X_step_pin, OUTPUT);
  digitalWrite(motor_X_step_pin, LOW);
  pinMode(motor_Y_step_pin, OUTPUT);
  digitalWrite(motor_Y_step_pin, LOW);
  pinMode(motor_X_dir_pin, OUTPUT);
  digitalWrite(motor_X_dir_pin, LOW);
  pinMode(motor_Y_dir_pin, OUTPUT);
  digitalWrite(motor_Y_dir_pin, LOW);
  
  // set up the LCD's number of columns and rows
  lcd.begin(16,2);
  lcd.clear();
        
  }

void loop (){
  // check for button press
    upButtonStatus = digitalRead(up_pin);
    downButtonStatus = digitalRead(down_pin);
    leftButtonStatus = digitalRead(left_pin);
    rightButtonStatus = digitalRead(right_pin);
    
  // step size measurement routine
    if(upButtonStatus==LOW && downButtonStatus==LOW && leftButtonStatus==LOW && rightButtonStatus==LOW){
     for(int countdown = 5; countdown > 0; countdown--){
      lcd.clear();
      lcd.print(measurementSteps);
      lcd.print(" X Steps In: ");
      lcd.setCursor(0,1);
      lcd.print(countdown);
      lcd.print(" sec");
      delay(1000);
     } 
     digitalWrite(motor_X_dir_pin,HIGH);
     delay(5);
     for(int XMeasurementStep = measurementSteps; XMeasurementStep > 0; XMeasurementStep--){
       lcd.clear();
       lcd.print(XMeasurementStep);
       digitalWrite(motor_X_step_pin,HIGH);
       delayMicroseconds(halfXDelayPerStep);
       digitalWrite(motor_X_step_pin,LOW);
       delayMicroseconds(halfXDelayPerStep);
     }
     lcd.clear();
     lcd.print("X Movement Complete");
     delay(2000);
     for(int countdown = 5; countdown > 0; countdown--){
      lcd.clear();
      lcd.print(measurementSteps);
      lcd.print(" Y Steps In: ");
      lcd.setCursor(0,1);
      lcd.print(countdown);
      lcd.print(" sec");
      delay(1000);
     } 
     digitalWrite(motor_Y_dir_pin,HIGH);
     delay(5);
     for(int YMeasurementStep = measurementSteps; YMeasurementStep > 0; YMeasurementStep--){
       lcd.clear();
       lcd.print(YMeasurementStep);
       digitalWrite(motor_Y_step_pin,HIGH);
       delayMicroseconds(halfYDelayPerStep);
       digitalWrite(motor_Y_step_pin,LOW);
       delayMicroseconds(halfYDelayPerStep);
     }
     lcd.clear();
     lcd.print("Y Movement Complete");
     delay(2000);
     upButtonStatus = digitalRead(up_pin);
     downButtonStatus = digitalRead(down_pin);
     leftButtonStatus = digitalRead(left_pin);
     rightButtonStatus = digitalRead(right_pin);
    }
    
  // check for encoder rotation
    lEncoderCount = lEncoder.read();
    rEncoderCount = rEncoder.read();
    
  // set target step (possibly done in read pot positions and or encoder check)
    XTargetStep = rEncoderCount;
    YTargetStep = lEncoderCount; 
    
  // check for current X step versus target X step
    if (XCurrentStep < XTargetStep){
      XStepFlag = 1;
    // X motor step pin to high, direction pin to high
      digitalWrite(motor_X_dir_pin,HIGH);
      delayMicroseconds(5);
      digitalWrite(motor_X_step_pin,HIGH);
    // increment current X step + 1
      XCurrentStep++;
    }
    else if (XCurrentStep > XTargetStep){
      XStepFlag = 1;
    // X motor step pin to high, direction pin to low 
      digitalWrite(motor_X_dir_pin,LOW);
      delayMicroseconds(5);
      digitalWrite(motor_X_step_pin,HIGH); 
    // increment current X step - 1
      XCurrentStep--;
    }
    
  // check for current Y step versus target Y step
    if (YCurrentStep < YTargetStep){
      YStepFlag = 1;
    // Y motor step pin to high, direction pin to high
      digitalWrite(motor_Y_dir_pin,HIGH);
      delayMicroseconds(5);
      digitalWrite(motor_Y_step_pin,HIGH);
    // increment current Y step + 1
      YCurrentStep++;
    }
    else if (YCurrentStep > YTargetStep){
      YStepFlag = 1;
    // Y motor step pin to high, direction pin to low 
      digitalWrite(motor_Y_dir_pin,LOW);
      delayMicroseconds(5);
      digitalWrite(motor_Y_step_pin,HIGH);
    // increment current Y step - 1
      YCurrentStep--;
    }
    
    if((XStepFlag + YStepFlag) == 2){
    // delay a short time
      delayMicroseconds(max(halfXDelayPerStep,halfYDelayPerStep));
    // X and Y motor step pins to low
      digitalWrite(motor_X_step_pin, LOW);
      digitalWrite(motor_Y_step_pin, LOW);
    // delay another short time
      delayMicroseconds(max(halfXDelayPerStep,halfYDelayPerStep));
      XStepFlag = 0;
      YStepFlag = 0;
    }
    else if (XStepFlag == 1){
      // delay a short time
        delayMicroseconds(halfXDelayPerStep);
      // X motor step pin to low
        digitalWrite(motor_X_step_pin, LOW);
      // delay another short time
        delayMicroseconds(halfXDelayPerStep);
        XStepFlag = 0;
        YStepFlag = 0;
    }
    else if (YStepFlag == 1){
      // delay a short time
        delayMicroseconds(halfYDelayPerStep);
      // X motor step pin to low
        digitalWrite(motor_Y_step_pin, LOW);
      // delay another short time
        delayMicroseconds(halfYDelayPerStep);
        XStepFlag = 0;
        YStepFlag = 0;
    }
  
  // calculate status variables
    // calculate position in inches
      XCurrentInches = XTargetStep * XInchesPerStep;
      YCurrentInches = YTargetStep * YInchesPerStep;
    
  // check for time and update lcd display after next lcd update time has passed
    if(millis() >= lastLcdUpdateTime + lcdDelay){
     lcdUpdate(upButtonStatus,downButtonStatus,leftButtonStatus,rightButtonStatus,XCurrentStep,YCurrentStep);
     lastLcdUpdateTime = millis();
    }
}

void lcdUpdate(int _upButtonStatus,int _downButtonStatus,int _leftButtonStatus,int _rightButtonStatus,long _XCurrentStep,long _YCurrentStep){
  lcd.clear();
  lcd.print("-");
  if(_upButtonStatus==LOW){
    lcd.print("U ");
  }
  else {
    lcd.print(" "); 
  }
  if(_downButtonStatus==LOW){
    lcd.print("D ");
  }
  else {
    lcd.print(" "); 
  }
  if(_leftButtonStatus==LOW){
    lcd.print("L ");
  }
  else {
    lcd.print(" "); 
  }
  if(_rightButtonStatus==LOW){
    lcd.print("R");
  }
  else {
    lcd.print(" "); 
  }
  lcd.print("-");
  
  lcd.setCursor(0,1);
  lcd.print("X:");
  lcd.print(XCurrentInches);
  lcd.print("  Y:");
  lcd.print(YCurrentInches);
}

