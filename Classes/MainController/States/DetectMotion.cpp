/*
 * 		DetectMotion.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "DetectMotion.h"

#include "Listening.h"
#include "Sleep.h"
#include "Errors.h"

namespace EXState {

DetectMotion::DetectMotion() : StateInterface("DetectMotion") {
	// TODO Auto-generated constructor stub

}

DetectMotion::~DetectMotion() {
	// TODO Auto-generated destructor stub
}

void DetectMotion::Initialize() {
	if (MotionDetection->Status() == MotionDetection::MotionStatusGoToNoPeopleInRange || MotionDetection->Status() == MotionDetection::MotionStatusNoPeopleInRange) {
		Context->SetState(new Sleep());
	} else if (MotionDetection->Status() == MotionDetection::MotionStatusPeopleInRange) {
		Context->SetState(new Listening());
	} else if (MotionDetection->Status() == MotionDetection::MotionStatusNone) {
		Context->SetState(new EXState::Error_NoMotion());
	}
}

} /* namespace EXState */
