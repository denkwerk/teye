/*
 * 		ServoSettings.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "ServoSettings.h"

#include "Settings.h"

namespace EXState {
namespace ApplicationSettings {


// ----------------------------------------------------------------------------------------------------------------------
// Servo
// ----------------------------------------------------------------------------------------------------------------------

Servo::Servo(StateMachine::StateInterface *BackState) : StateInterface("Servo"), BackState(BackState) {
	// TODO Auto-generated constructor stub

}

Servo::~Servo() {
	// TODO Auto-generated destructor stub
}

void Servo::Initialize() {
	PrintText("hello -> servo settings", true);
	PrintText("Command (drive/calibrate/back/<-) : ");
}

// Events

void Servo::ReceiveKeyEvent (const KeyEvent *Event) {

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
			if (Result == "drive") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::ServoDrive(this), false);
			} else if (Result == "calibrate") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::ServoCalibrate());
			} else if (Result == "back") {
				NewLine();
				if (BackState != 0) {
					Context->SetState(BackState);
				} else {
					Context->SetState(new EXState::Settings());
				}
			} else {
				CleanUp(Result);
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}


// ----------------------------------------------------------------------------------------------------------------------
// ServoDrive
// ----------------------------------------------------------------------------------------------------------------------

ServoDrive::ServoDrive(StateMachine::StateInterface *BackState) : StateInterface("ServoDrive"), BackState(BackState), Free(true), Wait(false), EXServoInternalMode(EXServoInternalModeNone) {
	// TODO Auto-generated constructor stub

}

ServoDrive::~ServoDrive() {
	// TODO Auto-generated destructor stub
}

void ServoDrive::Initialize() {
	PrintText("hello -> servo drive", true);
	PrintText("Command (position/emotion/degree/back/<-) : ");
}

void ServoDrive::ReceiveServoValueEvent (const ServoValueEvent *Event) {
	Free = true;
	this->CheckFree();
}

void ServoDrive::ReceiveKeyEvent (const KeyEvent *Event) {

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
			if (EXServoInternalMode == EXServoInternalModeNone) {
				if (Result == "position") {
					NewLine();
					printf("Command (left/top/right/back/<-) : ");
					EXServoInternalMode = EXServoInternalModePosition;
				} else if (Result == "emotion") {
					NewLine();
					printf("Command (angry/sad/blink(_right/_left)/sleep/hello/curious/back/<-) : ");
					EXServoInternalMode = EXServoInternalModeEmotion;
				} else if (Result == "degree") {
					NewLine();
					printf("Command (0-180/back) : ");
					EXServoInternalMode = EXServoInternalModeDegree;
				} else if (Result == "back") {
					NewLine();
					if (BackState != 0) {
						Context->SetState(BackState);
					} else {
						Context->SetState(new EXState::ApplicationSettings::Servo());
					}
				} else {
					CleanUp(Result);
				}
			} else if (EXServoInternalMode == EXServoInternalModePosition) {
				if (Result == "left") {
					CleanUp(Result);
					if (this->CheckFree()) {
						WaitContainer = GPIO::InternalFunctions::GPIOServoFunctionContainer(0, 0, true);
						ServoFunction->SetContainer(WaitContainer);
						Free = false;
					}
				} else if (Result == "top") {
					CleanUp(Result);
					if (this->CheckFree()) {
						WaitContainer = GPIO::InternalFunctions::GPIOServoFunctionContainer(90, 90, true);
						ServoFunction->SetContainer(WaitContainer);
						Free = false;
					}
				} else if (Result == "right") {
					CleanUp(Result);
					if (this->CheckFree()) {
						WaitContainer = GPIO::InternalFunctions::GPIOServoFunctionContainer(180, 180, true);
						ServoFunction->SetContainer(WaitContainer);
						Free = false;
					}
				} else if (Result == "back") {
					NewLine();
					PrintText("Command (position/emotion/degree/back/<-) : ");
					EXServoInternalMode = EXServoInternalModeNone;
				} else {
					CleanUp(Result);
				}
			} else if (EXServoInternalMode == EXServoInternalModeEmotion) {
				if (Result == "angry") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionAngry);
				} else if (Result == "sad") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSad);
				} else if (Result == "blink_right") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight);
				} else if (Result == "blink_left") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkLeft);
				} else if (Result == "sleep") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionSleep);
				} else if (Result == "hello") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionHello);
				} else if (Result == "curious") {
					CleanUp(Result);
					Free = false;
					ServoFunction->SetEmotion(GPIO::InternalFunctions::GPIOServoFunctionEmotionCurios);
				}else if (Result == "back") {
					NewLine();
					PrintText("Command (position/emotion/degree/back/<-) : ");
					EXServoInternalMode = EXServoInternalModeNone;
				} else {
					CleanUp(Result);
				}
			} else if (EXServoInternalMode == EXServoInternalModeDegree) {
				int Number = ToNumber(Result);
				if (Result == "back") {
					NewLine();
					PrintText("Command (position/emotion/degree/back/<-) : ");
					EXServoInternalMode = EXServoInternalModeNone;
				} else if (Number >= 0 && Number <= 180) {
					CleanUp(Result);
					GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = ServoFunction->GetDegree();
					ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(Number, Number, false));
				} else {
					CleanUp(Result);
				}
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}


// Internal

bool ServoDrive::CheckFree() {
	if (Free == true) {
		if (Wait == true) {
			Wait = false;
			CleanUp("!!WAIT!!");
		}
		return true;
	} else {
		if (Wait == false) {
			Wait = true;
			PrintText("!!WAIT!!");
		}
	}
	return false;
}


// ----------------------------------------------------------------------------------------------------------------------
// ServoCalibrate
// ----------------------------------------------------------------------------------------------------------------------

ServoCalibrate::ServoCalibrate(StateMachine::StateInterface *BackState) : StateInterface("ServoCalibrate"), BackState(BackState), EXCalibServo(GPIOServoFunctionServoNone), EXCalibPos(EXCalibPosNone) {
	// TODO Auto-generated constructor stub

}

ServoCalibrate::~ServoCalibrate() {
	// TODO Auto-generated destructor stub
}

void ServoCalibrate::Initialize() {
	PrintText("hello -> servo calibration", true);
	PrintText("-- Select Servo --", true);
	PrintText("Command (left/right/back/<-) : ");
}

void ServoCalibrate::ReceiveServoValueEvent (const ServoValueEvent *Event) {

}

void ServoCalibrate::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;
		KEY Key = Event->Key;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_UARROW || Event->Key == KEY_ACTION_DARROW) {

				Result = "";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (EXCalibServo == GPIOServoFunctionServoNone) {
				if (Result == "left" || Result == "right") {
					NewLine();
					if (Result == "left") {
						EXCalibServo = GPIOServoFunctionServoLeft;
					} else if (Result == "right") {
						EXCalibServo = GPIOServoFunctionServoRight;
					}
					//ServoFunction->SetCalibrationPosition((EXCalibServo == EXCalibServoLeft ? GPIO::InternalFunctions::GPIOServoFunctionServoLeft : GPIO::InternalFunctions::GPIOServoFunctionServoRight), GPIO::InternalFunctions::GPIOServoFunctionPositionLeft);
					PrintText("-- Select Position --", true);
					PrintText("Command (left/top/right/back/<-) : ");
				} else if (Result == "back") {
					NewLine();
					if (BackState != 0) {
						NewLine();
						Context->SetState(BackState);
					} else {
						NewLine();
						Context->SetState(new EXState::ApplicationSettings::Servo());
					}
				} else {
					CleanUp(Result);
				}
			} else {
				if (EXCalibPos == EXCalibPosNone) {
					if (Result == "left") {
						NewLine();
						ServoFunction->SetCalibrationPosition(EXCalibServo, GPIO::InternalFunctions::GPIOServoFunctionPositionLeft);
						PrintText("-- Calibrate Servo --", true);
						PrintText("Command (-180-180/s4ve/back/<-) : ");
						EXCalibPos = EXCalibPosLeft;
					} else if (Result == "top") {
						NewLine();
						ServoFunction->SetCalibrationPosition(EXCalibServo, GPIO::InternalFunctions::GPIOServoFunctionPositionTop);
						PrintText("-- Calibrate Servo --", true);
						PrintText("Command (-180-180/s4ve/back/<-) : ");
						EXCalibPos = EXCalibPosTop;
					} else if (Result == "right") {
						NewLine();
						ServoFunction->SetCalibrationPosition(EXCalibServo, GPIO::InternalFunctions::GPIOServoFunctionPositionRight);
						PrintText("-- Calibrate Servo --", true);
						PrintText("Command (-180-180/s4ve/back/<-) : ");
						EXCalibPos = EXCalibPosRight;
					} else if (Result == "back") {
						CleanUp(Result);
						PrintText("-- Select Servo --", true);
						PrintText("Command (left/right/back/<-) : ");
						EXCalibServo = GPIOServoFunctionServoNone;
						EXCalibPos = EXCalibPosNone;
					} else {
						CleanUp(Result);
					}
				} else {
					int Number = ToNumber(Result);
					if (Number != 0 && Number >= -180 && Number <= 180) {
						CleanUp(Result);
						GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = ServoFunction->GetDegree();
						//int DG = (EXCalibServo == GPIOServoFunctionServoRight ? Degree.Right + Number : -1);
						ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((EXCalibServo == GPIOServoFunctionServoLeft ? Degree.Left + Number : -1), (EXCalibServo == GPIOServoFunctionServoRight ? Degree.Right + Number : -1), false), true);
						//ServoFunction->SaveCalibrationPosition(EXCalibServo, Degree);
					} else if (Key == KEY_ACTION_UARROW) {
						CleanUp(Result);
						GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = ServoFunction->GetDegree();
						ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((EXCalibServo == GPIOServoFunctionServoLeft ? Degree.Left + 1 : -1), (EXCalibServo == GPIOServoFunctionServoRight ? Degree.Right + 1 : -1), false), true);
					} else if (Key == KEY_ACTION_DARROW) {
						CleanUp(Result);
						GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = ServoFunction->GetDegree();
						ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((EXCalibServo == GPIOServoFunctionServoLeft ? Degree.Left - 1 : -1), (EXCalibServo == GPIOServoFunctionServoRight ? Degree.Right - 1 : -1), false));
					} else if (Result == "s4ve") {
						CleanUp(Result);
						GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = ServoFunction->GetDegree();
						ServoFunction->SaveCalibrationPosition(EXCalibServo, (EXCalibServo == GPIOServoFunctionServoLeft ? Degree.Left : Degree.Right));

						GPIOServoFunctionCalibrationData *Calib[2] = {0};
						Calib[GPIOServoFunctionServoRight] = ServoFunction->GetCalibration(GPIOServoFunctionServoRight);
						Calib[GPIOServoFunctionServoLeft] = ServoFunction->GetCalibration(GPIOServoFunctionServoLeft);

						ApplicationSettings->_Servo_Calibration_A._Calibration_Left = Calib[GPIOServoFunctionServoLeft]->GetValueForPosition(GPIOServoFunctionPositionLeft);
						ApplicationSettings->_Servo_Calibration_A._Calibration_Top = Calib[GPIOServoFunctionServoLeft]->GetValueForPosition(GPIOServoFunctionPositionTop);
						ApplicationSettings->_Servo_Calibration_A._Calibration_Right = Calib[GPIOServoFunctionServoLeft]->GetValueForPosition(GPIOServoFunctionPositionRight);

						ApplicationSettings->_Servo_Calibration_B._Calibration_Left = Calib[GPIOServoFunctionServoRight]->GetValueForPosition(GPIOServoFunctionPositionLeft);
						ApplicationSettings->_Servo_Calibration_B._Calibration_Top = Calib[GPIOServoFunctionServoRight]->GetValueForPosition(GPIOServoFunctionPositionTop);
						ApplicationSettings->_Servo_Calibration_B._Calibration_Right = Calib[GPIOServoFunctionServoRight]->GetValueForPosition(GPIOServoFunctionPositionRight);

						if (SIPController->SaveApplicationSettings(*ApplicationSettings)) {
							PrintText(" - Success", true);
						} else {
							PrintText(" - Failed", true);
						}
						PrintText("Command (-180-180/s4ve/back/<-) : ");
					} else if (Result == "back") {
						NewLine();
						EXCalibPos = EXCalibPosNone;
						PrintText("-- Select Position --", true);
						PrintText("Command (left/top/right/back/<-) : ");
					} else {
						CleanUp(Result);
					}
				}
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}

} /* namespace Settings */
} /* namespace EXState */
