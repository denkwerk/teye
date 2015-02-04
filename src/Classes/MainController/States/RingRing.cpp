/*
 * 		RingRing.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "RingRing.h"

#include "Calling.h"
#include "EmotionTimeout.h"
#include "DetectMotion.h"

namespace EXState {

RingRing::RingRing() : StateInterface("RingRing"), RingCount(0) {
	// TODO Auto-generated constructor stub

#ifndef _WITHOUT_SPEECH_RECOGNITION

	// NEUE Liste
	ListeningMap["ANRUF ANNEHMEN"]							= "ANSWER";
	ListeningMap["ABHEBEN"]									= "ANSWER";
	ListeningMap["WER IST DA"]								= "ANSWER";
	ListeningMap["KONTAKT ERSTELLEN"]						= "ANSWER";
	ListeningMap["VERBINDEN"]								= "ANSWER";
	ListeningMap["ANNEHMEN"]								= "ANSWER";
	ListeningMap["BITTE ANRUF ANNEHMEN"]					= "ANSWER";
	ListeningMap["BITTE ABHEBEN"]							= "ANSWER";
	ListeningMap["BITTE KONTAKT HERSTELLEN"]				= "ANSWER";
	ListeningMap["BITTE VERBINDEN"]							= "ANSWER";
	ListeningMap["BITTE ANNEHMEN"]							= "ANSWER";

#endif
}

RingRing::~RingRing() {
	// TODO Auto-generated destructor stub
}

void RingRing::Initialize() {
	PrintText(std::string("SpeechCommands: ").append(GetSpeechCommandsAsString()), true);
	PrintText("Command (yes/no) : ");

	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft);

	SIPController->StartRingRing(true);

#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (SpeechController != 0 && SpeechController->DetectionModeOn() == false) {
		if (SpeechController->StartDetection() == true) {
		}
	}
#endif
}

void RingRing::GoAway () {
	SIPController->StartRingRing(false);
}


// Events

void RingRing::ReceiveSIPEvent (const SIP::SIPEvent *Event) {

	StateInterface::ReceiveSIPEvent(Event);
}

void RingRing::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted) {

	StateInterface::ReceiveSIPCallEvent(Event);
}

void RingRing::ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event) {

	if (Event->CallState == SIP::SIPCallStateCall && Event->StateType == SIP::SIPCallStateTypeDisconnected) {
		Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 5000, new DetectMotion()));
	}

	StateInterface::ReceiveSIPCallStateEvent(Event);
}

void RingRing::ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event) {

	StateInterface::ReceiveSIPStateEvent(Event);
}

#ifndef _WITHOUT_SPEECH_RECOGNITION
void RingRing::ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event) {

	string SpkAction;

	if (ComputeSpeechCommands(&SpkAction, Event->DetectedString, ListeningMap, false)) {

		if (SpkAction == "ANSWER") {
			if (SIPController != 0 && SIPController->IsInitialized()) {
				SIP::SIPCall *IncomingCall = SIPController->SIPCurrentCall();
				if (SIPController->SIPAcceptIncomingCall(IncomingCall)) {
					Context->SetState(new Calling(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight));
				} else {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 5000, new DetectMotion()));
				}
			}
		} else if (SpkAction == "REJECT") {
			if (SIPController != 0 && SIPController->IsInitialized()) {
				SIP::SIPCall *IncomingCall = SIPController->SIPCurrentCall();
				if (SIPController->SIPRejectIncomingCall(IncomingCall)) {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello, 5000, new DetectMotion()));
				} else {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 5000, new DetectMotion()));
				}
			}
		}
	}

	StateInterface::ReceiveSpeakRecognizeEvent(Event);
}
#endif

void RingRing::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		if (this->MapInput(Event, Result)) {
			if (Result == "yes") {
				SIP::SIPCall *IncomingCall = SIPController->SIPCurrentCall();
				if (SIPController->SIPAcceptIncomingCall(IncomingCall)) {
					Context->SetState(new Calling(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft));
				} else {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 5000, new DetectMotion()));
				}
			} else if (Result == "no") {
				SIP::SIPCall *IncomingCall = SIPController->SIPCurrentCall();
				if (SIPController->SIPRejectIncomingCall(IncomingCall)) {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello, 5000, new DetectMotion()));
				} else {
					Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 5000, new DetectMotion()));
				}
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}

void RingRing::ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event) {
	if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft) {
		if (RingCount == 5) {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello), 10000);
			SIPController->StartRingRing(false);
			RingCount = 0;
		} else {
			SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello));
			RingCount++;
		}
	} else if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionHello) {
		SendEvent(new ServoEmotionControlEvent(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft));
		SIPController->StartRingRing(true);
	}

	StateInterface::ReceiveServoValueEvent(Event);
}

} /* namespace EXState */
