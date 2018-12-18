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


namespace Temp {
// Analog pins for 3 way switch: 14 for up regulation
// 15 for down regulation of targetTemp.
// INVERTED, connect to GND not 5V
const int tUpPin = 18, tDownPin = 19;
unsigned long lastTempInput = 0;

// Declaration of temp variables
double currentTemp = 0;
double targetTemp = 0;
TempHolder target(0);  // Temp holder object for the target temperature
bool udpateTemp();
void manuallyAdjustTemp();

// Declaration for temperature probe
OneWire oneWire(4); // GPIO pin number 4 is used for 1W interface by default
DallasTemperature currentTempProbe(&oneWire);
// DeviceAddress is specific to each unique DS18B20 probe
// Find yours with the exapmles here:
// https://github.com/milesburton/Arduino-Temperature-Control-Library/tree/master/examples
DeviceAddress insideThermometer = {0x28, 0xFF, 0xC5, 0xDC, 0x80, 0x14, 0x02, 0xA2};
unsigned long lastTempUpdate = 0;
}

bool Temp::udpateTemp()
// Reads temperature from DS18B20 at set intervals
{
  if ((millis() - Temp::lastTempUpdate) > TEMP_READ_DELAY)
  {
    float currentTempFloat = Temp::currentTempProbe.getTempC(Temp::insideThermometer);
    Temp::currentTemp = double(currentTempFloat);
    Temp::targetTemp = Temp::target.getTemp();
    Temp::lastTempUpdate = millis();
    Temp::currentTempProbe.requestTemperatures();
    return true;
  }
  return false;
}

void Temp::manuallyAdjustTemp()
// Checks for input from 3 way switch that manually adjusts
// target temperature and updates TempHolder target
{
  if ((millis() - Temp::lastTempInput) > TEMP_INPUT_DELAY)
  {   
    if (digitalRead(Temp::tUpPin))
    {
      target.adjustTemp(1);
    }
    if (digitalRead(Temp::tDownPin))
    {
      target.adjustTemp(-1);
    }
    Temp::lastTempInput = millis();
  }
}

namespace PID {
// Declaration for PID controller
// heaterPin is output for heater relay
const int heaterPin = 10; 
double outputVal = 0;
double pulsePercent();
double Kp = 10, Ki = 0, Kd = 0;  // These will need adjusting
AutoPID myPID(&Temp::currentTemp, &Temp::targetTemp, &outputVal, OUTPUT_MIN, OUTPUT_MAX, Kp, Ki, Kd);
}

double PID::pulsePercent()
// Converts the analog bit value of the pulsewidth to a percentage for display
{
  double pulsePercent = ((PID::outputVal/255)*100);
  return pulsePercent;
}

namespace Liquid {
// Use LiquidCrystal if you're using the original LiquidCrystal
// library and have soldered wires directly from your LCD
// const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Use LiquidCrystal_I2C if you're using a serial converter chip
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Declarations for LCD 
void updateLCD();
unsigned long lastLCDUpdate = 0;
int stringStart = 0;
int stringStop = 0;
int scrollCursor = 20;
}

void Liquid::updateLCD ()
{
  if ((millis() - Liquid::lastLCDUpdate) > LCD_UPDATE_DELAY)
    {
      // String conversion and processing numbers
      double tTemp = Temp::target.getTemp();
      String targetTempString = String(tTemp, 1);
      String fullTargetTempString = String("Target : " + targetTempString + " \xDF""C");
      double cTemp = Temp::currentTemp;
      String currentTempString = String(cTemp, 1);
      String fullCurrentTempString = String("Current: " + currentTempString + " \xDF""C");
      double pulse = PID::pulsePercent();
      String pulsePercentString = String(pulse, 0);
      String outputValString = String(PID::outputVal, 0);
      String fullPulsePercentString = String("Pulse  : " + pulsePercentString + "% (" + outputValString + ")");
      String sKp = String(PID::Kp);
      String sKi = String(PID::Ki);
      String sKd = String(PID::Kd);
      String sKPID = String("Kp: " + sKp + " Ki: " + sKi + " Kd: " + sKd);
      
      // Writing to LCD screen, bottom row scrolls if the line length is wider than screen
      Liquid::lcd.clear();
      Liquid::lcd.setCursor(0, 0), Liquid::lcd.print(fullTargetTempString);
      Liquid::lcd.setCursor(0, 1), Liquid::lcd.print(fullCurrentTempString);
      Liquid::lcd.setCursor(0, 2), Liquid::lcd.print(fullPulsePercentString);
      if (sKPID.length() > 20)
      {
        Liquid::lcd.setCursor(Liquid::scrollCursor, 3), Liquid::lcd.print(sKPID.substring(Liquid::stringStart, Liquid::stringStop));
        if (Liquid::stringStart == 0 && Liquid::scrollCursor > 0)
        {
          Liquid::scrollCursor--;
          Liquid::stringStop++;
        }
        else if (Liquid::stringStart == Liquid::stringStop)
        {
          Liquid::stringStart = Liquid::stringStop = 0;
          Liquid::scrollCursor = 20;
        }
        else if (Liquid::stringStop == sKPID.length() && Liquid::scrollCursor == 0)
        {
          Liquid::stringStart++;
        }
        else
        {
          Liquid::stringStart++;
          Liquid::stringStop++;
        }
      }
      else
      {
        Liquid::lcd.setCursor(0, 3), Liquid::lcd.print(sKPID);
      }
    Liquid::lastLCDUpdate = millis();
  }
}

void setup()
{
  // Runs once at startup
  pinMode(Temp::tUpPin, INPUT_PULLUP);
  pinMode(Temp::tDownPin, INPUT_PULLUP);
  pinMode(PID::heaterPin, OUTPUT);
  Liquid::lcd.begin(20, 4);
  Temp::currentTempProbe.begin();
  Temp::currentTempProbe.requestTemperatures();
  while (!Temp::udpateTemp()) {}
  Temp::currentTemp = Temp::currentTempProbe.getTempC(Temp::insideThermometer);
  Temp::targetTemp = Temp::target.getTemp();
  Liquid::lcd.setCursor(0, 0), Liquid::lcd.print("Temp probe init OK");
  PID::myPID.setBangBang(10);  // min/max at +-10 degrees from targetTemp
  PID::myPID.setTimeStep(1000);
  Liquid::lcd.setCursor(0, 1), Liquid::lcd.print("PID: BangBang set at");
  Liquid::lcd.setCursor(0, 2), Liquid::lcd.print("+/- 10\xDF""C from target");
  Liquid::lcd.setCursor(0, 3), Liquid::lcd.print("System operational!");
  delay(3000);
}

void loop()
{
  // Runs continuously
  Temp::manuallyAdjustTemp();
  Temp::udpateTemp();
  PID::myPID.run();
  analogWrite(10, PID::outputVal);
  Liquid::updateLCD();
}
