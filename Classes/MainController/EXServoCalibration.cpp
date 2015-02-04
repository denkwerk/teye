/*
 * 		EXServoCalibration.cpp
 *
 *  	Created on: 04.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EXServoCalibration.h"

namespace EXCalibration {

EXServoCalibration::EXServoCalibration(GPIO::InternalFunctions::GPIOServoFunction *Servo) : EXIKeyParser(Servo), _current_servo(EXCalibServoNone), _current_pos(EXCalibPosNone) {
	// TODO Auto-generated constructor stub
}

EXServoCalibration::~EXServoCalibration() {
	// TODO Auto-generated destructor stub
}

bool EXServoCalibration::Result(std::string Result) {

	if (Inactive()) {
		if (Result == CALIB_ON) {
			SetActive();
			_current_servo = EXCalibServoNone;
			_current_pos = EXCalibPosNone;
			WaitForServerResponce(false);
			printf("ServoCalib: ON\n");
			printf("Commands: servo (left/right), servo_calib off\n");
			return true;
		}
	} else if (Active()) {
		if (Result == CALIB_OFF) {
			SetInactive();
			printf("ServoCalib: OFF\n");
			return true;
		} else if (Result == CALIB_SERVO_LEFT) {
			_current_servo = EXCalibServoLeft;
			_current_pos = EXCalibPosLeft;
			if (_servo != 0) {
				if (WaitForServerResponce(_servo->SetCalibrationPosition(GPIO::InternalFunctions::GPIOServoFunctionServoLeft, GPIO::InternalFunctions::GPIOServoFunctionPositionLeft))) {
					printf("ServoCalib: LEFT SERVO activated\n");
					printf("Commands: servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: LEFT SERVO FAILED\n");
				}
			}
		} else if (Result == CALIB_SERVO_RIGHT) {
			_current_servo = EXCalibServoRight;
			_current_pos = EXCalibPosLeft;
			if (_servo != 0) {
				if (WaitForServerResponce(_servo->SetCalibrationPosition(GPIO::InternalFunctions::GPIOServoFunctionServoRight, GPIO::InternalFunctions::GPIOServoFunctionPositionLeft))) {
					printf("ServoCalib: RIGHT SERVO activated\n");
					printf("Commands: servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: RIGHT SERVO FAILED\n");
				}
			}
		} else if (Result == CALIB_SERVO_POS_LEFT && _current_servo != EXCalibServoNone) {
			_current_pos = EXCalibPosLeft;
			if (_servo != 0) {
				if (WaitForServerResponce(_servo->SetCalibrationPosition((_current_servo == EXCalibServoLeft ? GPIO::InternalFunctions::GPIOServoFunctionServoLeft : GPIO::InternalFunctions::GPIOServoFunctionServoRight), GPIO::InternalFunctions::GPIOServoFunctionPositionLeft))) {
					printf("ServoCalib: LEFT POSITION\n");
					printf("Commands: -180 - 180, s4ve, servo (left/right), servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: LEFT POSITION FAILED\n");
				}
			}
		} else if (Result == CALIB_SERVO_POS_TOP && _current_servo != EXCalibServoNone) {
			_current_pos = EXCalibPosTop;
			if (_servo != 0) {
				if (WaitForServerResponce(_servo->SetCalibrationPosition((_current_servo == EXCalibServoLeft ? GPIO::InternalFunctions::GPIOServoFunctionServoLeft : GPIO::InternalFunctions::GPIOServoFunctionServoRight), GPIO::InternalFunctions::GPIOServoFunctionPositionTop))) {
					printf("ServoCalib: TOP POSITION\n");
					printf("Commands: -180 - 180, s4ve, servo (left/right), servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: TOP POSITION FAILED\n");
				}
			}
		} else if (Result == CALIB_SERVO_POS_RIGHT && _current_servo != EXCalibServoNone) {
			_current_pos = EXCalibPosRight;
			if (_servo != 0) {
				if (WaitForServerResponce(_servo->SetCalibrationPosition((_current_servo == EXCalibServoLeft ? GPIO::InternalFunctions::GPIOServoFunctionServoLeft : GPIO::InternalFunctions::GPIOServoFunctionServoRight), GPIO::InternalFunctions::GPIOServoFunctionPositionRight))) {
					printf("ServoCalib: RIGHT POSITION\n");
					printf("Commands: -180 - 180, s4ve, servo (left/right), servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: RIGHT POSITION FAILED\n");
				}
			}
		} else if (_current_servo != EXCalibServoNone && _current_pos != EXCalibPosNone && Result == CALIB_SAVE) {
			if (_servo != 0) {
				GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = _servo->GetDegree();
				if (_servo->SaveCalibrationPosition((_current_servo == EXCalibServoLeft ? GPIO::InternalFunctions::GPIOServoFunctionServoLeft : GPIO::InternalFunctions::GPIOServoFunctionServoRight), (_current_servo == EXCalibServoLeft ? Degree.Left : Degree.Right))) {
					printf("ServoCalib: SAVE SUCCESS\n");
					printf("Commands: -180 - 180, s4ve, servo (left/right), servo_pos (left/top/right), servo_calib off\n");
					return true;
				} else {
					printf("ServoCalib: SAVE SUCCESS FAILED\n");
				}
			}
		} else {
			if (_current_servo != EXCalibServoNone && _current_pos != EXCalibPosNone) {
				int Number = atoi(Result.c_str());
				GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = _servo->GetDegree();
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((_current_servo == EXCalibServoLeft ? Degree.Left + Number : -1), (_current_servo == EXCalibServoRight ? Degree.Right + Number : -1), false), true))) {
#if CALIB_DEBUG == 1
					std::string numberTest("ServoCalib: NEW VALUE: ");
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

bool EXServoCalibration::LiveKeys(KEY Key) {

	if (Active() && Ready()) {

		if (_current_servo != EXCalibServoNone && _current_pos != EXCalibPosNone) {
			if (Key == KEY_ACTION_LARROW) {
				GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = _servo->GetDegree();
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((_current_servo == EXCalibServoLeft ? Degree.Left + 1 : -1), (_current_servo == EXCalibServoRight ? Degree.Right + 1 : -1), false), true))) {
#if CALIB_DEBUG == 1
					std::string numberTest("ServoCalib: NEW VALUE: ");
					numberTest.append(to_string(Degree.Left + 1));
					numberTest.append("\n");
					printf(numberTest.c_str());
#endif
					return true;
				}
			} else if (Key == KEY_ACTION_RARROW) {
				GPIO::InternalFunctions::GPIOServoFunctionContainer Degree = _servo->GetDegree();
				if (WaitForServerResponce(_servo->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((_current_servo == EXCalibServoLeft ? Degree.Left - 1 : -1), (_current_servo == EXCalibServoRight ? Degree.Right - 1 : -1), false)))) {
#if CALIB_DEBUG == 1
					std::string numberTest("ServoCalib: NEW VALUE: ");
					numberTest.append(to_string(Degree.Left - 1));
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

} /* namespace EXCalibration */

