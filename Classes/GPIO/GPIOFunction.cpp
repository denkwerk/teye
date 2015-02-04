/*
 * 		GPIOFunction.cpp
 *
 *  	Created on: 16.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "GPIOFunction.h"
#include "GPIOController.h"

#include <wiringPi.h>

#include <stdlib.h>

namespace GPIO {

GPIOFunction::GPIOFunction() : _Controller(0), _Port(-1) {
	// TODO Auto-generated constructor stub

}

GPIOFunction::~GPIOFunction() {
	// TODO Auto-generated destructor stub
	_Ports.clear();
	_Port = -1;
	_Controller = 0;
	if (_Controller != 0) {
		//_Controller->RemoveFunction(this);
	}
}


// Functionality Protected

bool GPIOFunction::ClearMeFromQueue() {
	if (this->IsInitialized()) {
		return _Controller->ClearAllJobsFromFunction(this);
	}
	return false;
}

bool GPIOFunction::NeedsUpdate(void *Data) {
	if (this->IsInitialized()) {
		return _Controller->NeedUpdate(this, Data);
	}
	return false;
}

void GPIOFunction::MapPort (int Port, GPIOFunctionPortStructDirection Direction) {
	_Port = GPIOFunctionPortStruct(Port, Direction);
	_Ports.insert(_Ports.end(), _Port);
}

void GPIOFunction::MapPorts (vector<GPIOFunctionPortStruct> Ports) {
	_Ports = Ports;
}

int GPIOFunction::GPIOCommand (int Port, int Bit) {
	return GPIOCommandF(Port, Bit);
}

int GPIOFunction::GPIOCommandF (int Port, float Bit) {

	if (Port > -1) {
		vector<GPIOFunctionPortStruct>::iterator it = _Ports.begin();

		while (it !=_Ports.end()) {
			if (*it != 0 && (*it) == Port) {
				if ((*it).Type == GPIOType::GPIOTypePWM) {
					_Controller->PWMWrite(Port, Bit);
					return 1;
				} else if ((*it).Type == GPIOType::GPIOTypePWMRaw) {
					std::string command("echo ");
					command.append(to_string(_Controller->GetVirtualPort(Port, true)));
					command.append("=");
					command.append(to_string(Bit));
					command.append("% > /dev/servoblaster");
					system(command.c_str());
				} else {
					if ((*it).Direction == OUTPUT) {
						_Controller->DigitalWrite(Port, Bit);
					} else if ((*it).Direction == INPUT) {
						return _Controller->DigitalRead(Port);
					}
					return 1;
				}
			}
			it++;
		}
	}

	return -1;
}


// Functionality Private

void GPIOFunction::SetGPIOController (GPIOController* Controller) {
	_Controller = Controller;
}

bool GPIOFunction::IsInitialized () {
	if (_Controller != 0) {
		return _Controller->IsInitialized();
	}
	return false;
}


// Interface

// GPIOFunction Interface Methods

GPIOFunctionPortStruct GPIOFunction::MapPort() {
	return _Port;
}

vector<GPIOFunctionPortStruct> GPIOFunction::MapPorts () {
	return _Ports;
}

void GPIOFunction::PortRejected(GPIOFunctionPortStruct Port) {

}

void GPIOFunction::PortsRejected(vector<int>Ports) {

}


} /* namespace GPIO */
