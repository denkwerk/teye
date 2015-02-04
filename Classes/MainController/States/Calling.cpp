/*
 * 		Calling.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Calling.h"

#include "EmotionTimeout.h"
#include "DetectMotion.h"

namespace EXState {

Calling::Calling(GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion) : StateInterface("Calling"), StandardEmotion(Emotion) {
	// TODO Auto-generated constructor stub

	// NEUE Liste
	ListeningMap["DU BIST DOOF"]								= "ANGRY";
	ListeningMap["DU ARSCH"]									= "ANGRY";
	ListeningMap["SCHEISSE"]									= "ANGRY";
	ListeningMap["FUCK"]										= "ANGRY";
	ListeningMap["SCHADE"]										= "SAD";

	ListeningMap["TEYE AUFLEGEN"]								= "HANGUP";
	ListeningMap["TEYE ANRUF BEENDEN"]							= "HANGUP";
	ListeningMap["TEYE VERBINDUNG BEENDEN"]						= "HANGUP";
	ListeningMap["TEYE VERBINDUNG TRENNEN"]						= "HANGUP";
	ListeningMap["TEYE BITTE AUFLEGEN"]							= "HANGUP";
	ListeningMap["TEYE BITTE ANRUF BEENDEN"]					= "HANGUP";
	ListeningMap["TEYE BITTE VERBINDUNG TRENNEN"]				= "HANGUP";
	ListeningMap["TEYE BITTE VERBINDUNG BEENDEN"]				= "HANGUP";
	ListeningMap["TEYE LEG AUF"]								= "HANGUP";
}

Calling::~Calling() {
	// TODO Auto-generated destructor stub
}

void Calling::Initialize() {
	PrintText(std::string("SpeechCommands: ").append(GetSpeechCommandsAsString()), true);
	PrintText("Command (hangup) : ");

	ServoFunction->SetEmotion(StandardEmotion);
#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (SpeechController != 0 && SpeechController->DetectionModeOn() == false) {
		SpeechController->StartDetection();
	}
#endif
}


// Events

void Calling::ReceiveSIPEvent (const SIP::SIPEvent *Event) {

	StateInterface::ReceiveSIPEvent(Event);
}

void Calling::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted) {

	StateInterface::ReceiveSIPCallEvent(Event);
}

void Calling::ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event) {

	if (Event->CallState == SIP::SIPCallStateCall && Event->StateType == SIP::SIPCallStateTypeDisconnected) {
		Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 2000, new DetectMotion()));
	}

	StateInterface::ReceiveSIPCallStateEvent(Event);
}

void Calling::ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event) {

	StateInterface::ReceiveSIPStateEvent(Event);
}

#ifndef _WITHOUT_SPEECH_RECOGNITION

void Calling::ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event) {
	if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognizeStopped && _DetectedString.length() != 0) {
		string SpkAction;
		if (ComputeSpeechCommands(&SpkAction, _DetectedString, ListeningMap, false)) {

			if (SpkAction == "HANGUP") {
				if (SIPController != 0 && SIPController->IsInitialized()) {
					if (SIPController->SIPHangUpCall()) {
						Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios, 2000, new DetectMotion()));
					} else {
						Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 2000, new DetectMotion()));
					}
				}
			} else {
				Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new Calling(StandardEmotion)));
			}
		} else {
			Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new Calling(StandardEmotion)));
		}
	}
}

void Calling::ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event) {

	_DetectedString = Event->DetectedString;
	SpeechController->StopDetection();


	StateInterface::ReceiveSpeakRecognizeEvent(Event);
}
#endif

void Calling::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		if (this->MapInput(Event, Result)) {
			if (Result == "hangup") {
				if (SIPController->SIPHangUpCall()) {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios, 2000, new DetectMotion()));
				} else {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 2000, new DetectMotion()));
				}
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);

	StateInterface::ReceiveKeyEvent(Event);
}

} /* namespace EXState */
