/*
 * 		Errors.cpp
 *
 *  	Created on: 10.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Errors.h"

#include "EmotionTimeout.h"
#include "Exit.h"
#include "ServoSettings.h"
#include "Motion.h"
#include "Init.h"

namespace EXState {


// -----------------------------------------------------------------------------------------------
// Error_NoMotion --------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

Error_NoMotion::Error_NoMotion() : StateInterface("Error_NoMotion") {
	// TODO Auto-generated constructor stub

}

Error_NoMotion::~Error_NoMotion() {
	// TODO Auto-generated destructor stub
}

void Error_NoMotion::Initialize() {
	ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(0));
}


// Events

void Error_NoMotion::ReceiveServoValueEvent (const ServoValueEvent *Event) {
	Context->SetState(new ServoTimeout(GPIO::InternalFunctions::GPIOServoFunctionContainer(180), 2000, new Error_NoMotion()));
}


// -----------------------------------------------------------------------------------------------
// Error_MotionNotCalibrated ---------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

Error_MotionNotCalibrated::Error_MotionNotCalibrated() : StateInterface("Error_MotionNotCalibrated") {
	// TODO Auto-generated constructor stub

}

Error_MotionNotCalibrated::~Error_MotionNotCalibrated() {
	// TODO Auto-generated destructor stub
}

void Error_MotionNotCalibrated::Initialize() {
	if (MotionDetection->Status() == MotionDetection::MotionStatusCalibrationModus) {
		printf("Calibrate the Motion? (yes/no) : ");
		ServoFunction->SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer(45));
	} else {
		Context->SetState(new Init2());
	}
}


// Events

void Error_MotionNotCalibrated::ReceiveServoValueEvent (const ServoValueEvent *Event) {
	//Context->SetState(new ServoTimeout(GPIO::InternalFunctions::GPIOServoFunctionContainer(90), 2000, this, false), false);
}

void Error_MotionNotCalibrated::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {

}

void Error_MotionNotCalibrated::ReceiveMotionCalibrationEvent (const MotionDetection::MotionCalibrationEvent *Event) {

}

void Error_MotionNotCalibrated::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (Result == "yes") {
				NewLine();
				Context->SetState(new ApplicationSettings::MotionCalibration(new Init2(), new Error_MotionNotCalibrated()));
			} else if (Result == "no") {
				NewLine();
				Context->SetState(new Exit());
			} else {
				CleanUp(Result);
			}
		}
	}
}

} /* namespace EXState */
