#include "arrayOp.h"
#include "TM1637Display.h"

ArrayOpClass masterObj;
//Mastersweep: assigns objects from bogusObj to masterObj
//activates the display and searches for the pins and then goes into the charging loop
void ArrayOpClass::masterSweep(ArrayOpClass tempObj)
{
  masterObj = tempObj;
  voltDisplay = TM1637Display(pinClk, pinDIO);
  voltDisplay.setBrightness(5);
  int low = findLow();
  //THESE ARE SAFEGUARDS THAT ARE DISABLED FOR TESTING EVERYTHING ELSE
  if (low == -1)
    arrayStop();
  int high = findHigh(low);
  if (high == -1)
    arrayStop();
  chargingTime(low, high);
  return;
}

//finds the "ground relay" for charging, returns -1 if there is an error to prevent catastrophe
int ArrayOpClass::findLow()
{
  int seek = -1;
  for (int i = 0; i <= 3; i++) {
    if (seek >= 0)
      break;
    if (masterObj.chargePos[i] == 1) {
      seek = i;
    }
  }
  return seek;
}
//find the "high relay", returns -1 if there is an error to prevent catastrophe
int ArrayOpClass::findHigh(int initial)
{
  int upper = -1;
  if (masterObj.chargePos[initial] == 1) {
    upper = initial;
  }
  for (int i = (initial + 1); i <= 3; i++) {
    if (masterObj.chargePos[i] == 0)
      continue;
    if (masterObj.chargePos[i] == 1)
      upper = i;
  }
  return upper;
}

//This is it!
//Pointers to member functions in an array to simplify the power-level-search/select process
void ArrayOpClass::chargingTime(int bottom, int top)
{
  masterObj.funcArray[0] = masterObj.onePtr;
  masterObj.funcArray[1] = masterObj.twoPtr;
  masterObj.funcArray[2] = masterObj.threePtr;
  masterObj.funcArray[3] = masterObj.fourPtr;
  double powerX = 0;
  int powerY = 255;
  unsigned long currentAmps = 0;
  unsigned long avgNum = 0;
  voltDisplay.showNumberDec(0, 0, 1, 3);
  analogWrite(masterObj.pwmOutputPin, 255);
  digitalWrite(masterObj.relayPins[bottom][0], LOW);
  digitalWrite(masterObj.relayPins[top][1], LOW);
  unsigned long currentTime = millis();
  unsigned long secondaryTime = millis();
  while (millis() - currentTime <= 60000) {         //TIMER FOR CHARGING
    while (millis() - secondaryTime <= 200) {
      currentAmps += analogRead(masterObj.ampInputPin);
      avgNum++;
      analogWrite(masterObj.pwmOutputPin, powerY);
      delay(80);
    }
    currentAmps /= avgNum;
    powerY = (masterObj.*(masterObj.funcArray[top - bottom]))(powerX);
    if (currentAmps >= 90) {
      analogWrite(masterObj.pwmOutputPin, 255);
      arrayStop();
    }
    if (currentAmps < 78)
      powerX++;
    if (currentAmps >= 83 && currentAmps <= 87)
      powerX--;
    voltDisplay.showNumberDec(currentAmps * 2.564, 0, 4, 0);
    currentAmps = 0;
    avgNum = 0;
    secondaryTime = millis();
  }
  analogWrite(masterObj.pwmOutputPin, 255);
  digitalWrite(masterObj.relayPins[bottom][0], HIGH);
  digitalWrite(masterObj.relayPins[top][1], HIGH);
  return;
}

void ArrayOpClass::arrayStop() {
  {
    voltDisplay.clear();
    voltDisplay.setSegments(fail);
    while (1);
  }
}
