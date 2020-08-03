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
	if (low == -1)								//SAFEGUARDS
		arrayStop();
	int high = findHigh(low);
	if (high == -1)
		arrayStop();							//END SAFEGUARDS
	chargingTime(low, high);
	voltDisplay.clear();		//THIS IS TEMPORARY, NEED TO PUT IN END OF CHARGE CYCLE FUNCTIONS
	delay(2000);				//THIS IS TO DRAIN THE CAPACITOR
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
	masterObj.funcArray[0] = masterObj.onePtr;					//SETUP START
	masterObj.funcArray[1] = masterObj.twoPtr;
	masterObj.funcArray[2] = masterObj.threePtr;
	masterObj.funcArray[3] = masterObj.fourPtr;
	double powerX = 0;
	int powerY = 255;
	unsigned long currentAmps = 0;
	unsigned long avgNum = 0;
	int runningCharge = 0;
	for (int batIndex = 0; batIndex <= 3; batIndex++) {
		if (masterObj.chargePos[batIndex] == 0)
			continue;
		if (masterObj.chargePos[batIndex] == 1)
			voltDisplay.showNumberDec(batIndex);
		delay(750);
		float x;
		x = static_cast<float>(masterObj.voltArray[batIndex]) * .5191;			//THIS IS THE COEFFICIENT for volts
		int y = round(x);
		voltDisplay.showNumberDecEx(y, 0b01000000);
		delay(1250);
	}
	voltDisplay.clear();
	voltDisplay.showNumberDec(0, 0, 1, 3);
	analogWrite(masterObj.pwmOutputPin, 255);
	digitalWrite(masterObj.relayPins[bottom][0], LOW);
	digitalWrite(masterObj.relayPins[top][1], LOW);
	unsigned long currentTime = millis();
	unsigned long secondaryTime = millis();				//SETUP END
	while (millis() - currentTime <= 120000) {					//TIMER FOR CHARGING
		while (millis() - secondaryTime <= 200) {	//minor loop for amp numbers
			currentAmps += analogRead(masterObj.ampInputPin);
			avgNum++;
			analogWrite(masterObj.pwmOutputPin, powerY);
			delay(80);
		}			//end minor loop
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
		for (int probeLoop = 0; probeLoop <= 3; probeLoop++) {
			runningCharge += analogRead(masterObj.probeInputPin);
		}
		if (runningCharge <= 300) {
			voltDisplay.setSegments(fail);
			break;
		}
		runningCharge = 0;
		voltDisplay.showNumberDec(currentAmps * 2.564, 0, 4, 0);
		currentAmps = 0;
		avgNum = 0;
		secondaryTime = millis();
	}																///////END CHARGING
	analogWrite(masterObj.pwmOutputPin, 255);		//rest is end of charge shutoff
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
