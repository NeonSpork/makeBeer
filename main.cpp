#include <LiquidCrystal.h>
#include <DallasTemperature.h>

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2, dTemp = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
OneWire oneWire(dTemp);
DallasTemperature probe(&oneWire);

float currentTemp;
float targetTemp = 0;

// Analog pins for 3 way switch: 14 for up regulation
// 15 for down regulation of targetTemp.
// INVERTED, connect to GND not 5V
const int tUpPin = 14, tDownPin = 15; 

int heaterStatus;
int pulse;

void setup() {
    lcd.begin(20, 4);
    probe.begin();
    pinMode(tUpPin, INPUT_PULLUP);
    pinMode(tDownPin, INPUT_PULLUP);
}

void loop() {
    probe.requestTemperatures();
    currentTemp = probe.getTempCByIndex(0);

    if (digitalRead(tUpPin)) {
        targetTemp += 1;
    }
    if (digitalRead(tDownPin)) {
        targetTemp -= 1;
        if (targetTemp < 0) {
            targetTemp == 0;
        }
    }
    
    lcd.setCursor(0, 0), lcd.print("Target temp: ");
    lcd.setCursor(13, 0), lcd.print(targetTemp);
    lcd.setCursor(15, 0), lcd.print(" *C");
    lcd.setCursor(0, 1), lcd.print("Current temp: ");
    lcd.setCursor(14, 1), lcd.print(currentTemp);
    lcd.setCursor(16, 1), lcd.print(" *C");
    lcd.setCursor(0, 2), lcd.print("Pulse ");
    lcd.setCursor(6, 2), lcd.print(pulse);
}