/*
 * 		EmotionTimeout.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EMOTIONTIMEOUT_H_
#define EMOTIONTIMEOUT_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class EmotionTimeoutEvent : public Event {
		public:
			EmotionTimeoutEvent() {};
			virtual ~EmotionTimeoutEvent() {};
	};

	class EmotionTimeout : public StateMachine::StateInterface {
		public:
			EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion, int Timeout, StateMachine::StateInterface *NextCall);
			virtual ~EmotionTimeout();

		private:
			GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion;
			int Timeout;
			StateMachine::StateInterface *NextCall;

			void ReceiveEmotionTimeoutEvent (const EmotionTimeoutEvent *Event);

		protected:
			virtual void Initialize();
	};

	class ServoTimeoutEvent : public Event {
		public:
			ServoTimeoutEvent() {};
			virtual ~ServoTimeoutEvent() {};
	};

	class ServoTimeout : public StateMachine::StateInterface {
		public:
			ServoTimeout(GPIO::InternalFunctions::GPIOServoFunctionContainer Position, int Timeout, StateMachine::StateInterface *NextCall, bool Released = true);
			virtual ~ServoTimeout();

		private:
			GPIO::InternalFunctions::GPIOServoFunctionContainer ServoPosition;
			int Timeout;
			StateMachine::StateInterface *NextCall;
			bool Released;

			void ReceiveServoTimeoutEvent (const ServoTimeoutEvent *Event);

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* EMOTIONTIMEOUT_H_ */
