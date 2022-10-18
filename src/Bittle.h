/* ESP32Board
    PWM:
                        |--------------------------------
                        |    PWM[0]           PWM[6]    |
                        |    PWM[1]           PWM[7]    |
                        |    PWM[2]           PWM[8]    |
                        |-----------                    |
                        |           |                   |
                        |   ESP32   |  IMU         USB-C|~~~~Tail~~~~
                        |           |                   |
                        |-----------                    |
                        |    PWM[3]           PWM[9]    |
                        |    PWM[4]           PWM[10]   |
                        |    PWM[5]           PWM[11]   |
                        |-------------------------------|

    Pin Name    |   ESP32 Pin   |   Arduino Pin Name    |   Alternative Function
    PWM[0]          GPIO4               4                   GPIO / Ain / Touch
    PWM[1]          GPIO5               5                   GPIO / VSPI SS
    PWM[2]          GPIO18              18                  GPIO / VSPI SCK
    -----------------------------------------------------------------------------
    PWM[3]          GPIO32              32                  GPIO / Ain / Touch
    PWM[4]          GPIO33              33                  GPIO / Ain / Touch
    PWM[5]          GPIO19              19                  GPIO / VSPI MISO
    -----------------------------------------------------------------------------
    PWM[6]          GPIO2               2                   boot pin, DO NOT PUT HIGH WHEN BOOT!
    PWM[7]          GPIO15              15                  GPIO / HSPI SS / Ain Touch
    PWM[8]          GPIO13              13                  built-in LED / GPIO / HSPI MOSI / Ain / Touch
    -----------------------------------------------------------------------------
    PWM[9]          GPIO12              12                  GPIO / HSPI MISO / Ain / Touch
    PWM[10]         GPIO14              14                  GPIO / HSPI SCK / Ain / Touch
    PWM[11]         GPIO27              27                  GPIO / Ain / Touch

    I2C:

    Pin Name    |   ESP32 Pin   |   Arduino Pin Name    |   Alternative Function
    I2C-SCL         GPIO22              22                  Fixed - ICM20600 - Pulled
    I2C-SDA         GPIO21              21                  Fixed = ICM20600 - Pulled

    System default, nothing to declaration!

    Other Peripherals:

    Pin Name    |   ESP32 Pin   |   Arduino Pin Name    |   Alternative Function
    IR_Remote       GPIO23              23                  Fixed - VS1838B IR
    DAC_Out         GPIO25              25                  Fixed - PAM8302
    IMU_Int         GPIO26              26                  Fixed - MPU6050 Interrupt

    System default, nothing to declaration!
*/

/* Esp32Board
  IMU_Int     27
  BUZZER      14
  VOLTAGE     4
  RGB LED     15
  GREEN-LED   5
*/

/*  Total DOF            Walking DOF
                   Nybble    Bittle    Cub
   Esp32Board (16)  skip 0~8  skip 0~8  skip0~4
*/

#define BIRTHMARK 'x' // Send 'R' token to reset the birthmark in the EEPROM so that the robot will know to restart and reset
#ifdef Esp32Board     // Esp32Board
#define GYRO_PIN
#define PWM_NUM 16
#define INTERRUPT_PIN 27 // use pin 2 on Arduino Uno & most boards
#define BUZZER 14
#define VOLTAGE 4
#define LOW_VOLTAGE 6.8
// #define PWM_LED_PIN 5

//                                headPan, tilt, tailPan, NA
const uint8_t PWM_pin[PWM_NUM] = {
    12, 11, 4, 3,
    13, 10, 5, 2, // shoulder roll
    14, 9, 6, 1,  // shoulder pitch
    //                                  13,       10,     6,    2,     //shoulder roll
    //                                  14,        9,     5,    1,     //shoulder pitch
    15, 8, 7, 0 // knee
};
// L:Left R:Right F:Front B:Back   LF,        RF,    RB,   LB

#define SERVO_FREQ 240
#endif

#define DOF 16
#ifdef BITTLE
#define WALKING_DOF 8
#define GAIT_ARRAY_DOF 8
#endif

enum ServoModel_t
{
  G41 = 0,
  P1S,
  P2K
};

#ifdef BITTLE
#define HEAD
#define LL_LEG
#define REGULAR P1S
#define KNEE P1S
#include "InstinctBittleESP.h"
#endif

ServoModel_t servoModelList[] = {
    REGULAR, REGULAR, REGULAR, REGULAR,
    REGULAR, REGULAR, REGULAR, REGULAR,
    REGULAR, REGULAR, REGULAR, REGULAR,
    KNEE, KNEE, KNEE, KNEE};

bool newBoard = false;

#include <math.h>
// token list
#define T_ABORT 'a'
#define T_BEEP 'b'
#define T_BEEP_BIN 'B'
#define T_CALIBRATE 'c'
#define T_COLOR 'C'
#define T_REST 'd'
#define T_GYRO 'g'
#define T_HELP 'h'
#define T_INDEXED_SIMULTANEOUS_ASC 'i'
#define T_INDEXED_SIMULTANEOUS_BIN 'I'
#define T_JOINTS 'j'
#define T_SKILL 'k'
#define T_SKILL_DATA 'K'
#define T_LISTED_BIN 'L'
#define T_MOVE_ASC 'm'
#define T_MOVE_BIN 'M'
#define T_MELODY 'o'
#define T_PAUSE 'p'
#define T_RAMP 'r'
#define T_RESET 'R'
#define T_SAVE 's'
#define T_SERVO_MICROSECOND 'S'
#define T_TILT 't'
#define T_TEMP 'T' // call the last skill data received from the serial port
#define T_MEOW 'u'
#define T_PRINT_GYRO 'v'           // print Gyro data
#define T_VERBOSELY_PRINT_GYRO 'V' // verbosely print Gyro data
#define T_WORD 'w'
#define T_XLEG 'x'
#define T_ACCELERATE '.'
#define T_DECELERATE ','
#define T_RANDOM_MIND 'z'

bool updated[10];
float degPerRad = 180 / M_PI;
float radPerDeg = M_PI / 180;

// control related variables
#define CHECK_BATTERY_PERIOD 10000 // every 10 seconds. 60 mins -> 3600 seconds
int uptime = -1;
int frame;
int tStep = 1;
char token;
char lastToken;
#define CMD_LEN 10 // the last char will be '\0' so only CMD_LEN-1 elements are allowed
char *newCmd = new char[CMD_LEN];
char *lastCmd = new char[CMD_LEN];
int cmdLen;
byte newCmdIdx = 0;
int8_t *dataBuffer = new int8_t[1524];
int lastVoltage;

bool checkGyro = false;
bool printGyro = false;
bool autoSwitch = false;
bool walkingQ = false;
bool imuUpdated;
byte exceptions = 0;
byte transformSpeed = 2;
float protectiveShift; // reduce the wearing of the potentiometer

bool initialBoot = true;
bool safeRest = true;

int delayLong = 14;
int delayMid = 8;
int delayShort = 2;
int delayStep = 1;
int runDelay = delayMid;

int8_t middleShift[] = {0, 15, 0, 0,
                        -45, -45, -45, -45,
                        55, 55, -55, -55,
                        -55, -55, -55, -55};
int8_t rotationDirection[] = {1, -1, 1, 1,
                              1, -1, 1, -1,
                              1, -1, -1, 1,
                              -1, 1, 1, -1};
int angleLimit[][2] = {
    {-120, 120},
    {-30, 80},
    {-120, 120},
    {-120, 120},
    {-90, 60},
    {-90, 60},
    {-90, 90},
    {-90, 90},
    {-200, 80},
    {-200, 80},
    {-80, 200},
    {-80, 200},
    {-80, 200},
    {-80, 200},
    {-70, 200},
    {-80, 200},
};
int previousAng[DOF];
#ifdef X_LEG
int currentAng[DOF] = {-30, -80, -45, 0,
                       0, 0, 0, 0,
                       75, 75, -75, -75,
                       -55, -55, 55, 55};
#else
int currentAng[DOF] = {-30, -80, -45, 0,
                       0, 0, 0, 0,
                       75, 75, 75, 75,
                       -55, -55, -55, -55};
#endif
int zeroPosition[DOF] = {};
int calibratedZeroPosition[DOF] = {};

int8_t servoCalib[DOF] = {0, 0, 0, 0,
                          0, 0, 0, 0,
                          0, 0, 0, 0,
                          0, 0, 0, 0};

int16_t imuOffset[9] = {0, 0, 0,
                        0, 0, 0,
                        0, 0, 0};

// abbreviations
#define PT(s) Serial.print(s) // makes life easier
#define PTL(s) Serial.println(s)
#define PTF(s) Serial.print(F(s)) // trade flash memory for dynamic memory with F() function
#define PTLF(s) Serial.println(F(s))
#define PRINT_SKILL_DATA

//
// template <typename T> int8_t sign(T val) {
//  return (T(0) < val) - (val < T(0));
//}

void printRange(int r0 = 0, int r1 = 0)
{
  if (r1 == 0)
    for (byte i = 0; i < r0; i++)
    {
      PT(i);
      PT('\t');
    }
  else
    for (byte i = r0; i < r1; i++)
    {
      PT(i);
      PT('\t');
    }
  PTL();
}

String range2String(int r0 = 0, int r1 = 0)
{
  String temp = "";
  if (r1 == 0)
    for (byte i = 0; i < r0; i++)
    {
      temp += i;
      temp += '\t';
    }
  else
    for (byte i = r0; i < r1; i++)
    {
      temp += i;
      temp += '\t';
    }
  return temp;
}

template <typename T>
void printList(T *arr, byte len = DOF)
{
  String temp = "";
  for (byte i = 0; i < len; i++)
  {
    temp += String(int(arr[i]));
    temp += ",\t";
    // PT((T)(arr[i]));
    // PT('\t');
  }
  PTL(temp);
}

template <typename T>
String list2String(T *arr, byte len = DOF)
{
  String temp = "";
  for (byte i = 0; i < len; i++)
  {
    temp += String(int(arr[i])); // String(int(arr[i]));
    temp += ",\t";
    // PT((T)(arr[i]));
    // PT('\t');
  }
  return temp;
}

// float sign(float value) {
//   return ((value > 0) - (value < 0));
// }
template <typename T>
void printTable(T *list)
{
  printRange(0, DOF);
  printList(list, DOF);
}

#include "sound.h"
#include "I2cEEPROM.h"
#include "bleUart.h"
#ifdef GYRO_PIN
#include "imu.h"
#endif
#include "espServo.h"
#ifdef CAMERA
#include "camera.h"
#endif
#include "randomMind.h"
#include "io.h"
#include "motion.h"
#include "skill.h"
#include "reaction.h"
#include "qualityAssurance.h"
