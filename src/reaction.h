
void dealWithExceptions()
{
  if (checkGyro && token != T_CALIBRATE && exceptions)
  { // the gyro reaction switch can be toggled on/off by the 'g' token
    soundFallOver();
    //  for (int m = 0; m < 2; m++)
    //    meow(30 - m * 12, 42 - m * 12, 20);
    token = 'k';
    strcpy(newCmd, "rc");
    newCmdIdx = -1;
  }
}

#ifdef VOLTAGE
float vFactor = 4096 / 3.3 / 3;
float low_voltage = LOW_VOLTAGE * vFactor;
bool lowBattery()
{
  long currentTime = millis() / CHECK_BATTERY_PERIOD;
  if (currentTime > uptime)
  {
    uptime = currentTime;
    float voltage = analogRead(VOLTAGE);
    if (voltage == 0 || voltage < low_voltage && abs(voltage - lastVoltage) > 10)
    { // if battery voltage < threshold, it needs to be recharged
      // give the robot a break when voltage drops after sprint
      // adjust the thresholds according to your batteries' voltage
      // if set too high, the robot will stop working when the battery still has power.
      // If too low, the robot may not alarm before the battery shuts off
      if (!safeRest)
      {
        strcpy(lastCmd, "rest");
        loadBySkillName(lastCmd);
        shutServos();
        safeRest = true;
      }
      PT("Low power: ");
      PT(voltage / vFactor);
      PTL("V");

      playMelody(melodyLowBattery, sizeof(melodyLowBattery) / 2);

      //    strip.show();
      int8_t bStep = 1;
      for (byte brightness = 1; brightness > 0; brightness += bStep)
      {
        if (brightness == 255)
          bStep = -1;
        delay(5);
      }
      lastVoltage = voltage;
      return true;
    }
    if (safeRest)
    {
      strcpy(lastCmd, "rest");
      loadBySkillName(lastCmd);
      shutServos();
      safeRest = false;
    }
    lastVoltage = voltage;
  }
  return false;
}
#endif

void resetCmd()
{
  //  PTL("lastT: " + String(lastToken) + "\tT: " + String(token) + "\tLastCmd: " + String(lastCmd) + "\tCmd: " + String(newCmd));
  if (lastToken == T_SKILL)
    idleThreshold = IDLE_SHORT;
  else
    idleThreshold = IDLE_LONG;
  if (strcmp(newCmd, "rc") && token != T_INDEXED_SIMULTANEOUS_BIN && token != T_LISTED_BIN && token != T_MOVE_BIN && token != T_SKILL_DATA && token != T_COLOR)
  {
    strcpy(lastCmd, newCmd);
  }
  newCmd[0] = '\0';
  newCmdIdx = 0;
  lastToken = token;
  if (token != T_SKILL && token != T_CALIBRATE)
    token = '\0';
  //  PTL("******");
}

void reaction()
{
  if (newCmdIdx)
  {
    //    PTL("lastT: " + String(lastToken) + "\tT: " + String(token) + "\tLastCmd: " + String(lastCmd) + "\tCmd: " + String(newCmd));
    if (initialBoot)
    { //-1 for marking the bootup calibration state
      checkGyro = true;
      autoSwitch = RANDOM_MIND;
      initialBoot = false;
    }
    if (token != T_REST && newCmdIdx < 5)
      idleTimer = millis();
    if (newCmdIdx < 5 && token != T_BEEP && token != T_MEOW && token != T_LISTED_BIN && token != T_INDEXED_SIMULTANEOUS_BIN && token != T_TILT)
      beep(15 + newCmdIdx, 5); // ToDo: check the muted sound when newCmdIdx = -1
    if ((lastToken == T_CALIBRATE || lastToken == T_REST || !strcmp(lastCmd, "fd")) && token != T_CALIBRATE)
    {
#ifdef T_SERVO_MICROSECOND
//      setServoP(P_SOFT);
#endif
      checkGyro = true;
      printToken('G');
    }
    if (token != T_PAUSE && !tStep)
    {
      tStep = 1;
      printToken('p');
    }
    switch (token)
    {
    case T_GYRO:
    case T_PRINT_GYRO:
    case T_VERBOSELY_PRINT_GYRO:
    case T_RANDOM_MIND:
    case T_RAMP:
    {
      if (token == T_GYRO)
      {
        checkGyro = !checkGyro;
        token = checkGyro ? 'G' : 'g'; // G for activated gyro
      }
#ifdef GYRO_PIN
      else if (token == T_PRINT_GYRO)
      {
        print6Axis();
      }
#endif
      else if (token == T_VERBOSELY_PRINT_GYRO)
      {
        printGyro = !printGyro;
        token = printGyro ? 'V' : 'v'; // V for verbosely print gyro data
      }
      else if (token == T_RANDOM_MIND)
      {
        autoSwitch = !autoSwitch;
        token = autoSwitch ? 'Z' : 'z'; // G for activated gyro
      }
      else if (token == T_RAMP)
      {
        ramp = -ramp;
        token = ramp > 0 ? 'R' : 'r'; // G for activated gyro
      }
      break;
    }
    case T_PAUSE:
    {
      tStep = !tStep;            // tStep can be -1
      token = tStep ? 'p' : 'P'; // P for pause activated
      if (!tStep)
        shutServos();
      break;
    }
    case T_ACCELERATE:
    {
      runDelay = max(0, runDelay - 1);
      break;
    }
    case T_DECELERATE:
    {
      runDelay = min(delayLong, runDelay + 1);
      break;
    }
    case T_REST:
    {
      strcpy(newCmd, "rest");
      if (strcmp(newCmd, lastCmd))
      {
        loadBySkillName(newCmd);
      }
      shutServos();
      checkGyro = false;
      printToken('g');
      break;
    }
    case T_JOINTS:
    { // show the list of current joint anles
      //          printRange(DOF);
      //          printList(currentAng);
      printTable(currentAng);
      if (deviceConnected)
      {
        bleWrite(range2String(DOF));
        bleWrite(list2String(currentAng));
      }
      break;
    }
    case T_MELODY:
    {
      playMelody(melody1, sizeof(melody1) / 2);
      break;
    }
#ifdef ULTRASONIC
    case T_COLOR:
    {
      long color = ((long)(dataBuffer[0]) << 16) + ((long)(dataBuffer[1]) << 8) + (long)(dataBuffer[2]);
      if (dataBuffer[4] == -1) // no special effect
        mRUS04.SetRgbColor(E_RGB_INDEX(dataBuffer[3]), color);
      else
        mRUS04.SetRgbEffect(E_RGB_INDEX(dataBuffer[3]), color, dataBuffer[4]);
      break;
    }
#endif
    case T_SAVE:
    {
      PTLF("save offset");
      saveCalib(servoCalib);
      break;
    }
    case T_ABORT:
    {
      PTLF("aborted");
      i2c_eeprom_read_buffer(EEPROM_CALIB, (byte *)servoCalib, DOF);
      break;
    }
    case T_RESET:
    {
      i2c_eeprom_write_byte(EEPROM_BIRTHMARK_ADDRESS, 'R'); // esp_random() % 128); //mark the board as uninitialized
      PTL("Alter the birthmark for reset!");
      delay(5);
      ESP.restart();
      break;
    }
    case T_CALIBRATE:                // calibration
    case T_MOVE_ASC:                 // move multiple indexed joints to angles once at a time (ASCII format entered in the serial monitor)
    case T_INDEXED_SIMULTANEOUS_ASC: // move multiple indexed joints to angles simultaneously (ASCII format entered in the serial monitor)
#ifdef T_SERVO_MICROSECOND
    case T_SERVO_MICROSECOND: // send pulse with unit of microsecond to a servo
#endif
    case T_TILT: // tilt the robot, format: t axis angle. 0:yaw, 1:pitch, 2:roll
    case T_MEOW: // meow
    case T_BEEP: // beep(tone, duration): tone 0 is pause, duration range is 0~255
    {
      int targetFrame[DOF];
      arrayNCPY(targetFrame, currentAng, DOF);
      char *pch;
      char *input = (token == T_TILT) ? newCmd : (char *)dataBuffer;
      pch = strtok((char *)input, " ,");
      do
      { // it supports combining multiple commands at one time
        // for example: "m8 40 m8 -35 m 0 50" can be written as "m8 40 8 -35 0 50"
        // the combined commands should be less than four. string len <=30 to be exact.
        int target[2] = {};
        byte inLen = 0;
        for (byte b = 0; b < 2 && pch != NULL; b++)
        {
          target[b] = atoi(pch);
          pch = strtok(NULL, " ,\t");
          inLen++;
        }
        targetFrame[target[0]] = target[1];

        int angleStep = 0;
        if (token == T_CALIBRATE)
        {
          checkGyro = false;
          if (lastToken != T_CALIBRATE)
          {
#ifdef T_SERVO_MICROSECOND
            setServoP(P_HARD);
#endif
            strcpy(newCmd, "calib");
            loadBySkillName(newCmd);
          }
          if (inLen == 2)
          {
            if (target[1] >= 1001)
            { // Using 1001 for incremental calibration. 1001 is adding 1 degree, 1002 is adding 2 and 1009 is adding 9 degrees
              target[1] = servoCalib[target[0]] + target[1] - 1000;
            }
            else if (target[1] <= -1001)
            { // Using -1001 for incremental calibration. -1001 is removing 1 degree, 1002 is removing 2 and 1009 is removing 9 degrees
              target[1] = servoCalib[target[0]] + target[1] + 1000;
            }
            servoCalib[target[0]] = target[1];
          }

          int duty = zeroPosition[target[0]] + float(servoCalib[target[0]]) * rotationDirection[target[0]];
          int actualServoIndex = (PWM_NUM == 12 && target[0] > 3) ? target[0] - 4 : target[0];
          pwm.writeAngle(actualServoIndex, duty);
          //              printRange(DOF);
          //              printList(servoCalib);
          printTable(servoCalib);
          if (deviceConnected)
          {
            bleWrite(range2String(DOF));
            bleWrite(list2String(servoCalib));
          }
          PT(token);
          printList(target, 2);
        }
        else if (token == T_MOVE_ASC)
        {
          transform(targetFrame, 1, 1);
          delay(10);
        }
#ifdef T_SERVO_MICROSECOND
        else if (token == T_SERVO_MICROSECOND)
        {
          pwm.writeMicroseconds(PWM_pin[target[0]], target[1]);
        }
#endif
        else if (token == T_TILT)
        {
          yprTilt[target[0]] = target[1];
        }
        else if (token == T_MEOW)
        {
          meow(random() % 2 + 1, (random() % 4 + 2) * 10);
        }
        else if (token == T_BEEP)
        {
          if (target[1])
            beep(target[0], 1000 / target[1]);
        }
        delay(5);
      } while (pch != NULL);
      if (token == T_INDEXED_SIMULTANEOUS_ASC)
      {
        transform(targetFrame, 1, 0.5);
        //            delay(200);
      }
      delete[] pch;
      break;
    }
    // this block handles array like arguments
    case T_MOVE_BIN:
    case T_INDEXED_SIMULTANEOUS_BIN:
    { // indexed joint motions: joint0, angle0, joint1, angle1, ... (binary encoding)
      int targetFrame[DOF];
      for (int i = 0; i < DOF; i++)
      {
        targetFrame[i] = currentAng[i];
      }
      for (int i = 0; i < cmdLen; i += 2)
      {
        targetFrame[dataBuffer[i]] = dataBuffer[i + 1];
        if (token == T_MOVE_BIN)
        {
          transform(targetFrame, 1, 2);
          delay(10);
        }
      }
      if (token == T_INDEXED_SIMULTANEOUS_BIN)
        transform(targetFrame, 1, transformSpeed);
      break;
    }
    case T_LISTED_BIN:
    {                                           // list of all 16 joint: angle0, angle2,... angle15 (binary encoding)
      transform(dataBuffer, 1, transformSpeed); // need to add angleDataRatio if the angles are large
      break;
    }
    case T_BEEP_BIN:
    {
      for (byte b = 0; b < cmdLen / 2; b++)
        beep(dataBuffer[2 * b], 1000 / dataBuffer[2 * b + 1]);
      break;
    }
    case T_TEMP:
    { // call the last skill data received from the serial port
      loadDataFromI2cEeprom((unsigned int)i2c_eeprom_read_int16(SERIAL_BUFF));
      if (skill != NULL)
        delete[] skill;
      skill = new Skill(dataBuffer);
      //          skill->info();
      skill->transformToSkill(skill->nearestFrame());
      printToken(token);
      token = T_SKILL;
      break;
    }
    case T_SKILL_DATA:
    { // takes in the skill array from the serial port, load it as a regular skill object and run it locally without continuous communication with the master
      if (skill != NULL)
        delete[] skill;
      skill = new Skill(dataBuffer);
      skill->transformToSkill(skill->nearestFrame());
      unsigned int i2cEepromAddress = SERIAL_BUFF + 2 + esp_random() % (EEPROM_SIZE - SERIAL_BUFF - 2 - 2550); // save to random position to protect the EEPROM
      i2c_eeprom_write_int16(SERIAL_BUFF, i2cEepromAddress);
      copydataFromBufferToI2cEeprom(i2cEepromAddress, dataBuffer);
      newCmdIdx = 0;
      newCmd[0] = '\0';
      token = T_SKILL;

      break;
    }
    case T_SKILL:
    {
      if (strcmp(lastCmd, newCmd) || skill->period <= 1)
      { // won't transform for the same gait. skill->period can be NULL!
        // it's better to compare skill->skillName and newCmd.
        // but need more logics for non skill cmd in between
        loadBySkillName(newCmd);
      }
      break;
    }
    }
    if (token != T_SKILL || skill->period > 0)
    {
      printToken(); // postures, gaits and other tokens can confirm completion by sending the token back
      char lowerToken = tolower(token);
      if ((lowerToken == T_GYRO || lowerToken == T_PRINT_GYRO || lowerToken == T_JOINTS || lowerToken == T_BEEP || lowerToken == T_RANDOM_MIND || lowerToken == T_RAMP || lowerToken == T_ACCELERATE || lowerToken == T_DECELERATE) && lastToken == T_SKILL || token == T_PAUSE || token == T_TILT)
        token = T_SKILL;
    }
    resetCmd();
  }
  if (token == T_SKILL)
  {
    skill->perform();
    if (skill->period > 1)
      delay(delayShort + max(0,
                             int(runDelay - (max(abs(ypr[1]), abs(ypr[2])) / 10))));

    if (skill->period < 0)
    {
      if (exceptions && lastCmd[strlen(lastCmd) - 1] < 'L' && skillList->lookUp(lastCmd) > 0)
      { // can be simplified here. check OpenCat2.0
        strcpy(newCmd, lastCmd);
      }
      else if (!strcmp(skill->skillName, "fd"))
      { // need to optimize logic to combine "rest" and "fold"
        shutServos();
        checkGyro = false;
        printToken('g');
        idleTimer = 0;
        token = '\0';
      }
      else
      {
        //        strcpy(newCmd, "balance");
        strcpy(newCmd, "");
        arrayNCPY(skill->dutyAngles, skill->dutyAngles + (abs(skill->period) - 1) * skill->frameSize, DOF);
        skill->period = 1;
        frame = 0;
      }
      for (int i = 0; i < DOF; i++)
        currentAdjust[i] = 0;
      if (strcmp(newCmd, ""))
        loadBySkillName(newCmd);
      printToken(); // behavior can confirm completion by sending the token back
    }
  }
}
