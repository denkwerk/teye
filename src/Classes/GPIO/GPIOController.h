/*
 * 		GPIOController.h
 *
 *  	Created on: 15.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef GPIOCONTROLLER_H_
#define GPIOCONTROLLER_H_

#include <pthread.h>
#include <map>
#include <vector>

#include "../Event/TypeInfo.h"

#include "GPIOFunction.h"

#define GPIO_TIMECRITICAL 0

namespace GPIO {

	struct GPThreadContainer {
		GPIOFunction *Function;
		void *Data;

		GPThreadContainer () : Function(0), Data(0) {};
		GPThreadContainer (GPIOFunction *Function, void *Data = 0) : Function(Function), Data(Data) {};

		~GPThreadContainer() {};
	};

	using namespace std;

	class GPIOController {

		friend class GPIOFunction;

		private:

			typedef std::map<TypeInfo, long long> classmap;
			typedef vector<GPThreadContainer> queue;

			// Threading Variables
			pthread_t			_thread;
			pthread_mutex_t		_mutex_task_wait;
			pthread_mutex_t		_mutex_task_lock;
			pthread_mutex_t		_mutex_queue_access_wait;
			pthread_cond_t		_cond_task_wait;
			std::map<int, pthread_t>	_threads;
			int _exit;

			// Internal Variables
			queue _needUpdateQueue;
			std::map<GPIOFunctionPortStruct, GPIOFunction*> _reservedGPIOPorts;
			classmap _blockedClasses;
			bool _gpioEnabled;

			enum GPIOType {
				GPIOTypeOUTPUT,
				GPIOTypeINPUT
			};

			//int _degree;
			//int _selectedGPIO;

			const int _availableGPIOPorts[21][3] = {
					{2,8, 3},
					{3,9, 5},
					{4,7, 7},
					{7,11, 26},
					{8,10, 24},
					{9,13, 21},
					{10,12, 19},
					{11,14, 23},
					{14,15, 8},
					{15,16, 10},
					{17,0, 11},
					{18,1, 12},
					{22,3, 15},
					{23,4, 16},
					{24,5, 18},
					{25,6, 22},
					{27,2, 13},
					{28,17, 3},
					{29,18, 4},
					{30,19, 5},
					{31,20, 6}
			};

			// Threading Methods
			static void *RunTest (void *self);
			static void *StartIO (void *self);

			// Internal Methods
			void SetPosition (int Servo, int Degree);


			// Helper (Data)
			bool ActivateVirtualPorts (vector<GPIOFunctionPortStruct> AcceptedPorts);
			bool ReservePorts (vector<GPIOFunctionPortStruct> AcceptedPorts, GPIOFunction *Function);
			int GetVirtualPort (int Port, bool Hardware = false);
			vector<GPIOFunctionPortStruct> GetVirtualPorts (vector<GPIOFunctionPortStruct> AcceptedPorts);
			bool CheckFunction (GPIOFunction *Function, vector<GPIOFunctionPortStruct> &ReservePorts);
			bool FreePorts (GPIOFunction *Function);
			bool PortCheckB (GPIOFunctionPortStruct Port);
			bool PortCheckB (vector<GPIOFunctionPortStruct> Data);
			vector<int> PortCheck (GPIOFunctionPortStruct Port);
			vector<int> PortCheck (vector<GPIOFunctionPortStruct> Data);
			bool FunctionRegistered (GPIOFunction *Function);

			bool ExecuteThread ();

			bool AddToQueue (GPIOFunction *Function, void *Data = 0);
			GPThreadContainer PopFromQueue ();
			void DeleteAllFromQueue (GPIOFunction *Function, bool Locked = true);
			bool QueueEmpty ();

			bool BlockClassTypeForDuration (GPIOFunction *Function, long long Duration);
			bool ExistsBlockClasses ();
			bool CheckClassTypeIfFree (GPIOFunction *Function);

			// GPIO Library Methods
			bool GPIOActivate ();
			bool GPIOActivated ();
			bool GPIOBindPort (GPIOFunctionPortStruct Port);

			long long GetMillisecondTime ();

		protected:
			bool ClearAllJobsFromFunction (GPIOFunction *Function);
			bool NeedUpdate (GPIOFunction *Function, void *Data = 0);
			void PWMWrite (int Port, float Value);
			void DigitalWrite (int, int);
			int DigitalRead (int Port);

		public:
			GPIOController();
			virtual ~GPIOController();

			bool SetFunction (GPIOFunction *Function);
			bool RemoveFunction (GPIOFunction *Function);

			bool IsInitialized ();

			bool Stop();
	};

} /* namespace GPIO */

#endif /* GPIOCONTROLLER_H_ */
