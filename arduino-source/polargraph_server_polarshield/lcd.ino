/**
*  Polargraph Server for ATMEGA1280+ 
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_polarshield

Specific features for Polarshield / arduino mega.
LCD (and touch screen).

All the many routines for drawing the buttons and menus and controls that appear
on the polargraphSD's touchscreen, AND the interrupt-driven routines to handle
input through the screen.

There is a lot of this, but it's pretty samey.
*/


/**  This is the method that is called by an interrupt when the touchscreen 
is touched. It sets a parameter (displayTouched) to true, but does not act 
directly on the touch.
*/

void lcd_touchInput()
{
  // don't trigger if it's already in processing
  if (!displayTouched)
  {
    while (touch.dataAvailable()) {
      touch.read();
      long x = touch.getX();
      long y = touch.getY();
      if ((x != -1) and (y != -1)) {
        Serial.print("touch ok: ");
        Serial.print(touchX);
        Serial.print(",");
        Serial.println(touchY);
        touchX = x;
        touchY = y;
        displayTouched = true;
        lastInteractionTime = millis();
        break;
      }
    }
  }
  else {
    Serial.println("Already touched.");
  }
}

/** 
*   This method should be regularly polled to see if the machine has
*   been touched, and to act on the touches.  It checks the value of 
*   displayTouched boolean, and if it is true, then attempts to 
*   process the touch command.
*/
void lcd_checkForInput()
{
  lcd_touchInput();
  
  if (displayTouched)
  {
    Serial.print("2: ");
    Serial.print(touchX);
    Serial.print(",");
    Serial.println(touchY);
    
    Serial.println("Check for input");
    lastOperationTime = millis();
    if (screenState == SCREEN_STATE_POWER_SAVE)
    {
      delay(20);
      screenState = SCREEN_STATE_NORMAL;
      lcd_drawButtons();
    }
    else
    {
      Serial.println("Inputted!!");
      delay(20);
      lcd_processTouchCommand();
    }
    Serial.print("DONE.");
    displayTouched = false;
  }
  else
  {
//    Serial.print("2a: ");
//    Serial.print(touchX);
//    Serial.print(",");
//    Serial.println(touchY);

//    Serial.print("Screen state: ");
//    Serial.print(screenState);
//    Serial.print(", lastInteractionTime: ");
//    Serial.print(lastInteractionTime);
//    Serial.print(" plus idle: ");
//    Serial.print(lastInteractionTime + screenSaveIdleTime);
//    Serial.print(", millis: ");
//    Serial.println(millis());
    
    if (screenState == SCREEN_STATE_NORMAL
    && (millis() > (lastInteractionTime + screenSaveIdleTime)))
    {
      // put it to sleep
      screenState = SCREEN_STATE_POWER_SAVE;
      lcd.clrScr();
    }
    else if (screenState == SCREEN_STATE_POWER_SAVE
      && (millis() < lastInteractionTime + screenSaveIdleTime))
    {
      delay(20);
      screenState = SCREEN_STATE_NORMAL;
      lcd_drawButtons();
    }
  }
}

void lcd_updateDisplay()
{
}


/* =============================================================================
   This is the code for controlling the LCD, menus and the hardware controls
   There's a lot of it, but it's mostly all the same.
   =============================================================================*/
int buttonCoords[12][2];
byte  gap = (LCD_TYPE == ITDB24E_8 || LCD_TYPE == TFT01_24_8) ? 10 : 10;
byte  buttonSize = 60;
byte  grooveSize = 36;
static int screenWidth = (LCD_TYPE == ITDB24E_8 || LCD_TYPE == TFT01_24_8) ? 320 : 220;
static int screenHeight = (LCD_TYPE == ITDB24E_8 || LCD_TYPE == TFT01_24_8) ? 240 : 176;
static int centreYPosition = (LCD_TYPE == ITDB24E_8 || LCD_TYPE == TFT01_24_8) ? 112 : 80;

const byte MENU_INITIAL = 1;
const byte MENU_RUNNING = 2;
const byte MENU_CHOOSE_FILE = 3;
const byte MENU_ADJUST_PENSIZE = 4;
const byte MENU_ADJUST_POSITION = 5;
const byte MENU_ADJUST_SPEED = 6;
const byte MENU_SETTINGS = 7;
const byte MENU_SETTINGS_2 = 8;
const byte MENU_ADJUST_PENLIFT = 9;
volatile byte currentMenu = MENU_INITIAL;

byte numberOfMenuButtons = 3;
const byte BUTTON_SET_HOME = 1;
const byte BUTTON_DRAW_FROM_SD = 2;
const byte BUTTON_MORE_RIGHT = 3;
const byte BUTTON_PAUSE_RUNNING = 4;
const byte BUTTON_RESUME_RUNNING = 5;
const byte BUTTON_RESET = 6;
const byte BUTTON_PEN_UP = 7;
const byte BUTTON_PEN_DOWN = 8;
const byte BUTTON_INC_SPEED = 9;
const byte BUTTON_DEC_SPEED = 10;
const byte BUTTON_NEXT_FILE = 11;
const byte BUTTON_PREV_FILE = 12;
const byte BUTTON_MAIN_MENU = 13;
const byte BUTTON_OK = 14;
const byte BUTTON_CANCEL_FILE = 15;
const byte BUTTON_DRAW_THIS_FILE = 16;
const byte BUTTON_INC_ACCEL = 18;
const byte BUTTON_DEC_ACCEL = 19;
const byte BUTTON_DONE = 20;
const byte BUTTON_MOVE_INC_A = 21;
const byte BUTTON_MOVE_DEC_A = 22;
const byte BUTTON_MOVE_INC_B = 23;
const byte BUTTON_MOVE_DEC_B = 24;
const byte BUTTON_INC_PENSIZE = 25;
const byte BUTTON_DEC_PENSIZE = 26;
const byte BUTTON_INC_PENSIZE_INC = 27;
const byte BUTTON_DEC_PENSIZE_INC = 28;
const byte BUTTON_ADJUST_SPEED_MENU = 29;
const byte BUTTON_ADJUST_PENSIZE_MENU = 30;
const byte BUTTON_ADJUST_POSITION_MENU = 31;
const byte BUTTON_POWER_ON = 32;
const byte BUTTON_POWER_OFF = 33;
const byte BUTTON_STOP_FILE = 34;
const byte BUTTON_SETTINGS_MENU = 35;
const byte BUTTON_CALIBRATE = 36;
const byte BUTTON_TOGGLE_ECHO = 37;
const byte BUTTON_RESET_SD = 38;
const byte BUTTON_SETTINGS_MENU_2 = 39;

const byte BUTTON_INC_PENLIFT_UP = 40;
const byte BUTTON_DEC_PENLIFT_UP = 41;
const byte BUTTON_INC_PENLIFT_DOWN = 42;
const byte BUTTON_DEC_PENLIFT_DOWN = 43;
const byte BUTTON_ADJUST_PENLIFT = 44;
const byte BUTTON_PENLIFT_SAVE_TO_EEPROM = 45;

byte colorLightRed = random(50, 255);
byte colorLightGreen = random(50, 255);
byte colorLightBlue = random(50, 255);

byte colorDarkRed = 5;
byte colorDarkGreen = 5;
byte colorDarkBlue = 5;

byte colorBrightRed = 255;
byte colorBrightGreen = 255;
byte colorBrightBlue = 255;

void lcd_processTouchCommand()
{
  Serial.println("process touch.");
  // get control that is under the
        Serial.print("3: ");
        Serial.print(touchX);
        Serial.print(",");
        Serial.println(touchY);  
  byte buttonNumber = lcd_getButtonNumber(touchX, touchY);
  lcd_outlinePressedButton(buttonNumber,colorBrightRed,colorBrightGreen,colorBrightBlue);

  byte pressedButton = lcd_getWhichButtonPressed(buttonNumber, currentMenu);
  Serial.print("button:");
  Serial.println(pressedButton);
  switch (pressedButton)
  {
    case BUTTON_POWER_ON:
      lcd_runStartScript();
      lcd_drawButton(BUTTON_POWER_OFF);
      break;
    case BUTTON_POWER_OFF:
      lcd_runEndScript();
      lcd_drawButton(BUTTON_POWER_ON);
      break;
    case BUTTON_DRAW_FROM_SD:
      lcd_drawStoreContentsMenu();
      break;
//    case BUTTON_RESET_SD:
//      root.close();
//      sd_initSD();
//      lcd_drawStoreContentsMenu();
//      break;
    case BUTTON_MORE_RIGHT:
      break;
    case BUTTON_PAUSE_RUNNING:
      currentlyRunning = false;
      lcd_drawButton(BUTTON_RESUME_RUNNING);
      break;
    case BUTTON_RESUME_RUNNING:
      currentlyRunning = true;
      lcd_drawButton(BUTTON_PAUSE_RUNNING);
      break;
    case BUTTON_RESET:
      break;
    case BUTTON_PEN_UP:
      inNoOfParams=0;
      penlift_penUp();
      lcd_drawButton(BUTTON_PEN_DOWN);
      break;
    case BUTTON_PEN_DOWN:
      inNoOfParams=0;
      penlift_penDown();
      lcd_drawButton(BUTTON_PEN_UP);
      break;
    case BUTTON_INC_SPEED:
      exec_setMotorSpeed(currentMaxSpeed + speedChangeIncrement);
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, currentMaxSpeed);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_SPEED:
      exec_setMotorSpeed(currentMaxSpeed - speedChangeIncrement);
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, currentMaxSpeed);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_INC_ACCEL:
      exec_setMotorAcceleration(currentAcceleration + accelChangeIncrement);
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, currentAcceleration);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_ACCEL:
      exec_setMotorAcceleration(currentAcceleration - accelChangeIncrement);
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, currentAcceleration);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_INC_PENSIZE:
      penWidth = penWidth+penWidthIncrement;
      lcd_drawFloatWithBackground(buttonCoords[10][0], centreYPosition, penWidth);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_PENSIZE:
      penWidth = penWidth-penWidthIncrement;
      if (penWidth < penWidthIncrement)
        penWidth = penWidthIncrement;
      lcd_drawFloatWithBackground(buttonCoords[10][0], centreYPosition, penWidth);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_INC_PENSIZE_INC:
      penWidthIncrement += 0.005;
      lcd_drawFloatWithBackground(buttonCoords[8][0], centreYPosition, penWidthIncrement);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_PENSIZE_INC:
      penWidthIncrement -= 0.005;
      if (penWidthIncrement < 0.005)
        penWidthIncrement = 0.005;
      lcd_drawFloatWithBackground(buttonCoords[8][0], centreYPosition, penWidthIncrement);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_TOGGLE_ECHO:
      echoingStoredCommands = !echoingStoredCommands;
      delay(500);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DRAW_THIS_FILE:
      if (commandFilename != "None" && commandFilename != "" && commandFilename != "            ")
      {
        Serial.print("Drawing this file: ");
        Serial.println(commandFilename);
        currentlyDrawingFromFile = true;
        lcd_drawButton(BUTTON_STOP_FILE);
        displayTouched = false;      
        impl_exec_execFromStore(commandFilename);
        lcd_drawButton(pressedButton);
      }
      else
      {
        lcd_drawButton(pressedButton);
      }
      break;
    case BUTTON_STOP_FILE:
      Serial.print("Cancelling drawing this file: ");
      Serial.println(commandFilename);
      currentlyDrawingFromFile = false;
      lcd_drawButton(BUTTON_DRAW_THIS_FILE);
      break;
    case BUTTON_NEXT_FILE:
      // load the next filename
      Serial.println("looking up next filename.");
      commandFilename = lcd_loadFilename(commandFilename, 1);
      lcd_drawCurrentSelectedFilename();
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_PREV_FILE:
      // load the next filename
      Serial.println("looking up previous filename.");
      commandFilename = lcd_loadFilename(commandFilename, -1);
      lcd_drawCurrentSelectedFilename();
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_CANCEL_FILE:
      // return to main menu
      commandFilename = "";
      currentMenu = MENU_INITIAL;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_ADJUST_PENSIZE_MENU:
      currentMenu = MENU_ADJUST_PENSIZE;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_ADJUST_SPEED_MENU:
      currentMenu = MENU_ADJUST_SPEED;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_ADJUST_POSITION_MENU:
      currentMenu = MENU_ADJUST_POSITION;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_SETTINGS_MENU:
      currentMenu = MENU_SETTINGS;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_SETTINGS_MENU_2:
      currentMenu = MENU_SETTINGS_2;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_DONE:
      currentMenu = MENU_INITIAL;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_MOVE_INC_A:
      motorA.move(moveIncrement);
      while (motorA.distanceToGo() != 0)
        motorA.run();
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, motorA.currentPosition());
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_MOVE_DEC_A:
      motorA.move(0-moveIncrement);
      while (motorA.distanceToGo() != 0)
        motorA.run();
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, motorA.currentPosition());
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_MOVE_INC_B:
      motorB.move(moveIncrement);
      while (motorB.distanceToGo() != 0)
        motorB.run();
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, motorB.currentPosition());
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_MOVE_DEC_B:
      motorB.move(0-moveIncrement);
      while (motorB.distanceToGo() != 0)
        motorB.run();
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, motorB.currentPosition());
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_CALIBRATE:
      calibrate_doCalibration();
      lcd_drawButton(pressedButton);
      break;
      
    case BUTTON_ADJUST_PENLIFT:
      currentMenu = MENU_ADJUST_PENLIFT;
      lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
      lcd.clrScr();
      lcd_drawButtons();
      break;
    case BUTTON_INC_PENLIFT_UP:
      if (upPosition < 300) {
        upPosition += 1;
        if (isPenUp)
          penlift_movePen(upPosition-15, upPosition, penLiftSpeed);
        else
          penlift_movePen(downPosition, upPosition, penLiftSpeed);
        isPenUp = true;
      }
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, upPosition);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_PENLIFT_UP:
      if (upPosition > 0) {
        upPosition -= 1;
        if (isPenUp)
          penlift_movePen(upPosition+15, upPosition, penLiftSpeed);
        else
          penlift_movePen(downPosition, upPosition, penLiftSpeed);
        isPenUp = true;
      }
      lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, upPosition);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_INC_PENLIFT_DOWN:
      if (downPosition < 300) {
        downPosition += 1;
        if (isPenUp)
          penlift_movePen(upPosition, downPosition, penLiftSpeed);
        else
          penlift_movePen(downPosition-15, downPosition, penLiftSpeed);
        isPenUp = false;
      }
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, downPosition);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_DEC_PENLIFT_DOWN:
      if (downPosition > 0) {
        downPosition -= 1;
        if (isPenUp)
          penlift_movePen(upPosition, downPosition, penLiftSpeed);
        else
          penlift_movePen(downPosition+15, downPosition, penLiftSpeed);
        isPenUp = false;
      }
      lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, downPosition);
      lcd_drawButton(pressedButton);
      break;
    case BUTTON_PENLIFT_SAVE_TO_EEPROM:
      Serial.println("HJey");
      EEPROM_writeAnything(EEPROM_PENLIFT_DOWN, downPosition);
      EEPROM_writeAnything(EEPROM_PENLIFT_UP, upPosition);
      eeprom_loadPenLiftRange();
      delay(1000);
      lcd_drawButton(pressedButton);
      break;
  }
}


void lcd_drawNumberWithBackground(int x, int y, long value)
{
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.fillRect(x, y, x+buttonSize, y+20);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
  lcd.setBackColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.printNumI(value, x, y);
}
void lcd_drawFloatWithBackground(int x, int y, float value)
{
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.fillRect(x, y, x+buttonSize, y+20);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
  lcd.setBackColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.printNumF(value, 2, x, y);
}

void lcd_runStartScript()
{
  engageMotors();
  motorA.setCurrentPosition(startLengthStepsA);
  motorB.setCurrentPosition(startLengthStepsB);  
  Serial.println("finished start.");
}
void lcd_runEndScript()
{
  penlift_penUp();
  releaseMotors();
  isCalibrated = false;
}
void lcd_setCurrentMenu(byte menu)
{
  currentMenu = menu;
}

void lcd_displayFirstMenu()
{
  lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
  lcd.clrScr();
  lcd_setCurrentMenu(MENU_INITIAL);
  lcd_drawButtons();
}

void lcd_drawStoreContentsMenu()
{
  lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
  lcd.clrScr();
  lcd_setCurrentMenu(MENU_CHOOSE_FILE);
  lcd_drawButtons();
}

/*
This intialises the LCD itself, and builds the map of the button corner coordinates.
*/
void lcd_initLCD()
{
  lcd.InitLCD(LANDSCAPE);
  lcd.clrScr();

  
  // output lcd size
//  lcd.printNumI(lcd.getDisplayXSize(), 10, 40);
//  lcd.printNumI(lcd.getDisplayYSize(), 10, 52);


  buttonSize = (lcd.getDisplayXSize() - (gap*4)) / 3;
  grooveSize = lcd.getDisplayYSize() - (gap*2) - (buttonSize*2);


  buttonCoords[0][0] = gap;
  buttonCoords[0][1] = gap;
  buttonCoords[1][0] = gap+buttonSize;
  buttonCoords[1][1] = gap+buttonSize;

  buttonCoords[2][0] = gap+buttonSize+gap;
  buttonCoords[2][1] = gap;
  buttonCoords[3][0] = gap+buttonSize+gap+buttonSize;
  buttonCoords[3][1] = gap+buttonSize;

  buttonCoords[4][0] = gap+buttonSize+gap+buttonSize+gap;
  buttonCoords[4][1] = gap;
  buttonCoords[5][0] = gap+buttonSize+gap+buttonSize+gap+buttonSize;
  buttonCoords[5][1] = gap+buttonSize;

  buttonCoords[6][0] = gap;
  buttonCoords[6][1] = gap+buttonSize+grooveSize;
  buttonCoords[7][0] = gap+buttonSize;
  buttonCoords[7][1] = gap+buttonSize+buttonSize+grooveSize;

  buttonCoords[8][0] = gap+buttonSize+gap;
  buttonCoords[8][1] = gap+buttonSize+grooveSize;
  buttonCoords[9][0] = gap+buttonSize+gap+buttonSize;
  buttonCoords[9][1] = gap+buttonSize+buttonSize+grooveSize;

  buttonCoords[10][0] = gap+buttonSize+gap+buttonSize+gap;
  buttonCoords[10][1] = gap+buttonSize+grooveSize;
  buttonCoords[11][0] = gap+buttonSize+gap+buttonSize+gap+buttonSize;
  buttonCoords[11][1] = gap+buttonSize+buttonSize+grooveSize;

  lcd.setBackColor(colorDarkRed, colorDarkGreen, colorDarkBlue);
  lcd.setColor(colorLightRed, colorLightGreen, colorLightBlue);
  lcd.fillRect(0,buttonCoords[5][1], screenWidth,buttonCoords[5][1]+56);
  lcd.setBackColor(colorLightRed, colorLightGreen, colorLightBlue);
  lcd.setColor(colorBrightRed, colorBrightGreen, colorBrightBlue);

  touch.InitTouch();
  touch.setPrecision(PREC_MEDIUM);

  lcd.setFont(BigFont);
  lcd.print("Polargraph.", 17, buttonCoords[5][1]+10);
  lcd.setFont(SmallFont);
  lcd.print("Drawing with robots.", 20, buttonCoords[5][1]+32);
  lcd.setBackColor(colorDarkRed, colorDarkGreen, colorDarkBlue);
  lcd.print("v"+FIRMWARE_VERSION_NO, 20, buttonCoords[5][1]+buttonCoords[5][1]+gap);
  
}

void lcd_showSummary()
{
  int rowHeight = 17;
  int row = 0;
  
  if (cardPresent) {
    lcd.print("SD card present", 20, buttonCoords[5][1]+buttonCoords[5][1]-gap);
    delay(500);
    if (cardInit) {
      lcd.setColor(colorDarkRed, colorDarkGreen, colorDarkBlue);
      lcd.fillRect(0, buttonCoords[5][1]+buttonCoords[5][1]-gap, screenWidth, buttonCoords[5][1]+buttonCoords[5][1]-gap+17);
      delay(500);
      lcd.setColor(colorBrightRed, colorBrightGreen, colorBrightBlue);
      String msg;
      lcd.print("Card loaded - ", 20, buttonCoords[5][1]+buttonCoords[5][1]-gap);
      if (useAutoStartFromSD) {
        if (autoStartFileFound) {
          msg = "Card loaded, running: " + autoStartFilename;
        }
        else {
          msg = "Card loaded, no " + autoStartFilename;
        }
      } 
      else {
        msg = "Card loaded.";
      }
      lcd.print(msg, 20, buttonCoords[5][1]+buttonCoords[5][1]-gap);
    }
    else
      lcd.print("Card init failed!", 10, row+=rowHeight);
  }
  else {
    lcd.print("No SD card present", 20, buttonCoords[5][1]+buttonCoords[5][1]-gap);
  }
    
}

void lcd_drawButtons()
{
  lcd.setBackColor(colorLightRed,colorLightGreen,colorLightBlue);
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);

  if (currentMenu == MENU_INITIAL)
  {
    if (powerIsOn)
      lcd_drawButton(BUTTON_POWER_OFF);
    else
      lcd_drawButton(BUTTON_POWER_ON);
    
    if (cardInit)
      lcd_drawButton(BUTTON_DRAW_FROM_SD);
      
    if (canCalibrate)  
      lcd_drawButton(BUTTON_CALIBRATE);
      
    if (isPenUp)
      lcd_drawButton(BUTTON_PEN_DOWN);
    else
      lcd_drawButton(BUTTON_PEN_UP);
    
    if (currentlyRunning)
      lcd_drawButton(BUTTON_PAUSE_RUNNING);
    else
      lcd_drawButton(BUTTON_RESUME_RUNNING);
      
    lcd_drawButton(BUTTON_SETTINGS_MENU);
    
  }
  else if (currentMenu == MENU_RUNNING)
  {
    lcd_drawButton(BUTTON_ADJUST_SPEED_MENU);
    lcd_drawButton(BUTTON_ADJUST_PENSIZE_MENU);
    
    if (currentlyRunning)
      lcd_drawButton(BUTTON_PAUSE_RUNNING);
    else
      lcd_drawButton(BUTTON_RESUME_RUNNING);
  }
  else if (currentMenu == MENU_CHOOSE_FILE)
  {
    lcd_drawButton(BUTTON_RESET_SD);
    lcd_drawButton(BUTTON_NEXT_FILE);
    lcd_drawButton(BUTTON_PREV_FILE);
    if (currentlyDrawingFromFile) 
      lcd_drawButton(BUTTON_STOP_FILE);
    else
      lcd_drawButton(BUTTON_DRAW_THIS_FILE);
    lcd_drawButton(BUTTON_CANCEL_FILE);
    lcd_drawCurrentSelectedFilename();
  }
  else if (currentMenu == MENU_ADJUST_SPEED)
  {
    lcd_drawButton(BUTTON_INC_SPEED);
    lcd_drawButton(BUTTON_DEC_SPEED);
    lcd_drawButton(BUTTON_INC_ACCEL);
    lcd_drawButton(BUTTON_DEC_ACCEL);
    lcd_drawButton(BUTTON_DONE);
    lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, currentMaxSpeed);
    lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, currentAcceleration);
  }
  else if (currentMenu == MENU_ADJUST_PENSIZE)
  {
    lcd_drawButton(BUTTON_INC_PENSIZE);
    lcd_drawButton(BUTTON_DEC_PENSIZE);
    lcd_drawButton(BUTTON_INC_PENSIZE_INC);
    lcd_drawButton(BUTTON_DEC_PENSIZE_INC);
    lcd_drawButton(BUTTON_DONE);
    lcd_drawFloatWithBackground(buttonCoords[8][0], centreYPosition, penWidthIncrement);
    lcd_drawFloatWithBackground(buttonCoords[10][0], centreYPosition, penWidth);
  }
  else if (currentMenu == MENU_ADJUST_POSITION)
  {
    lcd_drawButton(BUTTON_MOVE_INC_A);
    lcd_drawButton(BUTTON_MOVE_DEC_A);
    lcd_drawButton(BUTTON_MOVE_INC_B);
    lcd_drawButton(BUTTON_MOVE_DEC_B);
    lcd_drawButton(BUTTON_DONE);
    lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, motorA.currentPosition());
    lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, motorB.currentPosition());
  }
  else if (currentMenu == MENU_SETTINGS)
  {
    lcd_drawButton(BUTTON_ADJUST_POSITION_MENU);
    lcd_drawButton(BUTTON_DONE);
    lcd_drawButton(BUTTON_ADJUST_SPEED_MENU);
    lcd_drawButton(BUTTON_ADJUST_PENSIZE_MENU);
    lcd_drawButton(BUTTON_TOGGLE_ECHO);
    lcd_drawButton(BUTTON_SETTINGS_MENU_2);
  }
  else if (currentMenu == MENU_SETTINGS_2)
  {
    lcd_drawButton(BUTTON_ADJUST_PENLIFT);
    lcd_drawButton(BUTTON_DONE);
  }
  else if (currentMenu == MENU_ADJUST_PENLIFT)
  {
    lcd_drawButton(BUTTON_INC_PENLIFT_UP);
    lcd_drawButton(BUTTON_DEC_PENLIFT_UP);
    lcd_drawButton(BUTTON_INC_PENLIFT_DOWN);
    lcd_drawButton(BUTTON_DEC_PENLIFT_DOWN);
    lcd_drawButton(BUTTON_PENLIFT_SAVE_TO_EEPROM);
    lcd_drawButton(BUTTON_DONE);
    lcd_drawNumberWithBackground(buttonCoords[8][0], centreYPosition, downPosition);
    lcd_drawNumberWithBackground(buttonCoords[10][0], centreYPosition, upPosition);
  }
}

void lcd_drawButtonBackground(byte coordsIndex)
{
  lcd.setColor(colorLightRed,colorLightGreen,colorLightBlue);
  lcd.fillRect(buttonCoords[coordsIndex][0], buttonCoords[coordsIndex][1], 
    buttonCoords[coordsIndex+1][0], buttonCoords[coordsIndex+1][1]);
    
  // set the colours to write the text
  lcd.setBackColor(colorLightRed,colorLightGreen,colorLightBlue);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
}
void lcd_outlinePressedButton(byte pressedButton, byte r, byte g, byte b)
{
  Serial.println("Outlining.");
  if (pressedButton >= 1 && pressedButton <=6)
  {
    byte coordsIndex = (pressedButton * 2)-2;
    lcd.setColor(r,g,b);
    lcd.drawRect(buttonCoords[coordsIndex][0], buttonCoords[coordsIndex][1], 
      buttonCoords[coordsIndex+1][0], buttonCoords[coordsIndex+1][1]-1);
    lcd.drawRect(buttonCoords[coordsIndex][0]+1, buttonCoords[coordsIndex][1]+1, 
      buttonCoords[coordsIndex+1][0]-1, buttonCoords[coordsIndex+1][1]-2);
    lcd.setBackColor(colorLightRed,colorLightGreen,colorLightBlue);
    lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  }
  Serial.println("Outlined!");
}
void lcd_drawButton(byte but)
{
  byte coordsIndex = 0;
  switch (but)
  {
    case BUTTON_SET_HOME: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("SET", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("HOME", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_POWER_ON: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("MOTORS", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("ON", buttonCoords[coordsIndex][0]+(buttonSize/2)-6, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_POWER_OFF: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("MOTORS", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("OFF", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_TOGGLE_ECHO: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("TOGGLE", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("COMMS", buttonCoords[coordsIndex][0]+(buttonSize/2)-16, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
      
    case BUTTON_ADJUST_PENLIFT: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("ADJUST", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("PENLIFT", buttonCoords[coordsIndex][0]+(buttonSize/2)-24, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
//    case BUTTON_RESET_SD: // pos 1
//      coordsIndex=0;
//      lcd_drawButtonBackground(coordsIndex);
//      lcd.print("RESET", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
//      lcd.print("CARD", buttonCoords[coordsIndex][0]+(buttonSize/2)-12, buttonCoords[coordsIndex][1]+(buttonSize/2));
//      break;
    case BUTTON_DRAW_FROM_SD: // pos 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DRAW", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("FROM", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("SD", buttonCoords[coordsIndex][0]+(buttonSize/2)-6, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_MORE_RIGHT: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("MORE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      break;
    case BUTTON_CALIBRATE: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("CALI-", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("BRATE", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_PAUSE_RUNNING: // pos 4
      coordsIndex=6;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("PAUSE", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      break;
    case BUTTON_RESUME_RUNNING: // pos 4
      coordsIndex=6;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("RESUME", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      break;
    case BUTTON_RESET: // pos 6
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("RESET", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      break;
    case BUTTON_PEN_UP: // pos 4
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("PEN", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("UP", buttonCoords[coordsIndex][0]+(buttonSize/2)-6, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_PEN_DOWN: // pos 4
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("PEN", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("DOWN", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_PREV_FILE: // pos 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("PREV", buttonCoords[coordsIndex][0]+(buttonSize/2)-12, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("FILE", buttonCoords[coordsIndex][0]+(buttonSize/2)-12, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_NEXT_FILE: // pos 5
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("NEXT", buttonCoords[coordsIndex][0]+(buttonSize/2)-12, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("FILE", buttonCoords[coordsIndex][0]+(buttonSize/2)-12, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_CANCEL_FILE: // pos 4
      coordsIndex=6;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("BACK", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      break;
    case BUTTON_DRAW_THIS_FILE: // pos 2
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DRAW", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("THIS", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("FILE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_STOP_FILE: // pos 2
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("STOP", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("THIS", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("FILE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_SETTINGS_MENU: // pos 2
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("MORE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("...", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_SETTINGS_MENU_2: // pos 2
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("MORE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("...", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_DEC_SPEED: // button 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("SPEED", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_INC_SPEED: // button 5
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("SPEED", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_DEC_ACCEL: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("ACCEL", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_INC_ACCEL: // pos 6
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("ACCEL", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_DEC_PENSIZE_INC: // button 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("INCR", buttonCoords[coordsIndex][0]+(buttonSize/2)-16, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_INC_PENSIZE_INC: // button 5
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("INCR", buttonCoords[coordsIndex][0]+(buttonSize/2)-16, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_DEC_PENSIZE: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("PEN", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("SIZE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_INC_PENSIZE: // pos 6
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("PEN", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("SIZE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_DONE: // pos 4
      coordsIndex=6;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DONE", buttonCoords[6][0]+(buttonSize/2)-18, buttonCoords[6][1]+(buttonSize/2)-6);
      break;
    case BUTTON_ADJUST_PENSIZE_MENU: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("ADJ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-18);
      lcd.print("PEN", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-6);
      lcd.print("SIZE", buttonCoords[coordsIndex][0]+(buttonSize/2)-14, buttonCoords[coordsIndex][1]+(buttonSize/2)+6);
      break;
    case BUTTON_ADJUST_SPEED_MENU: // pos 6
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("ADJ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("SPEED", buttonCoords[coordsIndex][0]+(buttonSize/2)-18, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_ADJUST_POSITION_MENU: // pos 4
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("ADJ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("POS", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
      
    case BUTTON_MOVE_DEC_A: // button 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print(" A ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_MOVE_INC_A: // button 5
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print(" A ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_MOVE_DEC_B: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print(" B ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_MOVE_INC_B: // pos 6
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print(" B ", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;


    case BUTTON_DEC_PENLIFT_DOWN: // button 2
      coordsIndex=2;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("PEN DOWN", buttonCoords[coordsIndex][0]+(buttonSize/2)-26, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_INC_PENLIFT_DOWN: // button 5
      coordsIndex=8;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("PEN DOWN", buttonCoords[coordsIndex][0]+(buttonSize/2)-26, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_DEC_PENLIFT_UP: // pos 3
      coordsIndex=4;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("DEC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("PEN UP", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_INC_PENLIFT_UP: // pos 6
      coordsIndex=10;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("INC", buttonCoords[coordsIndex][0]+(buttonSize/2)-10, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("PEN UP", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;
    case BUTTON_PENLIFT_SAVE_TO_EEPROM: // pos 1
      coordsIndex=0;
      lcd_drawButtonBackground(coordsIndex);
      lcd.print("SAVE TO", buttonCoords[coordsIndex][0]+(buttonSize/2)-24, buttonCoords[coordsIndex][1]+(buttonSize/2)-12);
      lcd.print("EEPROM", buttonCoords[coordsIndex][0]+(buttonSize/2)-22, buttonCoords[coordsIndex][1]+(buttonSize/2));
      break;

  }
}


void lcd_displayMachineSpec()
{
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.clrScr();
  lcd.fillRect(10, 10, 210, 166);
  lcd.setBackColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
  
  const int ROW_HEIGHT = 12;
  int currentRow = 30;
  byte key = 20;
  byte val = 130;
  
  lcd.print("Width: ", key, currentRow);
  lcd.printNumI(machineWidth, val, currentRow);
  
  currentRow += ROW_HEIGHT;
  lcd.print("Height:", key, currentRow);
  lcd.printNumI(machineHeight, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("MmPerRev:", key, currentRow);
  lcd.printNumF(mmPerRev, 5, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("StepsPerRev:", key, currentRow);
  lcd.printNumI(motorStepsPerRev, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("MmPerStep:", key, currentRow);
  lcd.printNumF(mmPerStep,5, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("StepsPerMM:", key, currentRow);
  lcd.printNumF(stepsPerMM,5, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("StepMulti:", key, currentRow);
  lcd.printNumI(stepMultiplier,val, currentRow);
  
  currentRow += ROW_HEIGHT;
  lcd.print("PageWidth:", key, currentRow);
  lcd.printNumI(pageWidth, val, currentRow);

  currentRow += ROW_HEIGHT;
  lcd.print("PageHeight:", key, currentRow);
  lcd.printNumI(pageHeight, val, currentRow);


  currentRow += ROW_HEIGHT;
  lcd.print("Name:", key, currentRow);
  lcd.print(machineName, val, currentRow);
  
}


void lcd_drawCurrentSelectedFilename()
{
  // erase the previous stuff
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.fillRect(buttonCoords[0][0],buttonCoords[1][1]+10, screenWidth, buttonCoords[1][1]+24);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
  lcd.setBackColor(colorDarkRed,colorDarkGreen,colorDarkBlue);

  // see if there's one already found
  String msg = "No card found";

  if (commandFilename == "" || commandFilename == "            ")
  {
    if (cardInit) {
      commandFilename = lcd_loadFilename("", 1);
      msg = commandFilename;
    }
  }
  else
  {
    msg = commandFilename;
  }
  
  lcd.print(msg, buttonCoords[0][0],buttonCoords[1][1]+10);
}

String lcd_loadFilename(String selectedFilename, int direction)
{
  Serial.print("Current command filename: ");
  Serial.println(selectedFilename);
  Serial.println("Loading filename.");
 
  // start from the beginning
//  root = SD.open("/");
  root.rewindDirectory();

  // deal with first showing
  if (selectedFilename == "") {
    File entry =  root.openNextFile();
    if (entry)
      selectedFilename = entry.name();
    entry.close();
  }
  else {
    if (direction > 0) {
      selectedFilename = lcd_getNextFile(selectedFilename);
    }
    else if (direction < 0) {
      selectedFilename = lcd_getPreviousFile(selectedFilename);
    }
  }

  Serial.print("Now command filename: ");
  Serial.println(selectedFilename);

  return selectedFilename;
}

String lcd_getNextFile(String selectedFilename) {
  boolean found = false;
  while(!found) {
    File entry = root.openNextFile();
//    Serial.println(entry.name());
    if (entry) {
      if (selectedFilename.equals(entry.name())) {
        Serial.println("Found file!");
        found = true;
        entry.close();

        // looking for next file
        entry = root.openNextFile();
        if (entry) {
          selectedFilename = entry.name();
        }
        entry.close();
      }
    } 
    else {
      found = true;
    }
    
    entry.close();
  }
  return selectedFilename;
}

String lcd_getPreviousFile(String selectedFilename) {
  boolean fileIncremented = false;
  String prevFilename = "";
  
  // see if it is the first one, and just return straight away if so
  File entry =  root.openNextFile();
  if (entry) {
    if (selectedFilename.equals(entry.name())) {
      Serial.println("ent2");
      entry.close();
      return selectedFilename;
    }
  }
  
  // else go through and find the currently selected file, and keep track of the previous filename
  prevFilename = entry.name();
  Serial.print("Prev file: ");
  Serial.println(prevFilename);
  
  while (true) {
      entry.close();
      entry =  root.openNextFile();
      if (entry) {
//        Serial.print("next file: ");
//        Serial.println(entry.name());
        if (selectedFilename.equals(entry.name())) {
          selectedFilename = prevFilename;
          break;
        }
        else {
          prevFilename = entry.name();
        }
      }
      else break;
  } 
  entry.close();
  return selectedFilename;
}


void lcd_echoLastCommandToDisplay(String com, String prefix)
{
  if (currentMenu != MENU_INITIAL) return;
  
  lcd.setColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.fillRect(buttonCoords[0][0],buttonCoords[1][1]+10, screenWidth, buttonCoords[1][1]+24);
  lcd.setColor(colorBrightRed,colorBrightGreen,colorBrightBlue);
  lcd.setBackColor(colorDarkRed,colorDarkGreen,colorDarkBlue);
  lcd.print(prefix + com, buttonCoords[0][0],buttonCoords[1][1]+10);
}

byte lcd_getWhichButtonPressed(byte buttonNumber, byte menu)
{
  if (currentMenu == MENU_INITIAL)
  {
    switch (buttonNumber)
    {
      case 1: 
        if (powerIsOn) return BUTTON_POWER_OFF;
        else return BUTTON_POWER_ON; 
        break;
      case 2: 
        if (cardInit) return BUTTON_DRAW_FROM_SD;
        break;
      case 3: if (canCalibrate) { return BUTTON_CALIBRATE; } break;
      case 4:
        if (currentlyRunning) return BUTTON_PAUSE_RUNNING;
        else return BUTTON_RESUME_RUNNING;
      case 5: 
        if (isPenUp) return BUTTON_PEN_DOWN;
        else return BUTTON_PEN_UP; 
        break;
      case 6:
        return BUTTON_SETTINGS_MENU;
        break;
    }
  }
  else if (currentMenu == MENU_ADJUST_SPEED)
  {
    switch (buttonNumber)
    {
      case 2: return BUTTON_DEC_SPEED; break;
      case 5: return BUTTON_INC_SPEED; break;
      case 4: return BUTTON_DONE; break;
      case 3: return BUTTON_DEC_ACCEL; break;
      case 6: return BUTTON_INC_ACCEL; break;
    }
  }
  else if (currentMenu == MENU_ADJUST_PENSIZE)
  {
    switch (buttonNumber)
    {
      case 2: return BUTTON_DEC_PENSIZE_INC; break;
      case 5: return BUTTON_INC_PENSIZE_INC; break;
      case 4: return BUTTON_DONE; break;
      case 3: return BUTTON_DEC_PENSIZE; break;
      case 6: return BUTTON_INC_PENSIZE; break;
    }
  }
  else if (currentMenu == MENU_ADJUST_POSITION)
  {
    switch (buttonNumber)
    {
      case 2: return BUTTON_MOVE_DEC_A; break;
      case 5: return BUTTON_MOVE_INC_A; break;
      case 4: return BUTTON_DONE; break;
      case 3: return BUTTON_MOVE_DEC_B; break;
      case 6: return BUTTON_MOVE_INC_B; break;
    }
  }
  else if (currentMenu == MENU_ADJUST_PENLIFT)
  {
    switch (buttonNumber)
    {
      case 1: return BUTTON_PENLIFT_SAVE_TO_EEPROM; break;
      case 2: return BUTTON_DEC_PENLIFT_DOWN; break;
      case 3: return BUTTON_DEC_PENLIFT_UP; break;
      case 4: return BUTTON_DONE; break;
      case 5: return BUTTON_INC_PENLIFT_DOWN; break;
      case 6: return BUTTON_INC_PENLIFT_UP; break;
    }
  }
  else if (currentMenu == MENU_CHOOSE_FILE)
  {
    switch (buttonNumber)
    {
      case 2: return BUTTON_PREV_FILE; break;
      case 4: return BUTTON_CANCEL_FILE; break;
      case 5: return BUTTON_NEXT_FILE; break;
      case 6: 
        if (currentlyDrawingFromFile) return BUTTON_STOP_FILE;
        else return BUTTON_DRAW_THIS_FILE; 
        break;
    }
  }
  else if (currentMenu == MENU_SETTINGS)
  {
    switch (buttonNumber)
    {
      case 1: return BUTTON_TOGGLE_ECHO; break;
      case 2: return BUTTON_ADJUST_POSITION_MENU; break;
      case 3: return BUTTON_ADJUST_PENSIZE_MENU; break;
      case 4: return BUTTON_DONE; break;
      case 5: return BUTTON_ADJUST_SPEED_MENU; break;
      case 6: return BUTTON_SETTINGS_MENU_2; break;
    }
  }
  else if (currentMenu == MENU_SETTINGS_2)
  {
    switch (buttonNumber)
    {
      case 1: return BUTTON_ADJUST_PENLIFT; break;
      case 4: return BUTTON_DONE; break;
    }
  }
}

byte lcd_getButtonNumber(int x, int y)
{
  Serial.print("X:");
  Serial.print(x);
  Serial.print(", Y:");
  Serial.println(y);
  if (x >= buttonCoords[0][0] && x <= buttonCoords[1][0] 
     && y >= buttonCoords[0][1] && y <= buttonCoords[1][1])
    return 1;
  else if (x >= buttonCoords[2][0] && x <= buttonCoords[3][0] 
     && y >= buttonCoords[2][1] && y <= buttonCoords[1][1])
    return 2;   
  else if (x >= buttonCoords[4][0] && x <= buttonCoords[5][0] 
     && y >= buttonCoords[4][1] && y <= buttonCoords[5][1])
    return 3;   

  else if (x >= buttonCoords[6][0] && x <= buttonCoords[7][0] 
     && y >= buttonCoords[6][1] && y <= buttonCoords[7][1])
     return 4;
  else if (x >= buttonCoords[8][0] && x <= buttonCoords[9][0] 
     && y >= buttonCoords[8][1] && y <= buttonCoords[9][1])
     return 5;
  else if (x >= buttonCoords[10][0] && x <= buttonCoords[11][0] 
     && y >= buttonCoords[10][1] && y <= buttonCoords[11][1])
    return 6;
}
