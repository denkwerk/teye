/*
 * 		Listening.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Listening.h"

#include "Sleep.h"
#include "TryCall.h"
#include "RingRing.h"
#include "Settings.h"
#include "Feeling.h"
#include "Exit.h"
#include "EmotionTimeout.h"

namespace EXState {

Listening::Listening() : StateInterface("Listening") {
	// TODO Auto-generated constructor stub

#ifndef _WITHOUT_SPEECH_RECOGNITION

	// NEUE Liste
	ListeningMap["ANRUF AUFBAUEN"]								= "CALL";
	ListeningMap["BERLIN ANRUFEN"]								= "CALL";
	ListeningMap["KÖLN ANRUFEN"]								= "CALL";
	ListeningMap["VERBINDUNG AUFBAUEN"]							= "CALL";
	ListeningMap["ANRUFEN"]										= "CALL";
	ListeningMap["BERLIN KONTAKTIEREN"]							= "CALL";
	ListeningMap["KÖLN KONTAKTIEREN"]							= "CALL";
	ListeningMap["RUFE BERLIN AN"]								= "CALL";
	ListeningMap["RUFE KÖLN AN"]								= "CALL";
	ListeningMap["RUFE AN"]										= "CALL";
	ListeningMap["ICH MÖCHTE BERLIN ANRUFEN"]					= "CALL";
	ListeningMap["ICH MÖCHTE KÖLN ANRUFEN"]						= "CALL";
	ListeningMap["RUF BITTE BERLIN AN"]							= "CALL";
	ListeningMap["RUF BITTE KÖLN AN"]							= "CALL";
	ListeningMap["BITTE VERBINDUNG AUFBAUEN"]					= "CALL";
	ListeningMap["DENKWERK BERLIN ANRUFEN"]						= "CALL";
	ListeningMap["DENKWERK KÖLN ANRUFEN"]						= "CALL";
	ListeningMap["DENKWERK ANRUFEN"]							= "CALL";
	ListeningMap["DENKWERK BERLIN KONTAKTIEREN"]				= "CALL";
	ListeningMap["DENKWERK KÖLN KONTAKTIEREN"]					= "CALL";
	ListeningMap["DENKWERK KONTAKTIEREN"]						= "CALL";
	ListeningMap["RUFE DAS DENKWERK BERLIN AN"]					= "CALL";
	ListeningMap["RUFE DAS DENKWERK KÖLN AN"]					= "CALL";
	ListeningMap["RUFE DAS DENKWERK AN"]						= "CALL";
	ListeningMap["ICH MÖCHTE DAS DENKWERK BERLIN ANRUFEN"]		= "CALL";
	ListeningMap["ICH MÖCHTE DAS DENKWERK KÖLN ANRUFEN"]		= "CALL";
	ListeningMap["ICH MÖCHTE DAS DENKWERK ANRUFEN"]				= "CALL";
	ListeningMap["RUF BITTE DAS DENKWERK BERLIN AN"]			= "CALL";
	ListeningMap["RUF BITTE DAS DENKWERK KÖLN AN"]				= "CALL";
	ListeningMap["RUF BITTE DAS DENKWERK AN"]					= "CALL";
	ListeningMap["BITTE VERBINDUNG ZUM DENKWERK AUFBAUEN"]		= "CALL";

#endif
}

Listening::~Listening() {
	// TODO Auto-generated destructor stub
}

void Listening::Initialize() {
	PrintText(std::string("SpeechCommands: ").append(GetSpeechCommandsAsString()), true);
	PrintText("Command (settings/call/exit) : ");
	ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello);

#if _OPEN_CV
	if (ImageRController != 0) {
		if (ImageRController->StartRecognition()) {
			if (ImageRController->EnableVideoOutput("/home/pi/applications/EXX2/VideoOut.mpeg")) {
				bool Activated = true;
			}
		}
	}
#endif
}

void Listening::GoAway() {

}

#ifndef _WITHOUT_SPEECH_RECOGNITION

void Listening::ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event) {

	std::string SpeakType = "None";
	if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeStartRecognize) {
		SpeakType = "Start Recognize";
	} else if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognize) {
		SpeakType = "Recognize";
	} else if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeStopRecognize) {
		SpeakType = "Stop Recognize";
	} else if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognizeStopped) {
		SpeakType = "Recognize Stopped";
	} else if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognizeFailed) {
		SpeakType = "Recognize Failed";
	}
	PrintText(std::string("###DEBUG### SpeakType: ").append(SpeakType), true);

	if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeRecognizeStopped && _DetectedString.length() != 0) {
		string SpkAction;
		if (ComputeSpeechCommands(&SpkAction, _DetectedString, ListeningMap, false)) {

			if (SpkAction == "CALL") {
				if (SIPController != 0 && SIPController->IsInitialized()) {
					if (SIPController->SIPCallToNumber(ApplicationSettings->_SIP_Config_Receiver)) {
						Context->SetState(new TryCall());
					}
				}
			} else if (SpkAction == "EXIT") {
				Context->SetState(new EXState::Exit());
			} else if (SpkAction == "HOWAREYOU") {
				Context->SetState(new EXState::Feeling());
			} else if (SpkAction == "SIPFREE") {

			} else if (SpkAction == "!SIPFREE") {

			} else {
				Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new Listening()));
			}
		} else {
			Context->SetState(new EXState::EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new Listening()));
		}
	}
}

void Listening::ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event) {


	_DetectedString = Event->DetectedString;
	SpeechController->StopDetection();


	StateInterface::ReceiveSpeakRecognizeEvent(Event);
}
#endif

void Listening::ReceiveServoValueEvent (const ServoValueEvent *Event) {

	if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionHello) {
		PrintText("###DEBUG### ServoEmotion: HELLO", true);
#ifndef _WITHOUT_SPEECH_RECOGNITION
		if (SpeechController->StartDetection() == false) {

		}
#endif
	}
}

void Listening::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {
	if (Event->_MotionStatus == MotionDetection::MotionStatusNoPeopleInRange) {
		Context->SetState(new Sleep());
	}

	StateInterface::ReceiveMotionEvent(Event);
}

void Listening::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted) {

	if (Event->CallState == SIP::SIPAccountStateIncomingCall) {
		Context->SetState(new RingRing());
	}

	StateInterface::ReceiveSIPCallEvent(Event, true);
}

void Listening::ReceiveKeyEvent (const KeyEvent *Event) {

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
void Listening::ReceiveImageEvent (const ImageRecognition::ImageEvent *Event) {

}

void Listening::ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event) {
	if (Event->ComputedFace.size().width == 0) {
		Context->SetState(new Sleep());
	} else {
		int Size = Event->DetectedSmiles.size();
		if (ImageRController->GetAverageSmileIntensity() > 0.0) {
			ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionAngry);
			//Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello, 1000, new Listening()));
		} else {
			ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad);
			//Context->SetState(new EmotionTimeout(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad, 1000, new Listening()));
		}
	}
}
#endif

} /* namespace EXState */
