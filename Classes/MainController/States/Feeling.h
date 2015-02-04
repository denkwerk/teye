/*
 * 		Feeling.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef FEELING_H_
#define FEELING_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class FeelingTimeoutEvent : public Event {
		public:
			FeelingTimeoutEvent() {};
			virtual ~FeelingTimeoutEvent() {};
	};

	class Feeling : public StateMachine::StateInterface {
		public:
			Feeling();
			virtual ~Feeling();

			virtual void ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event);

		private:
			void ReceiveFeelingTimeoutEvent (const FeelingTimeoutEvent *Event);

			bool Stop;

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* FEELING_H_ */
