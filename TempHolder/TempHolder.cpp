#include "TempHolder.hpp"


TempHolder::TempHolder(double initialTemp)
: mTemp(initialTemp) {}
TempHolder::~TempHolder(){}

double TempHolder::getTemp()
{
  return mTemp;
}
void TempHolder::setTemp(double newTemp)
{
  mTemp = newTemp;
}
void TempHolder::adjustTemp(double incrementTemp)
{
  mTemp += incrementTemp;
  if (mTemp<0)
  {
    mTemp = 0;
  }
}