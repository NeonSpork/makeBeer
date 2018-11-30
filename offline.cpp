#include <TempHolder.hpp>
#include <LiquidCrystal_I2C.h>
#include <AutoPID.h>
#include <DallasTemperature.h>
// #include <OneWire.h>
// #include <Wire.h>

#define LCD_UPDATE_DELAY 500
#define TEMP_READ_DELAY 800
#define TEMP_INPUT_DELAY 250
#define OUTPUT_MIN 0
#define OUTPUT_MAX 255

// Use LiquidCrystal if you're using the original LiquidCrystal
// library and have soldered wires directly from your LCD
// const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Use LiquidCrystal_I2C if you're using a serial converter chip
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Declarations for LCD 
void updateLCD();
unsigned long lastLCDUpdate = 0;
int stringStart, stringStop = 0;
int scrollCursor = 20;

// Analog pins for 3 way switch: 14 for up regulation
// 15 for down regulation of targetTemp.
// INVERTED, connect to GND not 5V
// heaterPin is output for heater relay
const int tUpPin = 18, tDownPin = 19, heaterPin = 10; 
unsigned long lastTempInput = 0;

// Declaration of temp variables
double currentTemp = 0;
double targetTemp = 0;
TempHolder target(0);  // Temp holder object for the target temperature
bool udpateTemp();
void manuallyAdjustTemp();

// Declaration for temperature probe
const int dTemp = 4;
OneWire oneWire(dTemp);
DallasTemperature currentTempProbe(&oneWire);
// DeviceAddress is specific to each unique DS18B20 probe
// Find yours with the exapmles here:
// https://github.com/milesburton/Arduino-Temperature-Control-Library/tree/master/examples
DeviceAddress insideThermometer = {0x28, 0xFF, 0xC5, 0xDC, 0x80, 0x14, 0x02, 0xA2};

// Declaration for PID controller
unsigned long lastTempUpdate;
double outputVal = 0;
double pulsePercent();
double Kp = 10, Ki = 0, Kd = 0;  // These will need adjusting
AutoPID myPID(&currentTemp, &targetTemp, &outputVal, OUTPUT_MIN, OUTPUT_MAX, Kp, Ki, Kd);

double pulsePercent()
// Converts the analog bit value of the pulsewidth to a percentage for display
{
  double pulsePercent = ((outputVal/255)*100);
  return pulsePercent;
}

bool udpateTemp()
// Reads temperature from DS18B20 at set intervals
{
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY)
  {
    float currentTempFloat = currentTempProbe.getTempC(insideThermometer);
    currentTemp = double(currentTempFloat);
    targetTemp = target.getTemp();
    lastTempUpdate = millis();
    currentTempProbe.requestTemperatures();
    return true;
  }
  return false;
}

void manuallyAdjustTemp()
// Checks for input from 3 way switch that manually adjusts
// target temperature and updates TempHolder target
{
  if ((millis() - lastTempInput) > TEMP_INPUT_DELAY)
  {   
    if (digitalRead(tUpPin))
    {
      target.adjustTemp(1);
    }
    if (digitalRead(tDownPin))
    {
      target.adjustTemp(-1);
    }
    lastTempInput = millis();
  }
}

void updateLCD ()
{
  if ((millis() - lastLCDUpdate) > LCD_UPDATE_DELAY)
    {
      // String conversion and processing numbers
      double tTemp = target.getTemp();
      String targetTempString = String(tTemp, 1);
      String fullTargetTempString = String("Target : " + targetTempString + " \xDF""C");
      double cTemp = currentTemp;
      String currentTempString = String(cTemp, 1);
      String fullCurrentTempString = String("Current: " + currentTempString + " \xDF""C");
      double pulse = pulsePercent();
      String pulsePercentString = String(pulse, 0);
      String outputValString = String(outputVal, 0);
      String fullPulsePercentString = String("Pulse  : " + pulsePercentString + "% (" + outputValString + ")");
      String KpString = String(Kp);
      String KiString = String(Ki);
      String KdString = String(Kd);
      String KPIDString = String("Kp: " + KpString + " Ki: " + KiString + " Kd: " + KdString);
      
      // Writing to LCD screen, bottom row scrolls if the line length is wider than screen
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print(fullTargetTempString);
      lcd.setCursor(0, 1), lcd.print(fullCurrentTempString);
      lcd.setCursor(0, 2), lcd.print(fullPulsePercentString);
      if (KPIDString.length() > 20)
      {
        lcd.setCursor(scrollCursor, 3), lcd.print(KPIDString.substring(stringStart, stringStop));
        if (stringStart == 0 && scrollCursor > 0)
        {
          scrollCursor--;
          stringStop++;
        }
        else if (stringStart == stringStop)
        {
          stringStart = stringStop = 0;
          scrollCursor = 20;
        }
        else if (stringStop == KPIDString.length() && scrollCursor == 0)
        {
          stringStart++;
        }
        else
        {
          stringStart++;
          stringStop++;
        }
      }
      else
      {
        lcd.setCursor(0, 3), lcd.print(KPIDString);
      }
    lastLCDUpdate = millis();
  }
}

void setup()
{
  // Runs once at startup
  pinMode(tUpPin, INPUT_PULLUP);
  pinMode(tDownPin, INPUT_PULLUP);
  pinMode(heaterPin, OUTPUT);
  lcd.begin(20, 4);
  currentTempProbe.begin();
  currentTempProbe.requestTemperatures();
  while (!udpateTemp()) {}
  currentTemp = currentTempProbe.getTempC(insideThermometer);
  targetTemp = target.getTemp();
  lcd.setCursor(0, 0), lcd.print("Temp probe init OK");
  myPID.setBangBang(10);  // min/max at +-10 degrees from targetTemp
  myPID.setTimeStep(1000);
  lcd.setCursor(0, 1), lcd.print("PID: BangBang set at");
  lcd.setCursor(0, 2), lcd.print("+/- 10\xDF""C from target");
  lcd.setCursor(0, 3), lcd.print("System operational!");
  delay(3000);
}

void loop()
{
  // Runs continuously
  manuallyAdjustTemp();
  udpateTemp();
  myPID.run();
  analogWrite(10, outputVal);
  updateLCD();
}
