#ifndef _ARRAYOP_h
#define _ARRAYOP_h
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#include "TM1637Display.h"
#else
#include "WProgram.h"
#endif
//this is boring setup
class ArrayOpClass : public TM1637Display
{
private:
	uint8_t pinClk = 11;
	uint8_t pinDIO = 12;
	unsigned int bitDelay = 100;
	TM1637Display voltDisplay;
public:
	//This first chunk until the functions below is just initializing stuff for the functions
	int (ArrayOpClass::* onePtr)(double) = &ArrayOpClass::oneBat;		//These are the pointers for the charging functions and set up the array
	int (ArrayOpClass::* twoPtr)(double) = &ArrayOpClass::twoBat;
	int (ArrayOpClass::* threePtr)(double) = &ArrayOpClass::threeBat;
	int (ArrayOpClass::* fourPtr)(double) = &ArrayOpClass::fourBat;
	int (ArrayOpClass::* funcArray[4])(double);
	//Class variables
	int voltArray[4];
	int probeInputPin = 0;
	int pwmOutputPin = 0;
	int ampInputPin = 0;
	int relayPins[4][2];
	int chargePos[4];
	int lowestBat[2] = { 0,0 };			//This is for the lowest battery, [0] is batt number and [1] is charge level
	const uint8_t done[4] = { 0b00111111, 0b01011100, 0b01010100, 0b01111001 };
	const uint8_t fail[4] = { 0b01110001,0b01110111,0b00110000,0b00111000 };
	const uint8_t current[4] = { 0b011100010,0b00000000,0b00111111,0b00111001 };
	
	//These are constructors, no arg is for masterObj in .cpp, other is for bogusObj in .ino
	ArrayOpClass() {
		int probeInputPin = 0;
		int pwmOutputPin = 0;
	}
	ArrayOpClass(int arr[][2])			//Constructor for bogusObj in the .ino, to pass the relay pin array to mastersweep via operator= overload
	{
		for (int row = 0; row <= 1; row++) {
			for (int col = 0; col <= 3; col++) {
				relayPins[col][row] = arr[col][row];
			}
		}
		int probeInputPin = 0;
		int pwmOutputPin = 0;
	}
	void operator=(const ArrayOpClass& obj)			//The assignment operator overload is to assign one object to another
	{
		probeInputPin = obj.probeInputPin;
		pwmOutputPin = obj.pwmOutputPin;
		ampInputPin = obj.ampInputPin;
		for (int i = 0; i <= 3; i++) {
			voltArray[i] = obj.voltArray[i];
			chargePos[i] = obj.chargePos[i];
		}
		for (int row = 0; row <= 1; row++) {
			for (int col = 0; col <= 3; col++) {
				relayPins[col][row] = obj.relayPins[col][row];
			}
		}
	}
	//These are for battery charging
	int oneBat(double x) {
		int pow = 0;
		pow = (35.0 / (.5 * x + 1.0) + 220.0);
		return pow;
	}
	int twoBat(double x) {
		int pow = 0;
		pow = (47.0 / (.5 * x + 1.0) + 208.0);
		return pow;
	}
	int threeBat(double x) {
		int pow = 0;
		pow = (65.0 / (.4 * x + 1.0) + 190.0);
		return pow;
	}
	int fourBat(double x) {
		int pow = 0;
		pow = (165.0 / (.8*x + 1.0) + 90.0);	
		return pow;
	}
	//Just the class functions
	void masterSweep(ArrayOpClass);
	void chargingTime(int, int);
	int findLow();
	int findHigh(int);
	void arrayStop();
	void overAmp();
};
extern ArrayOpClass ArrayOp;
#endif