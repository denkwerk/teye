/*
 * 		Init.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Init.h"

#include "DetectMotion.h"
#include "Listening.h"
#include "Sleep.h"
#include "Exit.h"
#include "Errors.h"

#include "Demo.h"

namespace EXState {


// -----------------------------------------------------------------------------------------------
// Init ------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

Init::Init() : StateInterface("Init"), _MotionCalibrated(Clear) {
	// TODO Auto-generated constructor stub
}

Init::~Init() {
	// TODO Auto-generated destructor stub
}

void Init::Initialize() {
	setvbuf (stdout, NULL, _IONBF, 0);

	PrintText("say hello to teye : v1.0", true);
	NewLine();

	if (ApplicationSettings == 0) {
		PrintText("- Load Settings");
		this->LoadSettings();
		PrintText("\t\t\t\t\t[\x1b[32mdone\x1b[39m]", true);
	}

	if (MCP3008Function == 0 || ServoFunction == 0 || MotionDetection == 0) {
		PrintText("- Init IO");
		this->InitIO();
		PrintText("\t\t\t\t\t[\x1b[32mdone\x1b[39m]", true);
	}

	PrintText(std::string("- SIP_Codec: \"").append(SIPController->GetCodec((SIP::SIPController::SIPCodecType)ApplicationSettings->_SIP_Config_Codec_Type).append("\"")));
	PrintText("\t\t\t[\x1b[32mdone\x1b[39m]", true);

	NewLine();

	_MotionCalibrated |= InitStep;

	this->CheckStep();
}


// Events

void Init::ReceiveServoValueEvent (const ServoValueEvent *Event) {

	StateInterface::ReceiveServoValueEvent(Event);
}

void Init::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {
	if (Event->_MotionStatus == MotionDetection::MotionStatusCalibrationModus) {
		_MotionCalibrated |= MotionEvent;
		_MotionCalibrated |= MotionNotCalibrated;
	} else if (Event->_MotionStatus == MotionDetection::MotionStatusInitialized) {
		_MotionCalibrated |= MotionEvent;
		_MotionCalibrated |= MotionCalibrated;
	}

	this->CheckStep();
}


// Internal

void Init::LoadSettings() {
	if (SIPController != 0) {
		SIPController->SetFilename("exx_settings");
		ApplicationSettings = new SIP::PersistentStore::ApplicationSettings();
		if (SIPController->LoadApplicationSettings(*ApplicationSettings) == false) {

			EpConfig ep_cfg;
			ep_cfg.logConfig.level = 3;
			ep_cfg.uaConfig.maxCalls = 1;
			ep_cfg.medConfig.sndClockRate = 8000;
			ep_cfg.medConfig.audioFramePtime = PJSUA_DEFAULT_AUDIO_FRAME_PTIME;
			ep_cfg.medConfig.quality = 9;
			ep_cfg.medConfig.sndRecLatency = 100;
			ep_cfg.medConfig.sndPlayLatency = 100;
			ep_cfg.medConfig.ptime = 20;
			ep_cfg.medConfig.ecOptions = 64;

			TransportConfig t_cfg;
			t_cfg.port = 0;

			AccountConfig a_cfg;
			a_cfg.idUri = "sip:3000@10.0.66.160";
			a_cfg.regConfig.timeoutSec = 200;
			a_cfg.regConfig.registrarUri = "sip:10.0.66.160";
			AuthCredInfo cred("digest", "*", "3000", 0, "eyeCallA2014");
			a_cfg.sipConfig.authCreds.push_back(cred);

			/*AccountConfig a_cfg_b;
			a_cfg_b.idUri = "sip:3000@10.0.66.160";
			a_cfg_b.regConfig.timeoutSec = 200;
			a_cfg_b.regConfig.registrarUri = "sip:10.0.66.160";
			AuthCredInfo cred("digest", "*", "3000", 0, "eyeCallA2014");
			a_cfg_b.sipConfig.authCreds.push_back(cred);*/
			ApplicationSettings->_Servo_Calibration_A._Calibration_Left = 0;
			ApplicationSettings->_Servo_Calibration_A._Calibration_Top = 90;
			ApplicationSettings->_Servo_Calibration_A._Calibration_Right = 180;

			ApplicationSettings->_Servo_Calibration_B._Calibration_Left = 0;
			ApplicationSettings->_Servo_Calibration_B._Calibration_Top = 90;
			ApplicationSettings->_Servo_Calibration_B._Calibration_Right = 180;

			ApplicationSettings->_SIP_Config_Endpoint = ep_cfg;
			ApplicationSettings->_SIP_Config_Transport = t_cfg;
			ApplicationSettings->_SIP_Config_Account = a_cfg;

			ApplicationSettings->_Motion_Port_CLK = 18;
			ApplicationSettings->_Motion_Port_DOut = 23;
			ApplicationSettings->_Motion_Port_DIn = 24;
			ApplicationSettings->_Motion_Port_CS = 25;

			ApplicationSettings->_Servo_Port_A = 17;
			ApplicationSettings->_Servo_Port_B = 27;

			ApplicationSettings->_Speak_LogOutput = 0;
			ApplicationSettings->_Speak_Language = 1;
			ApplicationSettings->_Speak_ErrorFilename = "";
			ApplicationSettings->_Speak_LogFilename = "";
			ApplicationSettings->_Speak_ModelDir = "recognition/models";
			ApplicationSettings->_Speak_Dictionary = "dictionary_light.dic";
			ApplicationSettings->_Speak_AudioModel = "voxforge.lm.DMP";
			ApplicationSettings->_Speak_SampelRate = 16000;
			ApplicationSettings->_Speak_BestPath = 1;

			ApplicationSettings->_Motion_Calibration_Min = 0;
			ApplicationSettings->_Motion_Calibration_Max = 0;
			ApplicationSettings->_Motion_Calibration_Threshold = 100;
			ApplicationSettings->_Motion_Calibration_Timeout = 10000;
			ApplicationSettings->_Motion_Calibration_CalibrationOffset = 30;

			SIPController->SaveApplicationSettings(*ApplicationSettings);
		}
		if (ApplicationSettings->Changes == true) {
			SIPController->SaveApplicationSettings(*ApplicationSettings);
		}
	}

	Context->SetApplicationSettings(ApplicationSettings);
}

bool Init::InitIO () {

	if (IOController != 0) {
		MCP3008Function = GPIOMCP3008Function::GPIOMCP3008FunctionWithPort(GPIOMCP3008FunctionPorts(ApplicationSettings->_Motion_Port_CLK, ApplicationSettings->_Motion_Port_DOut, ApplicationSettings->_Motion_Port_DIn, ApplicationSettings->_Motion_Port_CS));
		IOController->SetFunction(MCP3008Function);
#ifndef _WITHOUT_MOTION_DETECTION
		MCP3008Function->StartPolling();
#endif

		MotionDetection = new MotionDetection::EXMotionDetection();
		MotionDetection->SetCalibrationData(ApplicationSettings->_Motion_Calibration_Min, ApplicationSettings->_Motion_Calibration_Max);
		MotionDetection->SetThreshold(ApplicationSettings->_Motion_Calibration_Threshold);
		MotionDetection->SetTimeout(ApplicationSettings->_Motion_Calibration_Timeout);
		MotionDetection->SetCalibrationOffset(ApplicationSettings->_Motion_Calibration_CalibrationOffset);

		ServoFunction = GPIOServoFunction::GPIOServoFunctionWithPort(ApplicationSettings->_Servo_Port_A, ApplicationSettings->_Servo_Port_B);

		IOController->SetFunction(ServoFunction);

		GPIOServoFunctionCalibrationData *Left = new GPIOServoFunctionCalibrationData(GPIOServoFunctionServoLeft);
			Left->SetPosition(GPIOServoFunctionPositionLeft);
			Left->SetValueForPosition(ApplicationSettings->_Servo_Calibration_A._Calibration_Left);
			Left->SetPosition(GPIOServoFunctionPositionTop);
			Left->SetValueForPosition(ApplicationSettings->_Servo_Calibration_A._Calibration_Top);
			Left->SetPosition(GPIOServoFunctionPositionRight);
			Left->SetValueForPosition(ApplicationSettings->_Servo_Calibration_A._Calibration_Right);

		GPIOServoFunctionCalibrationData *Right = new GPIOServoFunctionCalibrationData(GPIOServoFunctionServoRight);
			Right->SetPosition(GPIOServoFunctionPositionLeft);
			Right->SetValueForPosition(ApplicationSettings->_Servo_Calibration_B._Calibration_Left);
			Right->SetPosition(GPIOServoFunctionPositionTop);
			Right->SetValueForPosition(ApplicationSettings->_Servo_Calibration_B._Calibration_Top);
			Right->SetPosition(GPIOServoFunctionPositionRight);
			Right->SetValueForPosition(ApplicationSettings->_Servo_Calibration_B._Calibration_Right);

		ServoFunction->SetCalibration(Left, Right);

		Context->SetIO(MCP3008Function, ServoFunction, MotionDetection);

		return true;
	}

	return false;
}

void Init::CheckStep() {
#ifndef _WITHOUT_MOTION_DETECTION
	if (_MotionCalibrated == MotionIsCalibrated) {
		Context->SetState(new Init2());
	} else if (_MotionCalibrated == MotionIsNotCalibrated) {
		Context->SetState(new Error_MotionNotCalibrated());
	}
#else
	Context->SetState(new Init2());
#endif
}


// -----------------------------------------------------------------------------------------------
// Init2 -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

Init2::Init2() : StateInterface("Init2") {
	// TODO Auto-generated constructor stub
}

Init2::~Init2() {
	// TODO Auto-generated destructor stub
}

void Init2::Initialize() {
	if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(ServoFunction->GetDegree()) != GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios) {
		ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios);
	} else {
		this->GoOn();
	}
}


// Events

void Init2::ReceiveServoValueEvent (const ServoValueEvent *Event) {

	if (GPIO::InternalFunctions::GPIOServoFunctionContainer::Emotion(Event->ServoValueContainer) == GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios) {
		this->GoOn();
	}

	StateInterface::ReceiveServoValueEvent(Event);
}

void Init2::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {
	if (Event->_MotionStatus == MotionDetection::MotionStatusCalibrationModus) {

	} else if (Event->_MotionStatus == MotionDetection::MotionStatusInitialized) {

	}
}


// Internal

void Init2::GoOn() {
#ifndef _WITHOUT_SPEECH_RECOGNITION

	if (SpeechController != 0) {
		Context->SetState(new InitSpeak());
	}
#else
#ifndef _WITHOUT_SIP
		if (SIPController != 0) {
			if (SIPController->InitializeSIP(*ApplicationSettings)) {
				Context->SetState(new InitSIP());
			}
		}
#else
#ifdef _OPEN_CV
		if (ImageRController != 0) {
			ImageRController->SetScaleFactor(1.0);
			ImageRController->SetFramerate(10);
			ImageRController->SetROIAverageSize(20);
			ImageRController->SetSMILEAverageSize(10);
			ImageRController->EnableUsingSmileMiddleSmoothing(false);
			ImageRController->SetRecognizeImagePercentage(30);
			ImageRController->EnableManualSmileDetectionSettings(true);

			// TEST
			ImageRController->SetShowDetectedSmiles(true);
			ImageRController->SetShowDetectedFaces(true);
			ImageRController->SetSmoothFaces(true);

			/*ImageRecognition::DetectionSettings Settings;
			Settings.MinNeighbors = 28;
			Settings.ScaleFactor = 1.1;
			ImageRController->SetManualSmileDetectionSettings(Settings);
			ImageRController->SetInputSize(cv::Size(640,480));*/

			Context->SetState(new InitImageRecognize());
		}
#else
#ifdef DEMO
		Context->SetState(new EXState::Demo());
#else
		if (MotionDetection->Status() == MotionDetection::MotionStatusPeopleInRange) {
			Context->SetState(new EXState::Listening());
		} else if (MotionDetection->Status() == MotionDetection::MotionStatusNoPeopleInRange || MotionDetection->Status() == MotionDetection::MotionStatusGoToNoPeopleInRange) {
			Context->SetState(new EXState::Sleep());
		}
#endif
#endif
#endif
#endif
}


// -----------------------------------------------------------------------------------------------
// InitSpeak -------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

#ifndef _WITHOUT_SPEECH_RECOGNITION
InitSpeak::InitSpeak() : StateInterface("InitSpeak") {

}

void InitSpeak::Initialize() {
	EXSpeech::EXSpeechConfig Config;
		Config.LogOutput		= ApplicationSettings->_Speak_LogOutput;
		Config.ErrorFilename	= ApplicationSettings->_Speak_ErrorFilename;
		Config.LogFilename		= ApplicationSettings->_Speak_LogFilename;
		Config.SampelRate		= ApplicationSettings->_Speak_SampelRate;
		Config.Language			= (EXSpeech::EXSpeechLanguage)ApplicationSettings->_Speak_Language;
		Config.Dictionary		= ApplicationSettings->_Speak_Dictionary;
		Config.AudioModel		= ApplicationSettings->_Speak_AudioModel;
		Config.ModelDir			= ApplicationSettings->_Speak_ModelDir;
		Config.BestPath			= ApplicationSettings->_Speak_BestPath;
	if (SpeechController->InitializeEngine(Config)) {

	}
}


void InitSpeak::ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event) {
	if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeInit) {
		this->GoToInitSIP();
	} else if (Event->Type == EXSpeech::EXSpeechTypes::ThreadWorkingTypeInitFailed) {
#ifdef EX_SOUND_CHECK_OFF
		this->GoToInitSIP();
#else
		Context->SetState(new Exit());
#endif
	}

	StateInterface::ReceiveSpeakEvent(Event);
}

void InitSpeak::GoToInitSIP() {
#ifndef _WITHOUT_SIP
	Context->SetState(new InitSIP());
#endif

}
#endif


// -----------------------------------------------------------------------------------------------
// InitSIP ---------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

#ifndef _WITHOUT_SIP
InitSIP::InitSIP() : StateInterface("InitSIP") {

}

void InitSIP::Initialize() {
	bool Success = false;

	if (SIPController != 0) {
		if ((Success |= SIPController->InitializeSIP(*ApplicationSettings))) {

		}
	}

	if (Success == false) {
		//Context->SetState(new EXState::DetectMotion());
		Context->SetState(new Exit());
	}
}

void InitSIP::ReceiveSIPEvent (const SIP::SIPEvent *Event) {
	if (Event->Status == SIP::SIPStatusStarted) {
		Context->SetState(new EXState::DetectMotion());
	}

	StateInterface::ReceiveSIPEvent(Event);
}
#endif


// -----------------------------------------------------------------------------------------------
// InitImageRecognize ----------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

#ifdef _OPEN_CV
InitImageRecognize::InitImageRecognize("InitImageRecognize") : StateInterface() {
	Device = ImageRecognition::CaptureDeviceNone;
#if __arm__ && _OPEN_CV_RASPI
	Device = ImageRecognition::CaptureDeviceRaspiCam;
#endif
}

void InitImageRecognize::Initialize() {
	if (ImageRController->Initialize(Device)) {

	}
}

void InitImageRecognize::ReceiveImageEvent (const ImageRecognition::ImageEvent *Event) {
	if (Event->Type == ImageRecognition::ThreadWorkingTypeInit) {
		if (MotionDetection->Status() == MotionDetection::MotionStatusPeopleInRange) {
			Context->SetState(new EXState::Listening());
		} else if (MotionDetection->Status() == MotionDetection::MotionStatusNoPeopleInRange || MotionDetection->Status() == MotionDetection::MotionStatusGoToNoPeopleInRange) {
			Context->SetState(new EXState::Sleep());
		}
	} else if (Event->Type == ImageRecognition::ThreadWorkingTypeUnInitialize) {

	} else if (Event->Type == ImageRecognition::ThreadWorkingTypeUnInitialized) {
		if (ImageRController->Initialize(Device)) {

		}
	}
}

void InitImageRecognize::ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event) {

}
#endif

} /* namespace EXState */
