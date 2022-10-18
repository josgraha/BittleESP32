float mean(float *a, int n)
{
  float sum = 0;
  for (int i = 0; i < n; i++)
    sum += a[i];
  return sum / n;
}

float sDev(float *a, float m, int n)
{
  float sum = 0;
  for (int i = 0; i < n; i++)
    sum += (a[i] - m) * (a[i] - m);
  return sqrt(sum / n);
}
byte mpuGood[] = {12, 16, 19,
                  4, 4, 4};
byte mpuBad[] = {19, 17, 16, 14, 12,
                 16, 16, 16, 16, 16};

#define MEAN_THRESHOLD 0.2
#define STD_THRESHOLD 0.02

void testMPU()
{
  PTL("\nIMU test: both mean and standard deviation should be small on Pitch and Roll axis\n");
  delay(1000);
  int count = 100;
  float **history = new float *[2];
  for (int a = 0; a < 2; a++)
    history[a] = new float[count];
  for (int t = 0; t < count; t++)
  {
    delay(5);
    read_IMU();
    print6Axis();
    for (int a = 0; a < 2; a++)
      history[a][t] = ypr[a + 1];
  }
  String axis[] = {"Pitch ", "Roll  "};
  for (int a = 0; a < 2; a++)
  {
    float m = mean(history[a], count);
    float dev = sDev(history[a], m, count);
    PT(axis[a]);
    PT("(in degrees)\tmean: ");
    PT(m);
    PT("\tstandard deviation: ");
    PT(dev);
    if (fabs(m) > MEAN_THRESHOLD || dev > STD_THRESHOLD)
    {
      PTL("\tFail!");
      while (1)
      {
        playMelody(mpuBad, sizeof(mpuBad) / 2);
        delay(500);
      }
    }
    else
    {
      PTL("\tPass!");
      playMelody(mpuGood, sizeof(mpuGood) / 2);
    }
  }
  delay(100);
  for (int a = 0; a < 2; a++)
    delete[] history[a];
  delete[] history;
};

void QA()
{
  if (newBoard)
  {
#ifndef AUTO_INIT
    PTL("Run factory quality assurance program? (Y/n)");
    while (!Serial.available())
      ;
    char choice = Serial.read();
    PTL(choice);
    if (choice != 'Y' && choice != 'y')
      return;
#endif
    testMPU();
    // tests...
    PTL("\nServo test: all servos should rotate and in sync\n");
    loadBySkillName("ts"); // test EEPROM
    while (1)
    {
      skill->perform();
      PTL("Pass");
      playMelody(melodyIRpass, sizeof(melodyIRpass) / 2);
      break;
    }
  }
}