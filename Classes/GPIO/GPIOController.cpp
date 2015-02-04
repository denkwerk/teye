/*
 * 		GPIOController.cpp
 *
 *  	Created on: 15.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "GPIOController.h"
#include <wiringPi.h>
#include <softPwm.h>

#include <ctime>
#include <sys/time.h>

//#include <mcp3004.h>


namespace GPIO {

GPIOController::GPIOController() : _thread(0), _exit(0), _gpioEnabled(false) {
	// TODO Auto-generated constructor stub

	pthread_mutex_init(&_mutex_task_wait, NULL);
	pthread_mutex_init(&_mutex_task_lock, NULL);
	pthread_cond_init(&_cond_task_wait, NULL);
	pthread_mutex_init(&_mutex_queue_access_wait, NULL);
}

GPIOController::~GPIOController() {
	// TODO Auto-generated destructor stub

	pthread_mutex_destroy(&_mutex_task_wait);
	pthread_mutex_destroy(&_mutex_task_lock);
	pthread_cond_destroy(&_cond_task_wait);
	pthread_mutex_destroy(&_mutex_queue_access_wait);
}

bool GPIOController::SetFunction (GPIOFunction *Function) {

	vector<GPIOFunctionPortStruct> AcceptedPorts;
	if (this->CheckFunction(Function, AcceptedPorts)) {
		if (this->ActivateVirtualPorts(AcceptedPorts)) {
			if (this->ReservePorts(AcceptedPorts, Function)) {
				Function->SetGPIOController(this);
				return this->ExecuteThread();
			}
		}
	}

	return false;
}

bool GPIOController::RemoveFunction (GPIOFunction *Function) {
	pthread_mutex_lock(&_mutex_task_lock);

	if (this->FunctionRegistered(Function)) {
		this->DeleteAllFromQueue(Function, false);
		if (this->FreePorts(Function)) {
			delete Function;
			Function = 0;
			pthread_mutex_unlock(&_mutex_task_lock);
			return true;
		}
	}

	pthread_mutex_unlock(&_mutex_task_lock);
	return false;
}

bool GPIOController::IsInitialized () {
	return this->GPIOActivated();
}


// Protected Functionality

bool GPIOController::ClearAllJobsFromFunction (GPIOFunction *Function) {
	if (this->FunctionRegistered(Function)) {
		this->DeleteAllFromQueue(Function, true);
		return true;
	}

	return false;
}

bool GPIOController::NeedUpdate (GPIOFunction *Function, void *Data) {
	if (this->FunctionRegistered(Function)) {
		if (this->AddToQueue(Function, Data)) {
			pthread_mutex_unlock(&_mutex_task_wait);
			return true;
		}
	}

	return false;
}

void GPIOController::PWMWrite (int Port, float Value) {
	if (Port > -1) {
		//digitalWrite(GetVirtualPort(Port), Bit);
		softPwmWrite(GetVirtualPort(Port), Value);
	}
}

void GPIOController::DigitalWrite (int Port, int Bit) {
	if (Port > -1) {
		digitalWrite(GetVirtualPort(Port), Bit);
	}
}

int GPIOController::DigitalRead (int Port) {
	if (Port > -1) {
		return digitalRead(GetVirtualPort(Port));
	}
	return -1;
}


bool GPIOController::ExecuteThread () {

	if (_thread == 0) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		int error = pthread_create(&_thread, NULL, &StartIO, (void*)this);

		pthread_attr_destroy(&attr);

		//pthread_mutex_lock(&_mutex_task_wait);

		if (error == 0) {
			return true;
		}
	}

	return false;

}

bool GPIOController::Stop () {
	if (_thread != 0) {
		_exit = 1;

		pthread_mutex_unlock(&_mutex_task_wait);

		void *status;
		int error = pthread_join(_thread, &status);

		if (error == 0) {
			_thread = 0;
			_exit = 0;
			return true;
		}
	}
	return false;
}


// Private Static Thread Methods

void *GPIOController::RunTest (void *self) {

	GPIOController *IOController = (GPIOController*)self;
		  if (wiringPiSetup() == -1) {
			  IOController->_exit = 1;
		  }

		  pinMode(0, OUTPUT);

		  while(!IOController->_exit) {
		    // LED an
		    digitalWrite(0, 1);

		    // Warte 100 ms
		    delay(50);

		    // LED aus
		    digitalWrite(0, 0);

		    // Warte 100 ms
		    delay(50);
		  }
		  pthread_exit(NULL);
}

void *GPIOController::StartIO (void *self) {
	GPIOController *IOController = (GPIOController*)self;

	if (IOController->IsInitialized() == true) {

		while(!IOController->_exit) {

			if (IOController->QueueEmpty()) {
				pthread_mutex_lock(&IOController->_mutex_task_wait);
			}

			pthread_mutex_lock(&IOController->_mutex_task_lock);

			if (!IOController->_exit) {
				GPThreadContainer Container = IOController->PopFromQueue();
				if (Container.Function != 0) {
					long long WaitDuration = Container.Function->RunFunction(Container.Data);
#ifdef GPIO_TIMECRITICAL
					if (GPIO_TIMECRITICAL == 1) {
						if (WaitDuration != 0) {
							IOController->BlockClassTypeForDuration (Container.Function, WaitDuration);
						}
					}
#endif
				}
			}

			pthread_mutex_unlock(&IOController->_mutex_task_lock);
		}

	}

	pthread_exit(NULL);
}


// Helper (Data)

bool GPIOController::ActivateVirtualPorts (vector<GPIOFunctionPortStruct> AcceptedPorts) {
	bool Success = true;

	vector<GPIOFunctionPortStruct> VirtualPorts = this->GetVirtualPorts(AcceptedPorts);

	if (this->GPIOActivated() == false) {
		if (this->GPIOActivate() == false) {
			return false;
		}
	}

	vector<GPIOFunctionPortStruct>::iterator it;
	it = VirtualPorts.begin();


	while (it != VirtualPorts.end()) {
		Success &= this->GPIOBindPort((GPIOFunctionPortStruct)*it);
		it++;
	};

	return Success;
}

bool GPIOController::ReservePorts (vector<GPIOFunctionPortStruct> AcceptedPorts, GPIOFunction *Function) {
	bool Success = false;
	vector<GPIOFunctionPortStruct>::iterator it;
	it = AcceptedPorts.begin();

	while (it != AcceptedPorts.end()){
		_reservedGPIOPorts.insert(pair<GPIOFunctionPortStruct,GPIOFunction*>(*it,Function));
		Success |= true;
		it++;
	};

	return Success;
}

int GPIOController::GetVirtualPort (int Port, bool Hardware) {

	int i = 0;
	if (Port > -1) {
		while (_availableGPIOPorts[i] != '\0' && i < 1000) {
			if (_availableGPIOPorts[i][0] == Port) {
				if (Hardware == false) {
					return _availableGPIOPorts[i][1];
				} else {
					return _availableGPIOPorts[i][2];
				}
			}
			i++;
		}
	}

	return -1;
}

vector<GPIOFunctionPortStruct> GPIOController::GetVirtualPorts (vector<GPIOFunctionPortStruct> AcceptedPorts) {
	vector<GPIOFunctionPortStruct> VirtualPorts;

	vector<GPIOFunctionPortStruct>::iterator it;
	it = AcceptedPorts.begin();

	while (it != AcceptedPorts.end()) {
		int i = 0;
		if ((*it).Port > -1) {
			while (_availableGPIOPorts[i] != '\0' && i < 1000) {
				if (_availableGPIOPorts[i][0] == (*it).Port) {
					VirtualPorts.insert(VirtualPorts.end(), GPIOFunctionPortStruct(_availableGPIOPorts[i][1], (*it).Direction, (*it).Type, (*it).Range));
					break;
				}
				i++;
			}
		}
		it++;
	};

	return VirtualPorts;
}

bool GPIOController::CheckFunction (GPIOFunction *Function, vector<GPIOFunctionPortStruct> &ReservePorts) {
	GPIOFunctionPortStruct Port;
	vector<GPIOFunctionPortStruct> Ports;
	if ((Port = Function->MapPort()) == -1) {
		int Size = 0;
		if ((Size = (Ports = Function->MapPorts()).size()) > 0) {
			vector<int> Rejected;
			if ((Rejected = this->PortCheck(Ports)).size() > 0) {
				Function->PortsRejected(Rejected);
				return false;
			}
			ReservePorts = Ports;
			return true;
		}
	} else {
		if (this->PortCheckB(Port) == false) {
			Function->PortRejected(Port);
			return false;
		}
		ReservePorts.insert(ReservePorts.end(), Port);
		return true;
	}

	return false;
}

bool GPIOController::FreePorts (GPIOFunction *Function) {
	bool Erased = false;
	map<GPIOFunctionPortStruct, GPIOFunction*>::iterator it = _reservedGPIOPorts.begin();

	while (it != _reservedGPIOPorts.end()) {
		if ((*it).second == Function) {
			_reservedGPIOPorts.erase(it);
			Erased |= true;
		}
		it++;
	}
	return Erased;
}

bool GPIOController::PortCheckB (GPIOFunctionPortStruct Port) {
	vector<GPIOFunctionPortStruct> tmp;
	tmp.insert(tmp.end(), Port);
	return this->PortCheckB(tmp);
}

bool GPIOController::PortCheckB (vector<GPIOFunctionPortStruct> Data) {
	return (this->PortCheck(Data).size() == 0 ? true : false);
}

vector<int> GPIOController::PortCheck (GPIOFunctionPortStruct Port) {
	vector<GPIOFunctionPortStruct> tmp;
	tmp.insert(tmp.end(), Port);
	return this->PortCheck(tmp);
}

vector<int> GPIOController::PortCheck (vector<GPIOFunctionPortStruct> Data) {
	vector<int> Back;
	vector<GPIOFunctionPortStruct>::iterator it2;

	it2 = Data.begin();
	while (it2 != Data.end()) {
		if (_reservedGPIOPorts.find(*it2) != _reservedGPIOPorts.end()) {
			Back.insert(Back.end(), (*it2).Port);
		}
		it2++;
	};

	return Back;
}

bool GPIOController::FunctionRegistered (GPIOFunction *Function) {

	map<GPIOFunctionPortStruct, GPIOFunction*>::iterator it = _reservedGPIOPorts.begin();

	while (it != _reservedGPIOPorts.end()) {
		if ((*it).second == Function) {
			return true;
		}
		it++;
	};

	return false;
}

bool GPIOController::AddToQueue (GPIOFunction *Function, void *Data) {

	pthread_mutex_lock(&_mutex_queue_access_wait);
	_needUpdateQueue.insert(_needUpdateQueue.begin(), GPThreadContainer(Function, Data));
	pthread_mutex_unlock(&_mutex_queue_access_wait);

	return true;
}

GPThreadContainer GPIOController::PopFromQueue () {

	GPThreadContainer Container;

	pthread_mutex_lock(&_mutex_queue_access_wait);
	if (_needUpdateQueue.size() > 0) {
		//int Size = _needUpdateQueue.size();

		bool TimeCritical = false;

#ifdef GPIO_TIMECRITICAL
		if (GPIO_TIMECRITICAL == 1) {
			TimeCritical = true;

			queue::reverse_iterator it = _needUpdateQueue.rbegin();
			bool Found = false;
			while (it != _needUpdateQueue.rend()) {
				if (this->ExistsBlockClasses() == true) {
					GPIOFunction *Func = (*it).Function;
					if (this->CheckClassTypeIfFree(Func)) {
						Container = *it;
						Found = true;
						break;
					}
				} else {
					Container = *it;
					Found = true;
					break;
				}
				it--;
			}

			if (Found == true) {
				_needUpdateQueue.erase(std::next(it).base());
			}
		}
#endif
		if (TimeCritical == false) {
			Container = _needUpdateQueue.back();
			_needUpdateQueue.pop_back();
		}

	}
	//int Size = _needUpdateQueue.size();
	pthread_mutex_unlock(&_mutex_queue_access_wait);

	return Container;
}

void GPIOController::DeleteAllFromQueue (GPIOFunction *Function, bool Locked) {
	if (Locked == true) {
		pthread_mutex_lock(&_mutex_queue_access_wait);
	}

	vector<GPThreadContainer>::iterator it = _needUpdateQueue.end();

	while (it != _needUpdateQueue.begin()) {
		if ((*it).Function == Function) {
			_needUpdateQueue.erase(it);
		}
		it--;
	}

	if (Locked == true) {
		pthread_mutex_unlock(&_mutex_queue_access_wait);
	}
}

bool GPIOController::QueueEmpty () {
	pthread_mutex_lock(&_mutex_queue_access_wait);
	bool Return (_needUpdateQueue.size() > 0 ? false : true);
	pthread_mutex_unlock(&_mutex_queue_access_wait);
	return Return;
}

bool GPIOController::BlockClassTypeForDuration (GPIOFunction *Function, long long Duration) {
	if (Function != 0) {
		TypeInfo info = TypeInfo(typeid(*Function));
		classmap::iterator it = _blockedClasses.find(info);

		long long CurrentTime = GetMillisecondTime();

		if (it == _blockedClasses.end()) {
			_blockedClasses[info] = CurrentTime + Duration;
			return true;
		} else {
			it->second = CurrentTime + Duration;
			return true;
		}
	}

	return false;
}

bool GPIOController::ExistsBlockClasses () {
	return (_blockedClasses.size() > 0 ? true : false);
}

bool GPIOController::CheckClassTypeIfFree (GPIOFunction *Function) {

	if (Function != 0) {
		TypeInfo info = TypeInfo(typeid(*Function));
		classmap::iterator it = _blockedClasses.find(info);

		if (it == _blockedClasses.end()) {
			return true;
		}

		long long CurrentTime = GetMillisecondTime();

		if (it->second <= CurrentTime) {
			_blockedClasses.erase(it);
			return true;
		}
	}

	return false;
}


// GPIO Library Methods

bool GPIOController::GPIOActivate () {
	if (wiringPiSetup() != -1) {
		_gpioEnabled = true;
		return true;
	}
	return false;
}

bool GPIOController::GPIOActivated () {
	return _gpioEnabled;
}

bool GPIOController::GPIOBindPort (GPIOFunctionPortStruct Port) {
	if (this->GPIOActivated()) {
		/*switch (Port.Direction) {
			case GPIOFunctionPortStruct::OUTPUT:
				pinMode(Port.Port, OUTPUT);
				return true;
				break;
			case GPIOTypeINPUT:
				pinMode(Port, OUTPUT);
				return true;
				break;
			default:
				break;
		}*/
		if (Port.Type == GPIOTypePWM) {
			softPwmCreate(Port.Port, 0, Port.Range);
			return true;
		} else if (Port.Type == GPIOTypePort) {
			pinMode(Port.Port, Port.Direction);
			return true;
		} else if (Port.Type == GPIOTypePWMRaw) {
			return true;
		}
	}
	return false;
}

long long GPIOController::GetMillisecondTime () {
	struct timeval  te;
	gettimeofday(&te, 0);

	return te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
}

} /* namespace GPIO */
