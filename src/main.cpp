#include <Arduino.h>
#include <avr/wdt.h>

#include <config.h>
#include <direction.h>

// define max message length
#define SERIAL_MAX_LENGTH 100

#ifdef LCD_ENABLED
  #include <LiquidCrystal.h>

  // setup LCD pins
  LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_EN, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7, PIN_LCD_D8);
#endif

void serialWrite(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  char output[SERIAL_MAX_LENGTH];
  //fmt = sprintf("%s", fmt);
  vsnprintf(output, sizeof output, fmt, args);
  Serial.print(output);
  Serial.println();
  Serial.flush();
  va_end(args);
}

// include eeprom for writing out calibration
#include <EEPROM.h>

// variables to hold the calibration info
uint16_t azimuthMinCalibration;
uint16_t azimuthMaxCalibration;
uint16_t elevationhMinCalibration;
uint16_t elevationMaxCalibration;

// hold the current state
uint16_t azimuth;
uint16_t elevation;
uint16_t azimuthTarget;
uint16_t elevationTarget;

// demo variables
#ifdef DEMO_MODE
  uint16_t demoAzimuth = 180;
  uint16_t demoElevation = 90;
#endif

// calibrates the minimum reading of the azimuth input
void calibrateAzimuthMin()
{
  // take the current reading, and write it to eeprom
  uint16_t reading = analogRead(PIN_POS_AZI);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(0, lower);
  EEPROM.write(1, upper);

  azimuthMinCalibration = reading;

  #ifdef DEBUG
    serialWrite("Azi Min Cal Set: %u", reading);
  #endif
}

// reads the calibration from eeprom
uint16_t readAzimuthMinCalibration()
{
  byte lower = EEPROM.read(0);
  byte upper = EEPROM.read(1);

  uint16_t output;
  return (upper << 8) | lower;
}

// calibrates the maximum reading of the azimuth input
void calibrateAzimuthMax()
{
  // take the current reading, and write it to eeprom
  uint16_t reading = analogRead(PIN_POS_AZI);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(2, lower);
  EEPROM.write(3, upper);

  azimuthMaxCalibration = reading;

  #ifdef DEBUG
    serialWrite("Azi Max Cal Set: %u", reading);
  #endif
}

// reads the calibration from eeprom
uint16_t readAzimuthMaxCalibration()
{
  byte lower = EEPROM.read(2);
  byte upper = EEPROM.read(3);

  uint16_t output;
  return (upper << 8) | lower;
}

// calibrates the minimum reading of the elevation
void calibrateElevationhMin()
{
  // take the current reading, and write it to eeprom
  uint16_t reading = analogRead(PIN_POS_ELE);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(4, lower);
  EEPROM.write(5, upper);

  elevationhMinCalibration = reading;

  #ifdef DEBUG
    serialWrite("Ele Min Cal Set: %u", reading);
  #endif
}

// reads the calibration from eeprom
uint16_t readElevationhMinCalibration()
{
  byte lower = EEPROM.read(4);
  byte upper = EEPROM.read(5);

  uint16_t output;
  return (upper << 8) | lower;
}

// calibrates the maximum reading of the azimuth input
void calibrateElevationMax()
{
  // take the current reading, and write it to eeprom
  uint16_t reading = analogRead(PIN_POS_ELE);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(6, lower);
  EEPROM.write(7, upper);

  elevationMaxCalibration = reading;

  #ifdef DEBUG
    serialWrite("Ele Max Cal Set: %u", reading);
  #endif
}

// reads the calibration from eeprom
uint16_t readElevationMaxCalibration()
{
  byte lower = EEPROM.read(6);
  byte upper = EEPROM.read(7);

  uint16_t output;
  return (upper << 8) | lower;
}

void readCalibrationInfo()
{
  azimuthMinCalibration = readAzimuthMinCalibration();
  azimuthMaxCalibration = readAzimuthMaxCalibration();
  elevationhMinCalibration = readElevationhMinCalibration();
  elevationMaxCalibration = readElevationMaxCalibration();

  #ifdef DEBUG
    serialWrite("Calibration: ");
    serialWrite("Azi Min: %4d", azimuthMinCalibration);
    serialWrite("Azi Max: %4d", azimuthMaxCalibration);
    serialWrite("Eli Min: %4d", elevationhMinCalibration);
    serialWrite("Eli Max: %4d", elevationMaxCalibration);
  #endif

  #ifdef LCD_ENABLED
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("A Cal: %u %u", azimuthMinCalibration, azimuthMaxCalibration);
    lcd.setCursor(0, 1);
    lcd.printf("E Cal: %u %u", elevationhMinCalibration, elevationMaxCalibration);
    delay(2000);
  #endif
};

float float_map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t readAzimuth()
{
  #ifdef DEMO_MODE
    return demoAzimuth;
  #else
    // due to some impedence, we need to read twice
    (void)analogRead(PIN_POS_AZI);
    delay(5);

    // 2V = 0 deg, 4.5V = 450 deg
    uint16_t readValue = analogRead(PIN_POS_AZI);

    if (readValue <= azimuthMinCalibration)
      return 0;
    if (readValue > -azimuthMaxCalibration)
      return 450;

    return (uint16_t)map(readValue, azimuthMinCalibration, azimuthMaxCalibration, 0, 450);
  #endif
}

uint16_t readElevation()
{
  #ifdef DEMO_MODE
    return demoElevation;
  #else
    // due to some impedence, we need to read twice
    (void)analogRead(PIN_POS_ELE);
    delay(5);

    // 2V = 0 deg, 4.5V = 180 deg
    uint16_t readValue = analogRead(PIN_POS_ELE);

    if (readValue <= elevationhMinCalibration)
      return 0;
    if (readValue >= elevationMaxCalibration)
      return 180;

    return (uint16_t)map(readValue, azimuthMinCalibration, azimuthMaxCalibration, 0, 180);
  #endif
}

void serialCommandHandler(char command[SERIAL_MAX_LENGTH])
{
  #ifdef DEBUG
    serialWrite("Command Received: %s", command);
  #endif

  switch (command[0])
  {
    case 'B':                                   // report elevation
      serialWrite("+0%03u", elevation); // +0000
      break;

    case 'C': // report azimuth (elevation)
      if (command[1] == '2')
      {                                                                 // report azimuth and elevation
        serialWrite("+0%03u+0%03u", azimuth, elevation); // +0000 +0000
      }
      else
      {                                         // report azimith
        serialWrite("+0%03u", azimuth); // +0000
      }
      break;

    case 'M': // set target azmuth
      static uint16_t inputAzimith;
      sscanf(command, "M%u", &inputAzimith);
      azimuthTarget = inputAzimith;
      Serial.print('\r');
      #ifdef DEBUG
          serialWrite("Directing to: %u", inputAzimith);
      #endif
      break;

    case 'W': // set target azimuth and elevation
      // static uint16_t inputAzimuth;
      // static uint16_t inputElevation;
      sscanf(command, "W%u %u", &azimuthTarget, &elevationTarget);
      // elevationTarget = inputElevation;
      // azimuthTarget = inputAzimith;
      Serial.print('\r');
      #ifdef DEBUG
          serialWrite("Directing to: %u, %u", azimuthTarget, elevationTarget);
      #endif
      break;

    case 'S': // stop
      azimuthTarget = azimuth;
      elevationTarget = elevation;
      digitalWrite(PIN_ROT_CW, LOW);
      digitalWrite(PIN_ROT_CCW, LOW);
      digitalWrite(PIN_ROT_UP, LOW);
      digitalWrite(PIN_ROT_DOWN, LOW);
      Serial.print('\r');
      break;

    case 'O': // calibrate min value
      if (command[1] == '2')
      { // calibrate elevation
        calibrateElevationhMin();
      }
      else
      { // calibrate azimuth
        calibrateAzimuthMin();
      }
      break;
    case 'F':
      if (command[1] == '2')
      { // calibrate elevation
        calibrateElevationMax();
      }
      else
      { // calibrate azimuth
        calibrateAzimuthMax();
      }
      Serial.print('\r');
      break;
  }
}

void setupPins()
{
  // setup rotator pins
  pinMode(PIN_ROT_CCW, OUTPUT);
  pinMode(PIN_ROT_CW, OUTPUT);
  pinMode(PIN_ROT_UP, OUTPUT);
  pinMode(PIN_ROT_DOWN, OUTPUT);
  pinMode(PIN_POS_ELE, INPUT);
  pinMode(PIN_POS_AZI, INPUT);

  // setup led pin
  pinMode(PIN_LED, OUTPUT);
}

void setup()
{
// setup lcd
#ifdef LCD_ENABLED
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("M3SUJ");
  lcd.setCursor(1, 1);
  lcd.print("SatController");

// if in demo mode, show it on the screen so we know why
#ifdef DEMO_MODE
  delay(1000);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("**DEMO**");
  lcd.setCursor(4, 1);
  lcd.print("**MODE**");
  delay(1500);
#endif

#endif

  // setup serial
  Serial.begin(SERIAL_BAUD);

#ifdef DEBUG
  serialWrite("Started up..");
#endif

  readCalibrationInfo();
  setupPins();

  // wait before doing anything else
  delay(1000);

  // enable the watchdog
  wdt_enable(WDTO_2S);
}

void loop()
{
  // let the watchdog know we're here
  wdt_reset();

  // read the current values
  azimuth = readAzimuth();
  elevation = readElevation();

  #ifdef DEBUG
    serialWrite("Target: %u %u", azimuthTarget, elevationTarget);
    serialWrite("Current: %u %u", azimuth, elevation);
  #endif

  if (azimuthTarget < azimuth)
  {
    digitalWrite(PIN_ROT_CW, LOW);
    digitalWrite(PIN_ROT_CCW, HIGH);

    // fake some rotation when in demo mode
    #ifdef DEMO_MODE
      demoAzimuth = demoAzimuth - 1;
    #endif
  }

  if (azimuthTarget > azimuth)
  {
    digitalWrite(PIN_ROT_CW, HIGH);
    digitalWrite(PIN_ROT_CCW, LOW);

    // fake some rotation when in demo mode
    #ifdef DEMO_MODE
      demoAzimuth = demoAzimuth + 1;
    #endif
  }

  if (azimuthTarget == azimuth)
  {
    digitalWrite(PIN_ROT_CW, LOW);
    digitalWrite(PIN_ROT_CCW, LOW);
  }

  if (elevationTarget < elevation)
  {
    digitalWrite(PIN_ROT_DOWN, HIGH);
    digitalWrite(PIN_ROT_UP, LOW);

    // fake some rotation when in demo mode
    #ifdef DEMO_MODE
      demoElevation = demoElevation - 1;
    #endif
    }

  if (elevationTarget > elevation)
  {
    digitalWrite(PIN_ROT_DOWN, LOW);
    digitalWrite(PIN_ROT_UP, HIGH);

    // fake some rotation when in demo mode
    #ifdef DEMO_MODE
      demoElevation = demoElevation + 1;
    #endif
  }

  if (elevationTarget == elevation)
  {
    digitalWrite(PIN_ROT_DOWN, LOW);
    digitalWrite(PIN_ROT_UP, LOW);
  }

  static char message[SERIAL_MAX_LENGTH];
  static uint8_t messagePos = 0;

  while (Serial.available() > 0)
  {
    char inByte = Serial.read();

    // if in debugging, print out the characters as they're typed
    #ifdef DEBUG
      Serial.write(inByte);
    #endif

    if (inByte != '\n' && inByte != '\r')
    {
      message[messagePos] = inByte;
      messagePos++;
    }
    else
    {
      message[messagePos] = '\0';
      serialCommandHandler(message);
      messagePos = 0;
      memset(message, 0, SERIAL_MAX_LENGTH);
    }
  }

  #ifdef LCD_ENABLED
    // this is to show if we've setup the lcd for the first time
    static bool lcd_setup;

    if (!lcd_setup)
    {
      // when setting up the lcd for the first time, clear the screen, then write the labels
      lcd.clear();
      delay(10);
      lcd.setCursor(0, 0);
      lcd.print("Azi: -");
      lcd.setCursor(0, 1);
      lcd.print("Eli: - ");
      lcd_setup = true;
      delay(1000);
    }

    // just write the azimuth/elevation to the parts of the screen
    lcd.setCursor(5, 0);

    lcd.printf("% 3d % 3s", azimuth, azimuth_to_direction(azimuth));
    lcd.setCursor(5, 1);
    lcd.printf("% 3d", elevation);

  // if debugging, also print the raw ADC readings on the far right of the screen
  // theres some weird bug that means when we do this "Azi: " isnt displayed... only does it in debug
    #ifdef DEBUG
      lcd.setCursor(12, 0);
      lcd.printf("% 4d", analogRead(PIN_POS_AZI));
      lcd.setCursor(12, 1);
      lcd.printf("% 4d", analogRead(PIN_POS_ELE));
    #endif
  #endif

  delay(500);
}