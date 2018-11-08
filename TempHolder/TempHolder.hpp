#ifndef TEMPHOLDER_CPP
#define TEMPHOLDER_CPP

// #include "Arduino.h"

class TempHolder
{
public:
    explicit TempHolder(double intialTemp);
    ~TempHolder();
    double getTemp();
    void setTemp(double newTemp);
    void adjustTemp(double incrementTemp);
private:
    double mTemp;
};

#endif