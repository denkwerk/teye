/*
 * 		Feeling.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Feeling.h"

#include "DetectMotion.h"

namespace EXState {

Feeling::Feeling() : StateInterface("Feeling"), Stop(false) {
	// TODO Auto-generated constructor stub

	RegisterEventFunc(this, &Feeling::ReceiveFeelingTimeoutEvent);
}

Feeling::~Feeling() {
	// TODO Auto-generated destructor stub

	UnRegisterEventFunc(this, &Feeling::ReceiveFeelingTimeoutEvent);
}

void Feeling::Initialize() {
	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad);
	SendEvent(new FeelingTimeoutEvent(), 5000);
}


// Events

void Feeling::ReceiveFeelingTimeoutEvent (const FeelingTimeoutEvent *Event) {
	Stop = true;
}

void Feeling::ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event) {

	if (Stop == false) {
		if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionSad) {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionAngry));
		} else if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionAngry) {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad));
		}
	} else {
		Context->SetState(new EXState::DetectMotion());
	}

	StateInterface::ReceiveServoValueEvent(Event);
}

} /* namespace EXState */
