#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

// Declarations for hardware
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2, dTemp = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
OneWire oneWire(dTemp);
DallasTemperature probe(&oneWire);

// Declarations for Wi-Fi
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
WiFiServer server(80);
String header;

// Template declaration for temperature holder
float targetTemp = 0, tTemp;
float currentTemp, cTemp;
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
    };
    T1& adjustTemp (const T1& x)
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

// Display function to update LCD scren with temp info
void updateLCD ();


// Analog pins for 3 way switch: 14 for up regulation
// 15 for down regulation of targetTemp.
// INVERTED, connect to GND not 5V
// heaterPin is output for heater relay
const int tUpPin = 18, tDownPin = 19, heaterPin = 10; 

// Declarations for PID and heater
int heaterStatus;
int pulse = 0;

void setup()
{
    Serial.begin(9600);
    pinMode(tUpPin, INPUT_PULLUP);
    pinMode(tDownPin, INPUT_PULLUP);
    pinMode(heaterPin, OUTPUT);
    lcd.begin(20, 4);
    lcd.setCursor(0, 0), lcd.print("Connecting to:");
    lcd.setCursor(0, 1), lcd.print(ssid);
    WiFi.begin(ssid, password);
    int wifiCounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        lcd.setCursor(wifiCounter, 2), lcd.print(".");
        wifiCounter++; 
    }
    lcd.clear();
    lcd.setCursor(0, 0), lcd.print("WiFi connected.");
    lcd.setCursor(0, 1), lcd.print("IP ");
    lcd.setCursor(3, 1), lcd.print(WiFi.localIP());
    server.begin();
    probe.begin();
    lcd.setCursor(0, 2), lcd.print("Temp probe init OK");
    lcd.setCursor(0, 3), lcd.print("System operational!");
    delay(1000);
}

void loop()
{
    probe.requestTemperatures();
    currentTemp = probe.getTempCByIndex(0);
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

    if (target.getTemp() < current.getTemp())
    {
        // DO PULSE THINGS
    }
    updateLCD();
}

void updateLCD ()
{
    lcd.clear();
    lcd.setCursor(0, 0), lcd.print("Target temp: ");
    lcd.setCursor(13, 0), lcd.print(target.getTemp());
    lcd.setCursor(0, 1), lcd.print("Current temp: ");
    lcd.setCursor(14, 1), lcd.print(current.getTemp());
    lcd.setCursor(0, 2), lcd.print("Pulse ");
    lcd.setCursor(6, 2), lcd.print(pulse);
    lcd.setCursor(0, 3), lcd.print("IP ");
    lcd.setCursor(3, 3), lcd.print(WiFi.localIP());
}
