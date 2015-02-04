/*
 * 		EmotionTimeout.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EmotionTimeout.h"

namespace EXState {

EmotionTimeout::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion, int Timeout, StateMachine::StateInterface *NextCall) : StateInterface("EmotionTimeout"), Emotion(Emotion), Timeout(Timeout), NextCall(NextCall) {
	// TODO Auto-generated constructor stub

	RegisterEventFunc(this, &EmotionTimeout::ReceiveEmotionTimeoutEvent);
}

EmotionTimeout::~EmotionTimeout() {
	// TODO Auto-generated destructor stub

	UnRegisterEventFunc(this, &EmotionTimeout::ReceiveEmotionTimeoutEvent);
}

void EmotionTimeout::Initialize() {
	ServoFunction->SetEmotion(Emotion);
	SendEvent(new EmotionTimeoutEvent(), Timeout);
}


// Events

void EmotionTimeout::ReceiveEmotionTimeoutEvent (const EmotionTimeoutEvent *Event) {

	Context->SetState(NextCall);
}


ServoTimeout::ServoTimeout(GPIO::InternalFunctions::GPIOServoFunctionContainer Position, int Timeout, StateMachine::StateInterface *NextCall, bool Released) : StateInterface("ServoTimeout"), ServoPosition(Position), Timeout(Timeout), NextCall(NextCall), Released(Released) {
	// TODO Auto-generated constructor stub

	RegisterEventFunc(this, &ServoTimeout::ReceiveServoTimeoutEvent);
}

ServoTimeout::~ServoTimeout() {
	// TODO Auto-generated destructor stub

	UnRegisterEventFunc(this, &ServoTimeout::ReceiveServoTimeoutEvent);
}

void ServoTimeout::Initialize() {
	ServoFunction->SetContainer(ServoPosition);
	SendEvent(new ServoTimeoutEvent(), Timeout);
}


// Events

void ServoTimeout::ReceiveServoTimeoutEvent (const ServoTimeoutEvent *Event) {

	Context->SetState(NextCall, Released);
}

} /* namespace EXState */
