/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_polarshield

EEPROM.

This is one of the core files for the polargraph server program.  
Has a couple of little helper methods for reading and writing
ints and floats to EEPROM using the EEPROM library.

But mostly it contains the loadMachineSpecFromEeprom method, which is
used to retrieve the machines saved values when it restarts, or 
whenever a value is written to the EEPROM.

*/

void eeprom_resetEeprom()
{
  for (int i = 0; i < (EEPROM_PENLIFT_UP +2); i++)
  {
    EEPROM.write(i, 0);
  }
  eeprom_loadMachineSpecFromEeprom();
}
void eeprom_dumpEeprom()
{
  for (int i = 0; i <40; i++)
  {
    Serial.print(i);
    Serial.print(". ");
    Serial.println(EEPROM.read(i));
  }
}  

void eeprom_loadMachineSize()
{
  EEPROM_readAnything(EEPROM_MACHINE_WIDTH, machineWidth);
  if (machineWidth < 1)
  {
    machineWidth = defaultMachineWidth;
  }
  Serial.print(F("Loaded machine width:"));
  Serial.println(machineWidth);
  
  EEPROM_readAnything(EEPROM_MACHINE_HEIGHT, machineHeight);
  if (machineHeight < 1)
  {
    machineHeight = defaultMachineHeight;
  }
  Serial.print(F("Loaded machine height:"));
  Serial.println(machineHeight);
}

void eeprom_loadSpoolSpec()
{
  EEPROM_readAnything(EEPROM_MACHINE_MM_PER_REV, mmPerRev);
  if (mmPerRev < 1)
  {
    mmPerRev = defaultMmPerRev;
  }
  Serial.print(F("Loaded mm per rev:"));
  Serial.println(mmPerRev);

  EEPROM_readAnything(EEPROM_MACHINE_STEPS_PER_REV, motorStepsPerRev);
  if (motorStepsPerRev < 1)
  {
    motorStepsPerRev = defaultStepsPerRev;
  }
  Serial.print(F("Loaded motor steps per rev:"));
  Serial.println(motorStepsPerRev);
}  

void eeprom_loadPenLiftRange()
{
  EEPROM_readAnything(EEPROM_PENLIFT_DOWN, downPosition);
  if (downPosition < 0)
  {
    downPosition = DEFAULT_DOWN_POSITION;
  }
  Serial.print(F("Loaded down pos:"));
  Serial.println(downPosition);

  EEPROM_readAnything(EEPROM_PENLIFT_UP, upPosition);
  if (upPosition < 0)
  {
    upPosition = DEFAULT_UP_POSITION;
  }
  Serial.print(F("Loaded up pos:"));
  Serial.println(upPosition);
}  

void eeprom_loadMachineName()
{
  String name = "";
  for (int i = 0; i < 8; i++)
  {
    char b = EEPROM.read(EEPROM_MACHINE_NAME+i);
    name = name + b;
  }
  
  if (name[0] == 0)
    name = DEFAULT_MACHINE_NAME;
  maxLength = 0;
  machineName = name;
  Serial.print(F("Loaded machine name:"));
  Serial.println(machineName);
}

void eeprom_loadStepMultiplier()
{
  EEPROM_readAnything(EEPROM_MACHINE_STEP_MULTIPLIER, stepMultiplier);
  if (stepMultiplier < 1)
  {
    stepMultiplier = defaultStepMultiplier;
  }
  Serial.print(F("Loaded motor step multiplier:"));
  Serial.println(stepMultiplier);  
}  

void eeprom_loadSpeed()
{
  // load speed, acceleration
  EEPROM_readAnything(EEPROM_MACHINE_MOTOR_SPEED, currentMaxSpeed);
  
  // not sure why this requires a cast to int for the comparision, but a 
  // if (currentMaxSpeed < 1.0) wasn't catching cases where 
  // currentMaxSpeed == 0.00, ODD.
  if (int(currentMaxSpeed) < 1) {
    currentMaxSpeed = 800.0;
  }
    
  EEPROM_readAnything(EEPROM_MACHINE_MOTOR_ACCEL, currentAcceleration);
  if (int(currentAcceleration) < 1) {
    currentAcceleration = 800.0;
  }
}

void eeprom_loadMachineSpecFromEeprom()
{
  impl_loadMachineSpecFromEeprom();

  eeprom_loadMachineSize();
  eeprom_loadSpoolSpec();
  eeprom_loadStepMultiplier();
  eeprom_loadMachineName();
  eeprom_loadPenLiftRange();
  eeprom_loadSpeed();

  // load penwidth
  float pWidth;
  EEPROM_readAnything(EEPROM_MACHINE_PEN_WIDTH, pWidth);

  if (!isnan(pWidth)) {
    penWidth = pWidth;
  }

  Serial.print("penWidth: ");
  Serial.println(penWidth);

  mmPerStep = mmPerRev / multiplier(motorStepsPerRev);
  stepsPerMM = multiplier(motorStepsPerRev) / mmPerRev;
  
  Serial.print(F("Recalc mmPerStep ("));
  Serial.print(mmPerStep);
  Serial.print(F("), stepsPerMM ("));
  Serial.print(stepsPerMM);
  Serial.print(F(")"));
  Serial.println();

  pageWidth = machineWidth * stepsPerMM;
  Serial.print(F("Recalc pageWidth in steps ("));
  Serial.print(pageWidth);
  Serial.print(F(")"));
  Serial.println();
  pageHeight = machineHeight * stepsPerMM;
  Serial.print(F("Recalc pageHeight in steps ("));
  Serial.print(pageHeight);
  Serial.print(F(")"));
  Serial.println();

  maxLength = 0;
}

