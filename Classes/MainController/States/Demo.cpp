/*
 * 		Demo.cpp
 *
 *  	Created on: 09.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Demo.h"

#include "EmotionTimeout.h"

namespace EXState {

Demo::Demo() : StateInterface("Demo") {
	// TODO Auto-generated constructor stub

}

Demo::~Demo() {
	// TODO Auto-generated destructor stub
}

void Demo::Initialize() {
	GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion = RandomEmotion();
	ServoFunction->SetEmotion(Emotion);
}


void Demo::ReceiveServoValueEvent (const ServoValueEvent *Event) {
	int Timeout = rand() % 10000;
	Timeout = (Timeout < 1000 ? 1000 : (Timeout > 10000 ? 10000 : Timeout));
	Context->SetState(new EmotionTimeout(RandomEmotion(), Timeout, new Demo()));
}

GPIO::InternalFunctions::GPIOServoFunctionEmotion Demo::RandomEmotion () {
	GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion = GPIOServoFunctionContainer::Emotion(ServoFunction->GetDegree());
	int EmotionInt = (int)Emotion;
	while ((GPIO::InternalFunctions::GPIOServoFunctionEmotion)EmotionInt == Emotion) {
		EmotionInt = rand() % 7;
	}
	EmotionInt = (EmotionInt < 0 ? 0 : (EmotionInt > 7 ? 7 : EmotionInt));
	return (GPIO::InternalFunctions::GPIOServoFunctionEmotion)EmotionInt;
}

} /* namespace EXState */
