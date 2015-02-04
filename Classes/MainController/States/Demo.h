/*
 * 		Demo.h
 *
 *  	Created on: 09.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef DEMO_H_
#define DEMO_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Demo : public StateMachine::StateInterface {
		public:
			Demo();
			virtual ~Demo();

			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);

		private:
			void Initialize();
			GPIO::InternalFunctions::GPIOServoFunctionEmotion RandomEmotion ();

	};

} /* namespace EXState */

#endif /* DEMO_H_ */
