#include "PetoiESP32Servo/ESP32Servo.h"

ServoModel servoG41(180, SERVO_FREQ, 500, 2500);
ServoModel servoP1S(290, SERVO_FREQ, 500, 2500); // 1s/4 = 250ms 250ms/2500us=100Hz
ServoModel servoP2K(290, SERVO_FREQ, 500, 2500);
#include "pcaServo.h"

#define P_STEP 32
#define P_BASE 3000 + 6 * P_STEP // 3000~3320
#define P_HARD (P_BASE + P_STEP * 2)
#define P_SOFT (P_BASE - P_STEP * 2)

// 16 servo objects can be created on the ESP32

// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
// Possible PWM GPIO pins on the ESP32-S2: 0(used by on-board button),1-17,18(used by on-board LED),19-21,26,33-42

void servoSetup()
{
    i2c_eeprom_read_buffer(EEPROM_CALIB, (byte *)servoCalib, DOF);
    //------------------angleRange  frequency  minPulse  maxPulse;
    ServoModel *model;
    Serial.println("Set up PCA9685 PWM servo driver...");
    pwm.begin();
    pwm.setup(servoModelList);
}

void setServoP(unsigned int p)
{
    for (byte s = 0; s < PWM_NUM; s++)
        pwm.writeMicroseconds(s, p);
}

void allRotate()
{
    for (int pos = -50; pos < 50; pos += 1)
    { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        for (int s = 0; s < PWM_NUM; s++)
        {
            pwm.writeAngle(s, pos);
            delay(1); // waits 15ms for the servo to reach the position
            Serial.println(pos);
        }
    }
    for (int pos = 50; pos > -50; pos -= 1)
    { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        for (int s = 0; s < PWM_NUM; s++)
        {
            // Esp32Board
            pwm.writeAngle(s, pos); // may go out of range. check!
            delay(1);               // waits 15ms for the servo to reach the position
            Serial.println(pos);
        }
    }
}

#ifdef GYRO_PIN
void allRotateWithIMU()
{
    for (int s = 0; s < PWM_NUM; s++)
    {
        // Esp32Board
        pwm.writeAngle(s, 90 + ypr[1] + ypr[2]);
        //    delay(1);             // waits 15ms for the servo to reach the position
        Serial.print(ypr[1]);
        Serial.print('\t');
        Serial.println(ypr[2]);
    }
}
#endif

// byte shutOrder[] = {4, 7, 8, 11, 0, 5, 6, 1, 2, 9, 10, 3};
// byte shutOrder[] = { 1, 2, 3, 5, 6, 8, 9, 10, 4, 7, 8, 11,};
// bool shutEsp32Servo = false;
void shutServos()
{
    ServoModel *model;
    for (byte s = 0; s < PWM_NUM; s++)
    { // PWM_NUM
        // using PCA9685
        pwm.setPWM(s, 0, 4096);
    }
    //  shutEsp32Servo = false;
}
