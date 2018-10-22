#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <AutoPID.h>

#define LCD_UPDATE_DELAY 300
#define TEMP_READ_DELAY 800
#define OUTPUT_MIN 0
#define OUTPUT_MAX 255

// Declarations for hardware
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2, dTemp = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void updateLCD ();
unsigned int lastLCDUpdate;
int stringStart, stringStop = 0;
int scrollCursor = 20;

// Template declaration for temperature holder
float targetTemp = 0, currentTemp;
template <typename T1>
class TempHolder
{
private:
    T1 mTemp;
public:
    explicit TempHolder (T1& val1)
    {
        mTemp = val1;
    };
    T1& getTemp ()
    {
        return mTemp;
    };
    T1& setTemp (T1& x)
    {
        mTemp = x;
        return mTemp;
    };
    T1& adjustTemp (T1& x)
    {
        mTemp += x;
        if (mTemp<0)
        {
            mTemp = 0;
        }
        return mTemp;
    };
};
// Specialization of temperature templates
TempHolder <float> target(targetTemp);
TempHolder <float> current(currentTemp);

double currentTempPID = current.getTemp();
double targetTempPID = target.getTemp();

// Declaration for temp and PID
OneWire oneWire(dTemp);
DallasTemperature probe(&oneWire);
unsigned int lastTempUpdate;
double outputVal;
double Kp = 0.12, Ki = 0.0003, Kd = 0;  // These will probably need adjusting
AutoPID myPID(&currentTempPID, &targetTempPID, &outputVal, OUTPUT_MIN, OUTPUT_MAX, Kp, Ki, Kd);

// Analog pins for 3 way switch: 14 for up regulation
// 15 for down regulation of targetTemp.
// INVERTED, connect to GND not 5V
// heaterPin is output for heater relay
const int tUpPin = 18, tDownPin = 19, heaterPin = 10; 

// Declarations for PID and heater
int pulse = 0;
int pulsePercent();

void setup()
{
    pinMode(tUpPin, INPUT_PULLUP);
    pinMode(tDownPin, INPUT_PULLUP);
    pinMode(heaterPin, OUTPUT);
    lcd.begin(20, 4);
    probe.begin();
    probe.requestTemperatures();
    lcd.setCursor(0, 0), lcd.print("Temp probe init OK");
    myPID.setBangBang(10);  // min/max at +-10 degrees from targetTemp
    myPID.setTimeStep(1000);
    lcd.setCursor(0, 1), lcd.print("PID: BangBang set at");
    lcd.setCursor(0, 2), lcd.print("+/- 10*C from target");
    lcd.setCursor(0, 3), lcd.print("System operational!");
    delay(3000);
}

void loop()
{
    if ((millis() - lastTempUpdate) > TEMP_READ_DELAY)
    {
        float tempRead = probe.getTempCByIndex(0);
        current.setTemp(tempRead);
        lastTempUpdate = millis();
        probe.requestTemperatures();
    }
    if (digitalRead(tUpPin))
    {
        target.adjustTemp(1);
        delay(100);
    }
    if (digitalRead(tDownPin))
    {
        target.adjustTemp(-1);
        delay(100);
    }
    myPID.run();
    analogWrite(10, outputVal);
    if ((millis() - lastLCDUpdate) > LCD_UPDATE_DELAY)
    {
        updateLCD();
        lastLCDUpdate = millis();
    }
}

void updateLCD ()
{
    // String conversion and processing numbers
    double tTemp = target.getTemp();
    String targetTempString = String(tTemp, 2);
    String fullTargetTempString = String("Target temp: " + targetTempString + " *C");
    double  cTemp = current.getTemp();
    String currentTempString = String(cTemp, 2);
    String fullCurrentTempString = String("Current temp: " + currentTempString + " *C");
    int pulse = pulsePercent();
    String pulsePercentString = String(pulse);
    String fullPulsePercentString = String("Pulse: " + pulsePercentString + "%");
    String KpString = String(Kp);
    String KiString = String(Ki);
    String KdString = String(Kd);
    String KPIDString = String("Kp: " + KpString + " Ki: " + KiString + " Kd: " + KdString);

    // Writing to LCD screen, bottom rown scrolls
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
        scrollCursor = 20;
    }
}

int pulsePercent (&outputVal)
{
    int mVal = &outputVal;
    int pulsePercentRounded = ((mVal/255)*100);
    return pulsePercentRounded;
}
