/*
 * 		EXMainController.h
 *
 *  	Created on: 21.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXMAINCONTROLLER_H_
#define EXMAINCONTROLLER_H_

#include "../Configuration/Config.h"

#include "../../Additionals/ArgvParser/argvparser.h"

#include "../UI/UIWindow.h"

#ifndef _WITHOUT_SPEECH_RECOGNITION
#include "../SpeechDetection/EXSpeechController.h"
#endif

#include "../SIP/SIPController.h"
#include "../GPIO/GPIOController.h"

#include "../GPIOFunctions/GPIOServoFunction.h"
#include "../GPIOFunctions/GPIOMCP3008Function.h"

#include "EXServoCalibration.h"
#include "EXServoManuallyDrive.h"

#include "../Motion/EXMotionDetection.h"

#include "../Event/EventHandler.h"

#include "../StateMachine/StateContext.h"
#include "States/Init.h"

#include <map>

namespace EX {

	enum EXExtremType {
		EXExtremTypeNone = -1,
		EXExtremTypeMin = 0,
		EXExtremTypeMax
	};


	class EXMainController : public EventHandler {
		public:
			EXMainController(int argc, char *argv[], std::string AbsolutePath, CommandLineProcessing::ArgvParser *argParser = 0);
			virtual ~EXMainController();

			// Module Setters
			void setWindowController (UI::UIWindow *WindowController);
#ifndef _WITHOUT_SPEECH_RECOGNITION
			void setSpeechController(EXSpeech::EXSpeechController *SpeechController);
#endif
#ifdef _OPEN_CV
			void setIRController(ImageRecognition::EXImageRecognition *ImageRecognitionController);
#endif
			void setSIPController (SIP::SIPController *_SIPControllerI);
			void setIOController (GPIO::GPIOController *IOController);

			// Events
#ifndef _WITHOUT_SPEECH_RECOGNITION
			void ReceiveSpeakEvent				(const EXSpeech::SpeechEvent *Event);
			void ReceiveSpeakRecognizeEvent		(const EXSpeech::SpeechRecognizeEvent *Event);
#endif
#ifdef _OPEN_CV
			void ReceiveImageEvent				(const ImageRecognition::ImageEvent *Event);
			void ReceiveImageRecognizeEvent		(const ImageRecognition::ImageRecognizeEvent *Event);
#endif
			void ReceiveRawMotionData			(const MotionRawEvent *Event);
			void ReceiveMotionEvent				(const MotionDetection::MotionEvent *Event);
			void ReceiveMotionCalibrationEvent	(const MotionDetection::MotionCalibrationEvent *Event);
			void ReceiveServoEvent				(const ServoEvent *Event);
			void ReceiveServoValueEvent			(const ServoValueEvent *Event);
			void ReceiveServoEmotionEvent		(const ServoEmotionEvent*Event);
			void ReceiveSystemEvent				(const SystemEvent *Event);
			void ReceiveKeyEvent				(const KeyEvent *Event);
			void ReceiveSIPEvent				(const SIP::SIPEvent *Event);
			void ReceiveSIPCallEvent			(const SIP::SIPCallEvent *Event);
			void ReceiveSIPCallStateEvent		(const SIP::SIPCallStateEvent *Event);
			void ReceiveSIPStateEvent			(const SIP::SIPStateEvent *Event);
			void ReceiveWindowEvent				(const UI::WindowEvent *Event);

		private:

			// Variables
			UI::UIWindow									*_WindowController;
#ifndef _WITHOUT_SPEECH_RECOGNITION
			EXSpeech::EXSpeechController						*_SpeakController;
#endif
#ifdef _OPEN_CV
			ImageRecognition::EXImageRecognition			*_IRController;
#endif
			SIP::SIPController								*_SIPController;
			GPIO::GPIOController							*_IOController;

			StateMachine::StateContext						*_StateContext;

			CommandLineProcessing::ArgvParser 				*ARGParser;

			GPIO::InternalFunctions::GPIOMCP3008Function	*_MCP3008Function;
			GPIO::InternalFunctions::GPIOServoFunction		*_ServoFunction;
			MotionDetection::EXMotionDetection				*_MotionDetection;

			// Path
			std::string										AbsolutePath;

			// Motion Debug
			int												_MotionAverageValue[MOTION_AVERAGE];
			int												_MotionAveragePoint;
			int												_MotionAverageLastValue;

			// Helper
			void GoDown();
	};

} /* namespace EX */

#endif /* EXMAINCONTROLLER_H_ */
