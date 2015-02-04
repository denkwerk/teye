/*
 * 		GPIOMCP3008Function.h
 *
 *  	Created on: 21.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef GPIOMCP3008FUNCTION_H_
#define GPIOMCP3008FUNCTION_H_

#include "../GPIO/GPIOFunction.h"

#include <pthread.h>

namespace GPIO {
	namespace InternalFunctions {

		enum EXSignalIOMotionType {
			EXSignalIOMotionTypeNone,
			EXSignalIOMotionTypeInitDone,
			EXSignalIOMotionTypeStartPolling,
			EXSignalIOMotionTypeStopPolling,
			EXSignalIOMotionTypeNewValue,
			EXSignalIOMotionTypeNewValueCleaned
		};

		class MotionRawEvent : public Event {
			public:
				MotionRawEvent() : RawMotionValue(0), RawMotionType(EXSignalIOMotionTypeNone) {};
				MotionRawEvent(EXSignalIOMotionType Type) : RawMotionValue(0), RawMotionType(Type) {};
				MotionRawEvent(int Value, EXSignalIOMotionType Type = EXSignalIOMotionTypeNewValue) : RawMotionValue(Value), RawMotionType(Type) {};

				int RawMotionValue;
				EXSignalIOMotionType RawMotionType;
		};

		struct GPIOMCP3008FunctionPorts {
			int CLK;
			int DOut;
			int DIn;
			int CS;
			int Input;

			GPIOMCP3008FunctionPorts() : CLK(-1), DOut(-1), DIn(-1), CS(-1), Input(0) {};
			GPIOMCP3008FunctionPorts(int CLK, int DOut, int DIn, int CS) : CLK(CLK), DOut(DOut), DIn(DIn), CS(CS), Input(0) {};
			GPIOMCP3008FunctionPorts(int CLK, int DOut, int DIn, int CS, int Input) : CLK(CLK), DOut(DOut), DIn(DIn), CS(CS), Input(Input) {};
			~GPIOMCP3008FunctionPorts() {};
		};

		class GPIOMCP3008Function: public GPIOFunction {

			public:
				static GPIOMCP3008Function* GPIOMCP3008FunctionWithPort (GPIOMCP3008FunctionPorts Ports);

				// MCP3008 Methods
				bool StartPolling();
				bool StopPolling();
				int GetValue ();

				void SetSignalCallback (void (*cb)(void *Signal));

			private:

				// Internal Variables
				GPIOMCP3008FunctionPorts	_Ports;
				pthread_t					_thread;
				pthread_mutex_t				_mutex_task_wait;
				volatile bool				_exit;
				volatile int				_value;

				// Private De -and Constructors
				GPIOMCP3008Function();
				virtual ~GPIOMCP3008Function();

				// GPIOFunction Interface Methods
				long long RunFunction(void *Data = 0);

				// Internal Methods
				static void *RunPolling (void *self);
				void SetPorts (GPIOMCP3008FunctionPorts Ports);

				// Helper
				void Clock_Write (int clockpin, int out, int value);
				int Clock_Read(int clockpin, int in);
				void Select_Chip (int bin[], int inputnum);
				int Power_Of_2 (int exp);
		};

	} /* namespace InternalFunctions */
} /* namespace GPIO */

#endif /* GPIOMCP3008FUNCTION_H_ */
