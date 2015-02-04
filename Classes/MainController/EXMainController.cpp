/*
 * 		EXMainController.cpp
 *
 *  	Created on: 21.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EXMainController.h"

#include <string>
#include <stdio.h>

#define IO_CONTROL_BYPASS false
#define SPEAK_CONTROL_BYPASS false
#define SIP_CONTROL_BYPASS false

namespace EX {

#ifdef _WITHOUT_SPEECH_RECOGNITION
#ifdef _OPEN_CV
	EXMainController::EXMainController(int argc, char *argv[], std::string AbsolutePath, CommandLineProcessing::ArgvParser *argParser) : _WindowController(0), _IRController(0), _SIPController(0), _IOController(0), _StateContext(0), ARGParser(0), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), AbsolutePath(AbsolutePath), _MotionAverageValue{0}, _MotionAveragePoint(0), _MotionAverageLastValue(0) {
#else
	EXMainController::EXMainController(int argc, char *argv[], std::string AbsolutePath, CommandLineProcessing::ArgvParser *argParser) : _WindowController(0), _SIPController(0), _IOController(0), _StateContext(0), ARGParser(0), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), AbsolutePath(AbsolutePath), _MotionAverageValue{0}, _MotionAveragePoint(0), _MotionAverageLastValue(0) {
#endif
#else
#ifdef _OPEN_CV
	EXMainController::EXMainController(int argc, char *argv[], std::string AbsolutePath, CommandLineProcessing::ArgvParser *argParser) : _WindowController(0), _SpeakController(0), _IRController(0), _SIPController(0), _IOController(0), _StateContext(0), ARGParser(0), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), AbsolutePath(AbsolutePath), _MotionAverageValue{0}, _MotionAveragePoint(0), _MotionAverageLastValue(0) {
#else
	EXMainController::EXMainController(int argc, char *argv[], std::string AbsolutePath, CommandLineProcessing::ArgvParser *argParser) : _WindowController(0), _SpeakController(0), _SIPController(0), _IOController(0), _StateContext(0), ARGParser(0), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), AbsolutePath(AbsolutePath), _MotionAverageValue{0}, _MotionAveragePoint(0), _MotionAverageLastValue(0) {
#endif
#endif
	// TODO Auto-generated constructor stub

	if (argParser != 0) {
		ARGParser = argParser;

		ARGParser->addErrorCode(0, "Success");
		ARGParser->addErrorCode(1, "Error");

		ARGParser->defineOption("io_bypass", "io_bypass. Default value: 0", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("io_bypass","i");
		ARGParser->defineOption("speak_bypass", "speak_bypass. Default value: 0", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("speak_bypass","p");
		ARGParser->defineOption("speak_bypass_detection", "speak_bypass_detection. Default value: 0", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("speak_bypass_detection","o");
		ARGParser->defineOption("sip_bypass", "sip_bypass. Default value: 0", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("sip_bypass","s");
		ARGParser->defineOption("manual", "manual. Default value: 0", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("manual","m");
		ARGParser->defineOption("continues_motion_output", "Continues Motion Output.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("continues_motion_output","c");
		ARGParser->defineOption("motion_status_output", "Motion Status Output.", CommandLineProcessing::ArgvParser::NoOptionAttribute);
		ARGParser->defineOptionAlternative("motion_status_output","n");
	}


	_MotionDetection = new MotionDetection::EXMotionDetection();
	_MotionDetection->SetThreshold(100);
	_MotionDetection->SetTimeout(10000);

	RegisterEventFunc(this, &EXMainController::ReceiveSystemEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveKeyEvent);
#ifndef _WITHOUT_SPEECH_RECOGNITION
	RegisterEventFunc(this, &EXMainController::ReceiveSpeakEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveSpeakRecognizeEvent);
#endif
#ifdef _OPEN_CV
	RegisterEventFunc(this, &EXMainController::ReceiveImageEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveImageRecognizeEvent);
#endif
	RegisterEventFunc(this, &EXMainController::ReceiveRawMotionData);
	RegisterEventFunc(this, &EXMainController::ReceiveMotionEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveMotionCalibrationEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveServoEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveServoValueEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveServoEmotionEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveSIPEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveSIPCallEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveSIPCallStateEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveSIPStateEvent);
	RegisterEventFunc(this, &EXMainController::ReceiveWindowEvent);
}

EXMainController::~EXMainController() {
	// TODO Auto-generated destructor stub
}


// Module Setter

void EXMainController::setWindowController (UI::UIWindow *WindowController) {
	_WindowController = WindowController;
	_WindowController->Initialize();
}

#ifndef _WITHOUT_SPEECH_RECOGNITION
void EXMainController::setSpeechController (EXSpeech::EXSpeechController *SpeechController) {
	_SpeakController = SpeechController;
}
#endif

#ifdef _OPEN_CV
void EXMainController::setIRController(ImageRecognition::EXImageRecognition *ImageRecognitionController) {
	_IRController = ImageRecognitionController;
}
#endif

void EXMainController::setSIPController (SIP::SIPController *_SIPControllerI) {
	_SIPController = _SIPControllerI;
}

void EXMainController::setIOController (GPIO::GPIOController *IOController) {
	_IOController = IOController;
}


// Lifecycle Signals

#ifndef _WITHOUT_SPEECH_RECOGNITION
void EXMainController::ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSpeakEvent(Event);
	}

}

void EXMainController::ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSpeakRecognizeEvent(Event);
	}

}
#endif

#ifdef _OPEN_CV

	void EXMainController::ReceiveImageEvent (const ImageRecognition::ImageEvent *Event) {
		if (_StateContext != 0) {
			_StateContext->Context()->ReceiveImageEvent(Event);
		}
	}

	void EXMainController::ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event) {
		if (_StateContext != 0) {
			_StateContext->Context()->ReceiveImageRecognizeEvent(Event);
		}
	}
#endif


// Motion Raw

void EXMainController::ReceiveRawMotionData(const MotionRawEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveRawMotionData(Event);
	}

}


// Motion Cleaned

void EXMainController::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveMotionEvent(Event);
	}

}

void EXMainController::ReceiveMotionCalibrationEvent (const MotionDetection::MotionCalibrationEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveMotionCalibrationEvent(Event);
	}

}

void EXMainController::ReceiveServoEvent (const ServoEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveServoEvent(Event);
	}

}

void EXMainController::ReceiveServoValueEvent (const ServoValueEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveServoValueEvent(Event);
	}

}

void EXMainController::ReceiveServoEmotionEvent (const ServoEmotionEvent*Event) {
	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveServoEmotionEvent(Event);
	}
}

void EXMainController::ReceiveSystemEvent (const SystemEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSystemEvent(Event);
	} else {

		if (Event->_EventType == SystemEventTypeStartup) {

#ifdef _WITHOUT_SPEECH_RECOGNITION
#ifdef _OPEN_CV
			_StateContext = new StateMachine::StateContext(_WindowController, _SIPController, _IOController, _IRController);
#else
			_StateContext = new StateMachine::StateContext(_WindowController, _SIPController, _IOController);
#endif
#else
#ifdef _OPEN_CV
			_StateContext = new StateMachine::StateContext(_WindowController, _SpeakController, _SIPController, _IOController, _IRController);
#else
			_StateContext = new StateMachine::StateContext(_WindowController, _SpeakController, _SIPController, _IOController);
#endif
#endif
			_StateContext->SetState(new EXState::Init());

		}
	}
}

void EXMainController::ReceiveKeyEvent (const KeyEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveKeyEvent(Event);
	}

}

void EXMainController::ReceiveSIPEvent (const SIP::SIPEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSIPEvent(Event);
	}

}

void EXMainController::ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSIPCallEvent(Event);
	}

}

void EXMainController::ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSIPCallStateEvent(Event);
	}

}

void EXMainController::ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveSIPStateEvent(Event);
	}

}

void EXMainController::ReceiveWindowEvent (const UI::WindowEvent *Event) {

	if (_StateContext != 0) {
		_StateContext->Context()->ReceiveWindowEvent(Event);
	}

}


// Helper

void EXMainController::GoDown() {
	bool SIPOnline = false;
	bool SpeakOnline = false;

	if (_SIPController != 0) {
		SIPOnline = true;
		if (_SIPController->IsInitialized() == false) {
			SIPOnline = false;
		} else if (_SIPController->UninitializeSIP()) {
			SIPOnline = false;
		}
	}

#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (_SpeakController != 0) {
		SpeakOnline = true;
		if (_SpeakController->IsInitialized() == false) {
			SpeakOnline = false;
		}
	}
#endif

	if (_ServoFunction != 0) {
		if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(_ServoFunction->GetDegree()) == GPIOServoFunctionEmotionSleep) {
		}
	}

	if (SIPOnline == false && SpeakOnline == false) {
		SendEvent(new SystemEvent(SystemEventTypeExit));
	}
}

} /* namespace EX */
