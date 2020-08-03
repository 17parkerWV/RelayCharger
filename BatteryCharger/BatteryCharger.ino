#include "arrayOp.h"
#include "TM1637Display.h"
//basically all of this until the end of setup() is relay pins related
const int relay1 = 9;
const int relay2 = 8;
const int relay3 = 7;
const int relay4 = 6;
const int relay5 = 5;
const int relay6 = 4;
const int relay7 = 3;
const int relay8 = 2;
int relayPairs[4][2] = { {relay1, relay5}, {relay2, relay6}, {relay3, relay7}, {relay4, relay8} };
const int pwmPin = 10;
const int probePin = A0;
const int ampPin = A7;
//prototypes and objects
void cellSearch();
void stop();
void doneCharge();
const uint8_t noBattery[4] = { 0b01010100,0b01011100,0b00000000,0b01111100 };	//says no b
const uint8_t done[4] = { 0b00111111, 0b01011100, 0b01010100, 0b01111001 };
ArrayOpClass bogusObj(relayPairs);
TM1637Display countdown = TM1637Display(11, 12);

void setup() {
	pinMode(relay1, OUTPUT);
	pinMode(relay2, OUTPUT);
	pinMode(relay3, OUTPUT);
	pinMode(relay4, OUTPUT);
	pinMode(relay5, OUTPUT);
	pinMode(relay6, OUTPUT);
	pinMode(relay7, OUTPUT);
	pinMode(relay8, OUTPUT);
	pinMode(pwmPin, OUTPUT);
	digitalWrite(relay1, HIGH);
	digitalWrite(relay2, HIGH);
	digitalWrite(relay3, HIGH);
	digitalWrite(relay4, HIGH);
	digitalWrite(relay5, HIGH);
	digitalWrite(relay6, HIGH);
	digitalWrite(relay7, HIGH);
	digitalWrite(relay8, HIGH);
	digitalWrite(pwmPin, HIGH);
	analogWrite(pwmPin, 255);
	countdown.setBrightness(4, 1);              //For the countdown display
	for (int tim = 0; tim <= 100; tim++) {
		countdown.showNumberDec(100 - tim, 0, 4, 0);
	}
	Serial.begin(9600);
}

//Makes bogusObj and sends that to the cpp file where the bulk of the work is done
//IDK why I am doing it this way but it's fun I guess
void loop() {
	bogusObj.probeInputPin = probePin;
	bogusObj.pwmOutputPin = pwmPin;
	bogusObj.ampInputPin = ampPin;
	cellSearch();
	bogusObj.masterSweep(bogusObj);
}

//find the cells that need to be charged, else stop() program
//"returns the array" of which cells are low, if none need charging stop() the program
void cellSearch() {
	int chargeSum = 0;
	static int c[4] = { 0, 0, 0, 0 };
	bool okToChargeFlag = false;
	for (int i = 0; i <= 3; i++) {						//Sets pins for relay, reads the probe pin, shuts off relay
		digitalWrite(bogusObj.relayPins[i][0], LOW);
		digitalWrite(bogusObj.relayPins[i][1], LOW);
		delay(150);
		chargeSum = 0;
		for (int probeLoop = 0; probeLoop <= 3; probeLoop++) {
			chargeSum += analogRead(probePin);
		}
		Serial.println(chargeSum);
		digitalWrite(bogusObj.relayPins[i][0], HIGH);
		digitalWrite(bogusObj.relayPins[i][1], HIGH);
		bogusObj.voltArray[i] = chargeSum;
		if (chargeSum < 805 && chargeSum > 550) {               //CUTOFF VALUE
			bogusObj.chargePos[i] = 1;
			okToChargeFlag = true;
		}
		if (chargeSum >= 805 || chargeSum <= 550) {             //CUTOFF VALUE
			bogusObj.chargePos[i] = 0;
		}
		delay(150);
	}
	if (okToChargeFlag == false)
		stop();
	if (okToChargeFlag == false)
		doneCharge();
	return;
}


//self explanatory
void stop() {
	countdown.clear();
	countdown.setSegments(noBattery);
	while (1);
}

void doneCharge() {
	countdown.clear();
	countdown.setSegments(done);
	while (1);
}
