/*
 * 		TryCall.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "TryCall.h"

#include "Calling.h"
#include "EmotionTimeout.h"
#include "Listening.h"

namespace EXState {

TryCall::TryCall(GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion) : StateInterface("TryCall"), StandardEmotion(Emotion), RingCount(0) {
	// TODO Auto-generated constructor stub

#ifndef _WITHOUT_SPEECH_RECOGNITION

	// NEUE Liste
	ListeningMap["AUFLEGEN"]								= "ABORT";
	ListeningMap["ANRUF BEENDEN"]							= "ABORT";
	ListeningMap["VERBINDUNG BEENDEN"]						= "ABORT";
	ListeningMap["VERBINDUNG TRENNEN"]						= "ABORT";
	ListeningMap["BITTE AUFLEGEN"]							= "ABORT";
	ListeningMap["BITTE ANRUF BEENDEN"]						= "ABORT";
	ListeningMap["BITTE VERBINDUNG BEENDEN"]				= "ABORT";
	ListeningMap["BITTE VERBINDUNG TRENNEN"]				= "ABORT";
	ListeningMap["LEG AUF"]									= "ABORT";

#endif
}

TryCall::~TryCall() {
	// TODO Auto-generated destructor stub
}

void TryCall::Initialize() {
	PrintText(std::string("SpeechCommands: ").append(GetSpeechCommandsAsString()), true);
	PrintText("Command (hangup) : ");

#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (SpeechController != 0 && SpeechController->DetectionModeOn() == false) {
		SpeechController->StartDetection();
	}
#endif

	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight);
}


// Events

void TryCall::ReceiveSIPEvent (const SIP::SIPEvent *Event) {

	StateInterface::ReceiveSIPEvent(Event);
}

void TryCall::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted) {

	StateInterface::ReceiveSIPCallEvent(Event);
}

void TryCall::ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event) {

	//int Delay = 0;
	if (Event->StateType == SIP::SIPCallStateTypeConnected) {

		Context->SetState(new Calling(StandardEmotion));
	} else if (Event->StateType == SIP::SIPCallStateTypeDisconnected) {
		Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 2000, new EXState::Listening()));
	}

	StateInterface::ReceiveSIPCallStateEvent(Event);
}

void TryCall::ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event) {

	StateInterface::ReceiveSIPStateEvent(Event);
}

void TryCall::ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event) {

	if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight) {
		if (RingCount == 5) {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello), 10000);
			RingCount = 0;
		} else {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello));
			RingCount++;
		}
	} else if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionHello) {
		SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight));
	}

	StateInterface::ReceiveServoValueEvent(Event);
}

#ifndef _WITHOUT_SPEECH_RECOGNITION

void TryCall::ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event) {
	if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognizeStopped && _DetectedString.length() != 0) {
		string SpkAction;
		if (ComputeSpeechCommands(&SpkAction, _DetectedString, ListeningMap, false)) {

			if (SpkAction == "ABORT") {
				if (SIPController != 0 && SIPController->IsInitialized()) {
					SIPController->SIPHangUpCall();
				}
			} else {
				Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new TryCall(StandardEmotion)));
			}
		} else {
			Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new TryCall(StandardEmotion)));
		}
	}
}
void TryCall::ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event) {

	_DetectedString = Event->DetectedString;
	SpeechController->StopDetection();


	StateInterface::ReceiveSpeakRecognizeEvent(Event);
}
#endif

void TryCall::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		if (this->MapInput(Event, Result)) {
			if (Result == "hangup") {
				if (SIPController != 0 && SIPController->IsInitialized()) {
					SIPController->SIPHangUpCall();
				}
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}

} /* namespace EXState */
