/*
 * 		GPIOFunction.h
 *
 *  	Created on: 16.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef GPIOFUNCTION_H_
#define GPIOFUNCTION_H_

#include <vector>
#include "../Event/EventHandler.h"

namespace GPIO {

	using namespace std;

	class GPIOController;

	enum GPIOValue {
		UNDEFINED = -1,
		HIGH = 1,
		LOW = 0,
		BIGHIGH = 100
	};

	enum GPIOFunctionPortStructDirection {
		UNDEFINE = -1,
		INPUT = 0,
		OUTPUT = 1,
		PWM_OUTPUT = 2,
		GPIO_CLOCK = 3
	};

	enum GPIOType {
		GPIOTypeNone = -1,
		GPIOTypePort,
		GPIOTypePWM,
		GPIOTypePWMRaw
	};

	struct GPIOFunctionPortStruct {
		int Port;
		GPIOFunctionPortStructDirection Direction;
		GPIOType Type;
		int Range;

		GPIOFunctionPortStruct() : Port(-1), Direction(GPIOFunctionPortStructDirection::UNDEFINE), Type(GPIOType::GPIOTypeNone), Range(200) {};
		GPIOFunctionPortStruct(int _Port, GPIOFunctionPortStructDirection _Direction = GPIOFunctionPortStructDirection::OUTPUT, GPIOType Type = GPIOType::GPIOTypePort, int Range = 200) : Port(_Port), Direction(_Direction), Type(Type), Range(Range) {};
		~GPIOFunctionPortStruct() {};

		inline GPIOFunctionPortStruct operator=(GPIOFunctionPortStruct PortI) {
			Port = PortI.Port;
			Direction = PortI.Direction;
			Type = PortI.Type;
			Range = PortI.Range;
			return Port;
		}

		bool operator!=(const int &PortI) const {
			return Port != PortI;
		}

		bool operator!=(const GPIOFunctionPortStruct &PortI) const {
			return Port != PortI.Port;
		}

		bool operator==(const GPIOFunctionPortStruct &PortI) const {
			return Port == PortI.Port;
		}

		bool operator<=(const GPIOFunctionPortStruct &PortI) const {
			return Port <= PortI.Port;
		}

		bool operator>=(const GPIOFunctionPortStruct &PortI) const {
			return Port >= PortI.Port;
		}

		bool operator>(const GPIOFunctionPortStruct &PortI) const {
			return Port > PortI.Port;
		}

		bool operator<(const GPIOFunctionPortStruct &PortI) const {
			return Port < PortI.Port;
		}
	};

	class GPIOFunction : public EventHandler {


		friend class GPIOController;

		public:
			GPIOFunction();
			virtual ~GPIOFunction();

			bool IsInitialized ();

		protected:
			virtual long long RunFunction(void *Data = 0) = 0;

			bool ClearMeFromQueue();
			bool NeedsUpdate(void *Data = 0);
			void MapPort (int Port, GPIOFunctionPortStructDirection Direction);
			void MapPorts (vector<GPIOFunctionPortStruct> Ports);
			int GPIOCommand(int Port, int Bit = GPIO::UNDEFINED);
			int GPIOCommandF(int Port, float Bit);

		private:

			// Vars
			GPIOController* _Controller;
			GPIOFunctionPortStruct _Port;
			vector<GPIOFunctionPortStruct> _Ports;

			// Methods
			void SetGPIOController (GPIOController* Controller);
			GPIOFunctionPortStruct MapPort();
			vector<GPIOFunctionPortStruct> MapPorts ();
			void PortRejected(GPIOFunctionPortStruct Port);
			void PortsRejected(vector<int>Ports);
	};

} /* namespace GPIO */

#endif /* GPIOFUNCTION_H_ */
