/*
 * 		GPIOMCP3008Function.cpp
 *
 *  	Created on: 21.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "GPIOMCP3008Function.h"

#include <unistd.h>

#include <stdio.h>
#include <string>

namespace GPIO {
namespace InternalFunctions {


// Helper Static Constructor

GPIOMCP3008Function* GPIOMCP3008Function::GPIOMCP3008FunctionWithPort (GPIOMCP3008FunctionPorts Ports) {
	GPIOMCP3008Function *Function = new GPIOMCP3008Function();

	Function->SetPorts(Ports);

	return Function;
}


// Private De -and Constructors

GPIOMCP3008Function::GPIOMCP3008Function() : GPIOFunction(), _thread(0), _exit(false), _value(0) {
	// TODO Auto-generated constructor stub

	pthread_mutex_init(&_mutex_task_wait, NULL);

	SendEvent(new MotionRawEvent(EXSignalIOMotionTypeInitDone));
}

GPIOMCP3008Function::~GPIOMCP3008Function() {
	// TODO Auto-generated destructor stub

	pthread_mutex_destroy(&_mutex_task_wait);
}


// MCP3008 Methods

bool GPIOMCP3008Function::StartPolling() {
	if (_thread == 0) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		SendEvent(new MotionRawEvent(EXSignalIOMotionTypeStartPolling));
		int error = pthread_create(&_thread, NULL, &RunPolling, (void*)this);

		pthread_attr_destroy(&attr);

		//pthread_mutex_lock(&_mutex_task_wait);
		if (error == 0) {
			return true;
		}
	}
	return false;
}

bool GPIOMCP3008Function::StopPolling() {
	if (_thread != 0) {
		_exit = 1;

		//pthread_mutex_unlock(&_mutex_task_wait);

		void *status;
		int error = pthread_join(_thread, &status);

		if (error == 0) {
			SendEvent(new MotionRawEvent(EXSignalIOMotionTypeStopPolling));
			_thread = 0;
			_exit = 0;
			return true;
		}
	}
	return false;
}

int GPIOMCP3008Function::GetValue () {
	int Value = 0;
	pthread_mutex_lock(&_mutex_task_wait);
	Value = _value;
	pthread_mutex_unlock(&_mutex_task_wait);
	return Value;
}


// Signal Handle
void GPIOMCP3008Function::SetSignalCallback (void (*cb)(void *Signal)) {

}


// GPIOFunction Interface Methods

long long GPIOMCP3008Function::RunFunction(void *Data) {
	int i; // "for-loop-integer"
	int InputArray[5]; // will contain the input number
	int Result = 0; // this will be returned


	this->GPIOCommand(_Ports.CS, GPIO::HIGH);
	this->GPIOCommand(_Ports.CLK, GPIO::LOW);
	this->GPIOCommand(_Ports.CS, GPIO::LOW);

	this->Select_Chip(InputArray, _Ports.Input);
	for (i=0; i<5; i++){
		this->Clock_Write(_Ports.CLK, _Ports.DIn, InputArray[i]); // selects the input on the chip.
	}

	for (i=12; i>0; i--) {
		if (this->Clock_Read(_Ports.CLK, _Ports.DOut)) {
			Result += this->Power_Of_2(i);
		}
	}

	this->GPIOCommand(_Ports.CS, GPIO::HIGH);
	Result /=4 ;

	pthread_mutex_lock(&_mutex_task_wait);
	_value = Result;
	SendEvent(new MotionRawEvent(_value));
	pthread_mutex_unlock(&_mutex_task_wait);

	return 0;
}


// Internal Methods

void *GPIOMCP3008Function::RunPolling (void *self) {
	GPIOMCP3008Function *IOController = (GPIOMCP3008Function*)self;

	while(!IOController->_exit) {
		IOController->NeedsUpdate();
		usleep(200000);
	}

	pthread_exit(NULL);
}

void GPIOMCP3008Function::SetPorts (GPIOMCP3008FunctionPorts Ports) {

	_Ports = Ports;

	vector<GPIOFunctionPortStruct> PortVector;
	PortVector.insert(PortVector.end(), GPIOFunctionPortStruct(Ports.CLK, GPIOFunctionPortStructDirection::OUTPUT));
	PortVector.insert(PortVector.end(), GPIOFunctionPortStruct(Ports.DOut, GPIOFunctionPortStructDirection::INPUT));
	PortVector.insert(PortVector.end(), GPIOFunctionPortStruct(Ports.DIn, GPIOFunctionPortStructDirection::OUTPUT));
	PortVector.insert(PortVector.end(), GPIOFunctionPortStruct(Ports.CS, GPIOFunctionPortStructDirection::OUTPUT));

	this->MapPorts(PortVector);
}


// Helper

void GPIOMCP3008Function::Clock_Write (int clockpin, int out, int value) {
	this->GPIOCommand(out, value);
	this->GPIOCommand(clockpin, GPIO::HIGH);
	this->GPIOCommand(clockpin, GPIO::LOW);
}

int GPIOMCP3008Function::Clock_Read(int clockpin, int in) {
	this->GPIOCommand(clockpin, GPIO::HIGH);
	this->GPIOCommand(clockpin, GPIO::LOW);
	return this->GPIOCommand(in);
}

void GPIOMCP3008Function::Select_Chip (int bin[], int inputnum) {
	bin[0] = 1; bin[1] = 1; //the first two bits have to be 1
	int i = 2;
	int o[3];

	for (i=2; i >= 0; i--) { // converts inputnum to a 3-Bit binary number
		o[i] = (inputnum % 2);
		inputnum /= 2;
	}

	int c;

	for (c  = i-1; c>=0; c--) {
		o[c] = 0;
	}

	c = 2;

	for (i = 0; i<3; i++) {
		bin[c] = o[i];
		c++;
	}
}

int GPIOMCP3008Function::Power_Of_2 (int exp) {
	int output = 1;
	int i;

	for (i = 1; i<=exp; i++) {
		output *= 2;
	}

	return output;
}

} /* namespace InternalFunctions */
} /* namespace GPIO */
