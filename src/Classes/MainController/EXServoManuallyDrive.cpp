/*
 * 		EXServoManuallyDrive.cpp
 *
 *  	Created on: 05.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EXServoManuallyDrive.h"

namespace EXServo {

EXServoManuallyDrive::EXServoManuallyDrive(GPIO::InternalFunctions::GPIOServoFunction *Servo) : EXIKeyParser(Servo), _internal_mode(EXServoInternalModeNone) {
	// TODO Auto-generated constructor stub
	SetActive();
}

EXServoManuallyDrive::~EXServoManuallyDrive() {
	// TODO Auto-generated destructor stub
}

void EXServoManuallyDrive::PrintStartCommand () {
	printf("Commands: mode (position/emotion/degree), servo_drive off\n");
}


// Interface Methods

bool EXServoManuallyDrive::Result(std::string Result) {
	if (Inactive()) {
		if (Result == SERVO_MANUALDRIVE_ON) {
			_internal_mode = EXServoInternalModeNone;
			if (SetActive()) {
				WaitForServerResponce(false);
				printf("ServoManuallyDrive: ON\n");
				printf("Commands: mode (position/emotion/degree), servo_drive off\n");
				return true;
			}
		}
	} else if (Active()) {
		if (Result == SERVO_MANUALDRIVE_OFF) {
			if (SetInactive()) {
				printf("ServoManuallyDrive: OFF\n");
				return true;
			}
		} else if (_internal_mode == EXServoInternalModeNone) {
			if (Result == SERVO_MANUALDRIVE_MODE_POS) {
				_internal_mode = EXServoInternalModePosition;
				printf("ServoManuallyDrive: MODE POSITION\n");
				printf("Commands: left, top, right, exit\n");
				return true;
			} else if (Result == SERVO_MANUALDRIVE_MODE_EMO) {
				_internal_mode = EXServoInternalModeEmotion;
				printf("ServoManuallyDrive: MODE EMOTION\n");
				printf("Commands: angry, sad, blink (right/left), sleep, hello, curious exit\n");
				return true;
			} else if (Result == SERVO_MANUALDRIVE_MODE_DEG) {
				_internal_mode = EXServoInternalModeDegree;
				printf("ServoManuallyDrive: MODE DEGREE\n");
				printf("Commands: 0-180, exit\n");
				return true;
			}
		} else if (_internal_mode == EXServoInternalModePosition) {
			if (Result == SERVO_MANUALDRIVE_MODE_EXIT) {
				_internal_mode = EXServoInternalModeNone;
				printf("ServoManuallyDrive: EXIT MODE\n");
				printf("Commands: mode (position/emotion/degree), servo_drive off\n");
				return true;
			} else if (Ready() && Result == SERVO_MANUALDRIVE_LEFT) {
				printf("ServoManuallyDrive: POSITION LEFT\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(0, 0, true)))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION LEFT EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_TOP) {
				printf("ServoManuallyDrive: POSITION TOP\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(90, 90, true)))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION TOP EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_RIGHT) {
				printf("ServoManuallyDrive: POSITION RIGHT\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(180, 180, true)))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION RIGHT EXECUTED\n");
#endif
					return true;
				}
			}
		} else if (_internal_mode == EXServoInternalModeEmotion) {
			if (Result == SERVO_MANUALDRIVE_MODE_EXIT) {
				_internal_mode = EXServoInternalModeNone;
				printf("ServoManuallyDrive: EXIT MODE\n");
				printf("Commands: mode (position/emotion/degree), servo_drive off\n");
				return true;
			} else if (Ready() && Result == SERVO_MANUALDRIVE_ANGRY) {
				printf("ServoManuallyDrive: EMOTION ANGRY\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionAngry))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION ANGRY EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_SAD) {
				printf("ServoManuallyDrive: EMOTION SAD\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION SAD EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_BLINKL) {
				printf("ServoManuallyDrive: EMOTION BLINK LEFT\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION BLINK LEFT EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_BLINKR) {
				printf("ServoManuallyDrive: EMOTION BLINK RIGHT\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION BLINK RIGHT EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_SLEEP) {
				printf("ServoManuallyDrive: EMOTION SLEEP\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSleep))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION SLEEP EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_HELLO) {
				printf("ServoManuallyDrive: EMOTION HELLO\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION HELLO EXECUTED\n");
#endif
					return true;
				}
			} else if (Ready() && Result == SERVO_MANUALDRIVE_CURIOUS) {
				printf("ServoManuallyDrive: EMOTION CURIOUS\n");
				if (WaitForServerResponce(_servo->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios))) {
#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: EMOTION CURIOUS EXECUTED\n");
#endif
					return true;
				}
			}
		} else if (_internal_mode == EXServoInternalModeDegree) {
			if (Result == SERVO_MANUALDRIVE_MODE_EXIT) {
				_internal_mode = EXServoInternalModeNone;
				printf("ServoManuallyDrive: EXIT MODE\n");
				printf("Commands: mode (position/emotion/degree), servo_drive off\n");
				return true;
			} else if (Ready()){
				int Number = ToNumber(Result);
				GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = _servo->GetDegree();
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(Number, Number, false)))) {
#if SERVODRIVE_DEBUG == 1
					std::string numberTest("ServoManuallyDrive: NEW VALUE: ");
					numberTest.append(to_string(Number));
					numberTest.append("\n");
					printf(numberTest.c_str());
#endif
					return true;
				}
			}
		}
	}

	return false;
}

bool EXServoManuallyDrive::LiveKeys(KEY Key) {
	if (Active() && Ready()) {
		if (_internal_mode == EXServoInternalModePosition) {
			if (Key == KEY_ACTION_LARROW) {
				printf("ServoManuallyDrive: POSITION LEFT\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(0, 0, true)))) {
				#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION LEFT EXECUTED\n");
				#endif
					return true;
				}
			} else if (Key == KEY_ACTION_RARROW) {
				printf("ServoManuallyDrive: POSITION RIGHT\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(180, 180, true)))) {
				#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION RIGHT EXECUTED\n");
				#endif
					return true;
				}
			} else if (Key == KEY_ACTION_UARROW) {
				printf("ServoManuallyDrive: POSITION TOP\n");
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(90, 90, true)))) {
				#if SERVODRIVE_DEBUG == 1
					printf("ServoManuallyDrive: POSITION TOP EXECUTED\n");
				#endif
					return true;
				}
			}
		}
	}
	return false;
}

} /* namespace EXServo */
