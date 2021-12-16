#include <Arduino.h>
#include <config.h>

// define max message length
#define SERIAL_MAX_LENGTH 100

// ADC steps to volts
#define VOLTAGE_MULTIPLIER 5 / 1000

#ifdef LCD_ENABLED
#include <LiquidCrystal.h>

// setup LCD pins
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_EN, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7, PIN_LCD_D8);
#endif

// include eeprom for writing out calibration
#include <EEPROM.h>


// variables to hold the calibration info
int azimuthMinCalibration;
int azimuthMaxCalibration;
int elevationhMinCalibration;
int elevationMaxCalibration;

// calibrates the minimum reading of the azimuth input
void calibrateAzimithMin() {
  // take the current reading, and write it to eeprom
  int reading = analogRead(PIN_POS_AZI);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(0, lower);
  EEPROM.write(1, upper);

  azimuthMinCalibration = reading;
}

// reads the calibration from eeprom
int readAzimuthMinCalibration() {
  byte lower = EEPROM.read(0);
  byte upper = EEPROM.read(1);

  int output;
  return (upper << 8) | lower;
}

// calibrates the maximum reading of the azimuth input
void calibrateAzimithMax() {
  // take the current reading, and write it to eeprom
  int reading = analogRead(PIN_POS_AZI);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(2, lower);
  EEPROM.write(3, upper);

  azimuthMaxCalibration = reading;
}

// reads the calibration from eeprom
int readAzimuthMaxCalibration() {
  byte lower = EEPROM.read(2);
  byte upper = EEPROM.read(3);

  int output;
  return (upper << 8) | lower;
}

// calibrates the minimum reading of the elevation
void calibrateElevationhMin() {
  // take the current reading, and write it to eeprom
  int reading = analogRead(PIN_POS_ELE);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(4, lower);
  EEPROM.write(5, upper);

  elevationhMinCalibration = reading;
}

// reads the calibration from eeprom
int readElevationhMinCalibration() {
  byte lower = EEPROM.read(4);
  byte upper = EEPROM.read(5);

  int output;
  return (upper << 8) | lower;
}
 
// calibrates the maximum reading of the azimuth input
void calibrateElevationMax() {
  // take the current reading, and write it to eeprom
  int reading = analogRead(PIN_POS_ELE);

  // int is 2 bits, and EEPROM.write stores 1 bit at a time, so we need to split them up
  byte lower = reading;
  byte upper = reading >> 8;
  EEPROM.write(6, lower);
  EEPROM.write(7, upper);

  elevationMaxCalibration = reading;
}

// reads the calibration from eeprom
int readElevationMaxCalibration() {
  byte lower = EEPROM.read(6);
  byte upper = EEPROM.read(7);

  int output;
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
    serialWrite("Eli Max: %4d", elevationMaxCalibration;
  #endif
};


int readAzimuth()
{
  // due to some impedence, we need to read twice
  (void)analogRead(PIN_POS_AZI);
  delay(5);

  // 2V = 0 deg, 4.5V = 450 deg
  int readValue = analogRead(PIN_POS_AZI);


#ifdef DEMO_MODE
  return 123;
#else
  if ((readValue * VOLTAGE_MULTIPLIER) < 2)
  {
    // return 0 if the voltage is under 2 volts.
    return 0;
  }
  else
  {
    return (int)((readValue * VOLTAGE_MULTIPLIER) - 2) * 180;
  }
#endif
}

int readElevation()
{
  // due to some impedence, we need to read twice
  (void)analogRead(PIN_POS_ELE);
  delay(5);

  // 2V = 0 deg, 4.5V = 180 deg
  int readValue = analogRead(PIN_POS_ELE);

#ifdef DEMO_MODE
  return 90;
#else
  if ((readValue * VOLTAGE_MULTIPLIER) < 2)
  {
    return 0;
  }
  else
  {
    return (int)((readValue * VOLTAGE_MULTIPLIER) - 2) * 72;
  }
#endif
}

void serialWrite(const char* fmt, ...) 
{
  va_list args; 
  va_start(args, fmt);
  char output[SERIAL_MAX_LENGTH];
  fmt = strcat(fmt, "\r");
  vsnprintf(output, sizeof output, fmt, args);
  Serial.write(output);
  va_end(args);
}

bool commandMatch(char stringA[], char stringB[])
{
  return strcmp(stringA, stringB) == 0 ? true : false;
}

void serialCommandHandler(char command[SERIAL_MAX_LENGTH])
{
  #ifdef DEBUG
  serialWrite("Command Received: %s", command);
  #endif

  if (commandMatch(command, "B")) // Report elevation
  {
    // +0000
    serialWrite("+0%03d", readElevation());
  }
  else if (commandMatch(command, "C")) // Report Azimuth
  {
    // +0000
    serialWrite("+0%03d", readAzimuth());
  }
  else if (commandMatch(command, "C2")) // Report azimuth and elevation
  {
    // +0000 +0000
    serialWrite("+0%03d +0%03d", readAzimuth(), readElevation());
  } 
  else if (commandMatch(command, "M")) // move to azimith
  {

  } else if (commandMatch(command, "W")) // move to azimuth and elevation
  {

  }
}

void setup()
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

// setup lcd
#ifdef LCD_ENABLED
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("M3SUJ");
  lcd.setCursor(1, 1);
  lcd.print("SatController");
#endif

  // setup serial
  Serial.begin(SERIAL_BAUD);
  
  #ifdef DEBUG
    serialWrite("Started up..");
  #endif

  // wait before doing anything else
  delay(1000);
}

void loop()
{ 
  while (Serial.available() > 0)
  {
    static char message[SERIAL_MAX_LENGTH];
    static unsigned int messagePos = 0;

    char inByte = Serial.read();

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

  if (!lcd_setup) {
    // when setting up the lcd for the first time, clear the screen, then write the labels
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Azi: -");
    lcd.setCursor(0, 1);
    lcd.print("Eli: - ");
    lcd_setup = true;
    delay(10);
  }

  // just write the azimuth/elevation to the parts of the screen
  lcd.setCursor(5, 0);
  lcd.print(String(readAzimuth()));
  lcd.setCursor(5, 1);
  lcd.print(String(readElevation()));

  // if debugging, also print the raw ADC readings on the far right of the screen
  // theres some weird bug that means when we do this "Azi: " isnt displayed... only does it in debug
  #ifdef DEBUG
    lcd.setCursor(12, 0);
    lcd.print(analogRead(PIN_POS_AZI));
    lcd.setCursor(12, 1); 
    lcd.print(analogRead(PIN_POS_ELE));
  #endif
#endif


  // blink the led to show im alive
  // digitalWrite(PIN_LED, HIGH);
  // delay(1000);
  // digitalWrite(PIN_LED, LOW);
  // delay(1000);

  delay(500);
}