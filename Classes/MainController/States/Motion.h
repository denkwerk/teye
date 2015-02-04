/*
 * 		Motion.h
 *
 *  	Created on: 11.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef MOTION_H_
#define MOTION_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {
	namespace ApplicationSettings {

		class Motion : public StateMachine::StateInterface {
			public:
				Motion(StateMachine::StateInterface *BackState = 0);
				virtual ~Motion();

				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface *BackState;

			protected:
				virtual void Initialize();
		};

		class MotionLive : public StateMachine::StateInterface {
			public:
				MotionLive(StateMachine::StateInterface *BackState = 0);
				virtual ~MotionLive();

				virtual void ReceiveRawMotionData(const MotionRawEvent *Event);
				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface	*BackState;

				int								_CurrentPercent;
				float							_OldValue;

			protected:
				virtual void Initialize();
		};

		class MotionCalibration : public StateMachine::StateInterface {
			public:
				MotionCalibration(StateMachine::StateInterface *SuccessBackState = 0, StateMachine::StateInterface *FailedBackState = 0);
				virtual ~MotionCalibration();

				virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);
				virtual void ReceiveMotionCalibrationEvent (const MotionDetection::MotionCalibrationEvent *Event);
				virtual void ReceiveKeyEvent (const KeyEvent *Event);

				static void Repaint (int Percent, int &Current, int MaxDistance = 10, std::string OptionalValue = std::string(""), std::string OldValue = std::string(""));

			private:
				StateMachine::StateInterface *SuccessBackState;
				StateMachine::StateInterface *FailedBackState;
				bool Started;
				int _CurrentPercent;
				MotionDetection::CalibrationStep Step;

			protected:
				virtual void Initialize();
		};

		class MotionSettings : public StateMachine::StateInterface {
			public:
				MotionSettings(StateMachine::StateInterface *BackState = 0);
				virtual ~MotionSettings();

				virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);
				virtual void ReceiveKeyEvent (const KeyEvent *Event);

			private:
				StateMachine::StateInterface *BackState;

				bool ParseSetting (std::string Result);
				std::string SettingWorker (std::string Setting, int Value, bool Get = false);

				protected:
					virtual void Initialize();
		};

	} /* namespace ApplicationSettings */
} /* namespace EXState */

#endif /* MOTION_H_ */
