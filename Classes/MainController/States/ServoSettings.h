/*
 * 		ServoSettings.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef SERVOSETTINGS_H_
#define SERVOSETTINGS_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {
	namespace ApplicationSettings {

		class Servo : public StateMachine::StateInterface {
			public:
				Servo(StateMachine::StateInterface *BackState = 0);
				virtual ~Servo();

				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface *BackState;

			protected:
				virtual void Initialize();
		};

		class ServoDrive : public StateMachine::StateInterface {
			public:
				ServoDrive(StateMachine::StateInterface *BackState = 0);
				virtual ~ServoDrive();

				virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface *BackState;
				GPIO::InternalFunctions::GPIOServoFunctionContainer WaitContainer;
				bool Free;
				bool Wait;

				enum EXServoInternalMode {
					EXServoInternalModeNone = -1,
					EXServoInternalModePosition,
					EXServoInternalModeEmotion,
					EXServoInternalModeDegree
				} EXServoInternalMode;

				bool CheckFree();

			protected:
				virtual void Initialize();
		};

		class ServoCalibrate : public StateMachine::StateInterface {
			public:
				ServoCalibrate(StateMachine::StateInterface *BackState = 0);
				virtual ~ServoCalibrate();

				virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface *BackState;

				GPIO::InternalFunctions::GPIOServoFunctionServo EXCalibServo;

				enum EXCalibPos {
					EXCalibPosNone = -1,
					EXCalibPosLeft,
					EXCalibPosTop,
					EXCalibPosRight
				} EXCalibPos;

			protected:
				virtual void Initialize();
		};

	} /* namespace Settings */
} /* namespace EXState */

#endif /* SERVOSETTINGS_H_ */
