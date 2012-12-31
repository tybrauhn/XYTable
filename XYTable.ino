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
 void CheckButtons(void);
 void LCDUpdate(void);
 void ZeroPosition(void);
 void MoveToTarget(void);
 void StepSizeMeasurementRoutine(void);
 
 // Set up all pins
 int motor_X_step_pin = 20;
 int motor_X_dir_pin = 21;
 int motor_Y_step_pin = 4;
 int motor_Y_dir_pin = 5;
 int motor_Z_step_pin = 0;
 int motor_Z_dir_pin = 0; 
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
 int motorZSteps = 200; 
 int motorXSpeed = 600; //stepper motor speed in RPM
 int motorYSpeed = 800;
 int motorZSpeed = 500;
 float XInchesPer100Steps = .17; //inches of total table travel per 100 motor steps
 float YInchesPer100Steps = .03;
 float ZInchesPer100Steps = .03; 
 float XTotalInchesOfTravel = 12;
 float YTotalInchesOfTravel = 6;
 float ZTotalInchesOfTravel = 6;
 const int potReadingCount = 1000; //number of readings used in rolling average
 unsigned long lcdDelay = 500;
 int measurementSteps = 100;
 unsigned long debounceTime = 10;
 float dimensionResolution = .01;
 
 // initialize remaining variables
 float XDelayPerStep, YDelayPerStep, ZDelayPerStep, halfXDelayPerStep, halfYDelayPerStep, halfZDelayPerStep, maxDelayPerStep, maxHalfDelayPerStep;
 float XInchesPerStep, YInchesPerStep, ZInchesPerStep;
 unsigned long XTotalStepsOfTravel, YTotalStepsOfTravel, ZTotalStepsOfTravel;
 unsigned long XstepsPerAnalog, YstepsPerAnalog, ZstepsPerAnalog;
 long XTargetStep, YTargetStep;
 long ZTargetStep = 0;
 long XCurrentStep = 0;
 long YCurrentStep = 0;
 long ZCurrentStep = 0;
 float XCurrentInches, YCurrentInches, ZCurrentInches;
 float XTargetInches, YTargetInches, ZTargetInches;
 unsigned long lastLcdUpdateTime = 0;
 int lEncoderCount = 0;
 int rEncoderCount = 0;
 int upButtonStatus, downButtonStatus, leftButtonStatus, rightButtonStatus;
 int upButtonReading, downButtonReading, leftButtonReading, rightButtonReading;
 int lastUpButtonStatus = HIGH;
 int lastDownButtonStatus = HIGH;
 int lastLeftButtonStatus = HIGH;
 int lastRightButtonStatus = HIGH;
 int lastUpButtonReading = HIGH;
 int lastDownButtonReading = HIGH;
 int lastLeftButtonReading = HIGH;
 int lastRightButtonReading = HIGH;
 unsigned long lastUpButtonReadTime = 0;
 unsigned long lastDownButtonReadTime = 0;
 unsigned long lastLeftButtonReadTime = 0;
 unsigned long lastRightButtonReadTime = 0;
 int XStepFlag = 0;
 int YStepFlag = 0;
 int ZStepFlag = 0;
 unsigned long waitStart = 0;
 
 // create encoder objects
 Encoder lEncoder(left_encoder_pin_a,left_encoder_pin_b);
 Encoder rEncoder(right_encoder_pin_a,right_encoder_pin_b);
 
void setup() {
  
  // calculate delay in microseconds between steps for each motor
  XDelayPerStep = 60000000 * ( 1.0 / (motorXSteps * motorXSpeed));
  halfXDelayPerStep = XDelayPerStep / 2.0;
  YDelayPerStep = 60000000 * ( 1.0 / (motorYSteps * motorYSpeed));
  halfYDelayPerStep = YDelayPerStep / 2.0;
  ZDelayPerStep = 60000000 * ( 1.0 / (motorZSteps * motorZSpeed));
  halfZDelayPerStep = ZDelayPerStep / 2.0;
  
  // calculate max delay between axes
  maxDelayPerStep = max(max(XDelayPerStep,YDelayPerStep),ZDelayPerStep);
  maxHalfDelayPerStep = max(max(halfXDelayPerStep,halfYDelayPerStep),halfZDelayPerStep);
  
  // calculate inches per step
  XInchesPerStep = XInchesPer100Steps/100;
  YInchesPerStep = YInchesPer100Steps/100;
  ZInchesPerStep = ZInchesPer100Steps/100;
  
  // calculate total number of steps of travel
  XTotalStepsOfTravel = XTotalInchesOfTravel / XInchesPerStep;
  YTotalStepsOfTravel = YTotalInchesOfTravel / YInchesPerStep;
  ZTotalStepsOfTravel = ZTotalInchesOfTravel / ZInchesPerStep;
  
  // set up potentiometer operation
  XstepsPerAnalog = XTotalStepsOfTravel/1024;
  YstepsPerAnalog = YTotalStepsOfTravel/1024;
  ZstepsPerAnalog = ZTotalStepsOfTravel/1024;
      
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
  pinMode(motor_Z_step_pin, OUTPUT);
  digitalWrite(motor_Z_step_pin, LOW);
  pinMode(motor_X_dir_pin, OUTPUT);
  digitalWrite(motor_X_dir_pin, LOW);
  pinMode(motor_Y_dir_pin, OUTPUT);
  digitalWrite(motor_Y_dir_pin, LOW);
  pinMode(motor_Z_dir_pin, OUTPUT);
  digitalWrite(motor_Z_dir_pin, LOW);
  
  // set up the LCD's number of columns and rows
  lcd.begin(16,2);
  lcd.clear();
        
  }

void loop (){
  // check for button press with debounce delay
    CheckButtons();
    
  // step size measurement routine
    if(upButtonStatus==LOW && downButtonStatus==LOW && leftButtonStatus==LOW && rightButtonStatus==LOW){
      StepSizeMeasurementRoutine();
      CheckButtons();
    }
    
  // check for encoder rotation
    lEncoderCount = lEncoder.read();
    rEncoderCount = rEncoder.read();
    
  // set x and y target step (possibly done in read pot positions and or encoder check)
    XTargetStep = lEncoderCount;
    YTargetStep = rEncoderCount * 8; //"* 8" is rough estimation for .01 per click
    
  // set z target step based on up or down button press
    if(upButtonStatus==HIGH && lastUpButtonStatus==LOW && lastDownButtonStatus==HIGH && lastLeftButtonStatus==HIGH && lastRightButtonStatus==HIGH){
      ZTargetStep = ZTargetStep + (dimensionResolution * 100 / ZInchesPer100Steps);
    }
    if(downButtonStatus==HIGH && lastUpButtonStatus==HIGH && lastDownButtonStatus==LOW && lastLeftButtonStatus==HIGH && lastRightButtonStatus==HIGH){
      ZTargetStep = ZTargetStep - (dimensionResolution * 100 / ZInchesPer100Steps);
    }    
  
  // calculate status variables
    // calculate position in inches
      XTargetInches = XTargetStep * XInchesPerStep;
      YTargetInches = YTargetStep * YInchesPerStep;
      ZTargetInches = ZTargetStep * ZInchesPerStep;
      XCurrentInches = XCurrentStep * XInchesPerStep;
      YCurrentInches = YCurrentStep * YInchesPerStep;
      ZCurrentInches = ZCurrentStep * ZInchesPerStep;
    
  // check for time and update lcd display after next lcd update time has passed
    if(millis() >= lastLcdUpdateTime + lcdDelay){
     LCDUpdate();
     lastLcdUpdateTime = millis();
    }
    
  // check for right button press to move to target position
    if(upButtonStatus==HIGH && downButtonStatus==HIGH && leftButtonStatus==HIGH && rightButtonStatus==LOW){
      MoveToTarget();
    }
    
  // check for left button press to zero current and target positions
    if(upButtonStatus==HIGH && downButtonStatus==HIGH && leftButtonStatus==LOW && rightButtonStatus==HIGH){
      ZeroPosition();
    }
    
  // store last button status
    lastUpButtonStatus = upButtonStatus;
    lastDownButtonStatus = downButtonStatus;
    lastLeftButtonStatus = leftButtonStatus;
    lastRightButtonStatus = rightButtonStatus;
    
}

void CheckButtons(){
    upButtonReading = digitalRead(up_pin);
    if (upButtonReading != lastUpButtonReading){
      lastUpButtonReadTime = millis(); 
    }
    if((millis() - lastUpButtonReadTime) > debounceTime){
      upButtonStatus = upButtonReading;
    }
    lastUpButtonReading = upButtonReading;
    
    downButtonReading = digitalRead(down_pin);
    if (downButtonReading != lastDownButtonReading){
      lastDownButtonReadTime = millis(); 
    }
    if((millis() - lastDownButtonReadTime) > debounceTime){
      downButtonStatus = downButtonReading;
    }
    lastDownButtonReading = downButtonReading;
    
    leftButtonReading = digitalRead(left_pin);
    if (leftButtonReading != lastLeftButtonReading){
      lastLeftButtonReadTime = millis(); 
    }
    if((millis() - lastLeftButtonReadTime) > debounceTime){
      leftButtonStatus = leftButtonReading;
    }
    lastLeftButtonReading = leftButtonReading;
    
    rightButtonReading = digitalRead(right_pin);
    if (rightButtonReading != lastRightButtonReading){
      lastRightButtonReadTime = millis(); 
    }
    if((millis() - lastRightButtonReadTime) > debounceTime){
      rightButtonStatus = rightButtonReading;
    }
    lastRightButtonReading = rightButtonReading;
}

void LCDUpdate(){
  lcd.clear();
  lcd.print(XCurrentInches);
  lcd.print(",");
  lcd.print(YCurrentInches);
  lcd.print(",");
  lcd.print(ZCurrentInches);
  lcd.setCursor(0,1);
  lcd.print(XTargetInches);
  lcd.print(",");
  lcd.print(YTargetInches);
  lcd.print(",");
  lcd.print(ZTargetInches);
}
  
void ZeroPosition(){
  lEncoder.write(0);
  rEncoder.write(0);
  ZTargetStep = 0;
  XCurrentStep = 0;
  YCurrentStep = 0;
  ZCurrentStep = 0;
}

void MoveToTarget(){
  // indicate movement on lcd screen
    lcd.clear();
    lcd.print("Moving...");
  
  while((XCurrentStep != XTargetStep) | (YCurrentStep != YTargetStep) | (ZCurrentStep != ZTargetStep)){
  // check for current X step versus target X step
    if (XCurrentStep < XTargetStep){
      XStepFlag = 1;
    // X motor step pin to high, direction pin to high
      digitalWrite(motor_X_dir_pin,HIGH);
      delayMicroseconds(1);
      digitalWrite(motor_X_step_pin,HIGH);
    // increment current X step + 1
      XCurrentStep++;
    }
    else if (XCurrentStep > XTargetStep){
      XStepFlag = 1;
    // X motor step pin to high, direction pin to low 
      digitalWrite(motor_X_dir_pin,LOW);
      delayMicroseconds(1);
      digitalWrite(motor_X_step_pin,HIGH); 
    // increment current X step - 1
      XCurrentStep--;
    }
    
  // check for current Y step versus target Y step
    if (YCurrentStep < YTargetStep){
      YStepFlag = 1;
    // Y motor step pin to high, direction pin to high
      digitalWrite(motor_Y_dir_pin,HIGH);
      delayMicroseconds(1);
      digitalWrite(motor_Y_step_pin,HIGH);
    // increment current Y step + 1
      YCurrentStep++;
    }
    else if (YCurrentStep > YTargetStep){
      YStepFlag = 1;
    // Y motor step pin to high, direction pin to low 
      digitalWrite(motor_Y_dir_pin,LOW);
      delayMicroseconds(1);
      digitalWrite(motor_Y_step_pin,HIGH);
    // increment current Y step - 1
      YCurrentStep--;
    }
    
    // check for current Z step versus target Z step
    if (ZCurrentStep < ZTargetStep){
      ZStepFlag = 1;
    // Z motor step pin to high, direction pin to high
      digitalWrite(motor_Z_dir_pin,HIGH);
      delayMicroseconds(1);
      digitalWrite(motor_Z_step_pin,HIGH);
    // increment current Z step + 1
      ZCurrentStep++;
    }
    else if (ZCurrentStep > ZTargetStep){
      ZStepFlag = 1;
    // Z motor step pin to high, direction pin to low 
      digitalWrite(motor_Z_dir_pin,LOW);
      delayMicroseconds(1);
      digitalWrite(motor_Z_step_pin,HIGH);
    // increment current Z step - 1
      ZCurrentStep--;
    }
    
    if((XStepFlag + YStepFlag + ZStepFlag) >= 2){
    // delay a short time
      delayMicroseconds(maxHalfDelayPerStep);
    // X and Y motor step pins to low
      digitalWrite(motor_X_step_pin, LOW);
      digitalWrite(motor_Y_step_pin, LOW);
      digitalWrite(motor_Z_step_pin, LOW);
    // delay another short time
      delayMicroseconds(maxHalfDelayPerStep);
      XStepFlag = 0;
      YStepFlag = 0;
      ZStepFlag = 0;
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
        ZStepFlag = 0;
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
        ZStepFlag = 0;
    }
    else if (ZStepFlag == 1){
      // delay a short time
        delayMicroseconds(halfZDelayPerStep);
      // X motor step pin to low
        digitalWrite(motor_Z_step_pin, LOW);
      // delay another short time
        delayMicroseconds(halfZDelayPerStep);
        XStepFlag = 0;
        YStepFlag = 0;
        ZStepFlag = 0;
    }
  }
}

void StepSizeMeasurementRoutine(){
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
     
     for(int countdown = 5; countdown > 0; countdown--){
      lcd.clear();
      lcd.print(measurementSteps);
      lcd.print(" Z Steps In: ");
      lcd.setCursor(0,1);
      lcd.print(countdown);
      lcd.print(" sec");
      delay(1000);
     } 
     digitalWrite(motor_Z_dir_pin,HIGH);
     delay(5);
     for(int ZMeasurementStep = measurementSteps; ZMeasurementStep > 0; ZMeasurementStep--){
       lcd.clear();
       lcd.print(ZMeasurementStep);
       digitalWrite(motor_Z_step_pin,HIGH);
       delayMicroseconds(halfZDelayPerStep);
       digitalWrite(motor_Z_step_pin,LOW);
       delayMicroseconds(halfZDelayPerStep);
     }
     lcd.clear();
     lcd.print("Z Movement Complete");
     delay(2000);
}

/* 
   Modified 29DEC12
   by Tyler Brauhn
   * Add variables and routines for z-axis
   * Moved code for button check from "loop()" to "CheckButtons()"
   * Moved code to run motors from "loop()" to "MoveToTarget()"
   * Moved code for step size measurment routine from "loop()" to "StepSizeMeasurementRoutine()"
   * Change LCD display to show target position and current position without button status
   * Add functionality to buttons: left zero's position, right starts movement, up and down move target z position
   * Add debounce to button updates
 */
