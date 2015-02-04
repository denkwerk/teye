/*
 * 		Sleep.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Sleep.h"

#include "Listening.h"
#include "Settings.h"
#include "RingRing.h"
#include "TryCall.h"
#include "Exit.h"

namespace EXState {

Sleep::Sleep() : StateInterface("Sleep") {
	// TODO Auto-generated constructor stub

}

Sleep::~Sleep() {
	// TODO Auto-generated destructor stub
}

void Sleep::Initialize() {
	PrintText("Command (settings/call/exit) : ");
	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSleep);
#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (SpeechController->StopDetection() == false) {

	}
#endif
#if _OPEN_CV
	if (ImageRController != 0) {
		if (ImageRController->StartRecognition()) {
			ImageRController->EnableVideoOutput("VideoOut.mpeg");
		}
	}
#endif
}

void Sleep::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {
	if (Event->_MotionStatus == MotionDetection::MotionStatusPeopleInRange) {
		Context->SetState(new Listening());
	} else if (Event->_MotionStatus == MotionDetection::MotionStatusNoPeopleInRange) {

	}

	StateInterface::ReceiveMotionEvent(Event);
}

void Sleep::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted) {

	if (Event->CallState == SIP::SIPAccountStateIncomingCall) {
		Context->SetState(new RingRing());
	}

	StateInterface::ReceiveSIPCallEvent(Event, true);
}

void Sleep::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		if (this->MapInput(Event, Result)) {
			if (Result == "settings") {
				NewLine();
				Context->SetState(new Settings());
			} else if (Result == "call") {
				NewLine();
				if (SIPController != 0 && SIPController->IsInitialized()) {
					if (SIPController->SIPCallToNumber(ApplicationSettings->_SIP_Config_Receiver)) {
						Context->SetState(new TryCall());
					}
				}
			} else if (Result == "exit") {
				NewLine();
				Context->SetState(new EXState::Exit());
			} else {
				CleanUp(Result);
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}

#ifdef _OPEN_CV
void Sleep::ReceiveImageEvent (const ImageRecognition::ImageEvent *Event) {

}

void Sleep::ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event) {
	if (Event->ComputedFace.size().width > 0) {
		Context->SetState(new Listening());
	}
}
#endif

} /* namespace EXState */
