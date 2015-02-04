/*
 * 		Errors.h
 *
 *  	Created on: 10.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef ERRORS_H_
#define ERRORS_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Error_NoMotion : public StateMachine::StateInterface {

		public:
			Error_NoMotion();
			virtual ~Error_NoMotion();

			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);


		private:


		protected:
			virtual void Initialize();

	};

	class Error_MotionNotCalibrated : public StateMachine::StateInterface {

		public:
			Error_MotionNotCalibrated();
			virtual ~Error_MotionNotCalibrated();

			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
			virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);
			virtual void ReceiveMotionCalibrationEvent (const MotionDetection::MotionCalibrationEvent *Event);
			virtual void ReceiveKeyEvent (const KeyEvent *Event);


		private:


		protected:
			virtual void Initialize();

	};

} /* namespace EXState */

#endif /* ERRORS_H_ */
