/*
 * 		Settings.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Settings.h"

#include "ServoSettings.h"
#include "DetectMotion.h"
#include "Motion.h"

namespace EXState {

Settings::Settings(StateMachine::StateInterface *BackState) : StateInterface("Settings"), BackState(BackState) {
	// TODO Auto-generated constructor stub

}

Settings::~Settings() {
	// TODO Auto-generated destructor stub
}

void Settings::Initialize() {

	if (SpeechController != 0 && SpeechController->DetectionModeOn()) {
		SpeechController->StopDetection();
	}

	PrintText("hello -> settings", true);
	PrintText("Command (servo/motion/back/<-) : ");
	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios);
}


// Events

void Settings::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "back";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (Result == "servo") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::Servo());
			} else if (Result == "motion") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::Motion());
			} else if (Result == "back") {
				NewLine();
				if (BackState != 0) {
					Context->SetState(BackState);
				} else {
					Context->SetState(new EXState::DetectMotion());
				}
			} else {
				CleanUp(Result);
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}

} /* namespace EXState */
