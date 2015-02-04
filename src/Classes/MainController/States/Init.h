/*
 * 		Init.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef INIT_H_
#define INIT_H_

#include "../../StateMachine/StateContext.h"
#include <stdint.h>

namespace EXState {

	class Init : public StateMachine::StateInterface {
		public:

			Init();
			virtual ~Init();

			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
			virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);

		private:

			enum StepMask {
				Clear					= 0x0,
				MotionEvent				= 0x1,
				InitStep				= 0x2,
				MotionCalibrated		= 0x4,
				MotionNotCalibrated		= 0x8,
				MotionIsCalibrated		= 0x7,
				MotionIsNotCalibrated	= 0xB,
			};

			void LoadSettings();
			bool InitIO();

			void CheckStep();

			uint16_t _MotionCalibrated;

		protected:
			virtual void Initialize();
	};

	class Init2 : public StateMachine::StateInterface {
		public:

			Init2();
			virtual ~Init2();

			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
			virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);

		private:
			void GoOn();

		protected:
			virtual void Initialize();
	};

#ifndef _WITHOUT_SPEECH_RECOGNITION
	class InitSpeak : public StateMachine::StateInterface {
		public:
			InitSpeak();
			virtual ~InitSpeak() {};

			virtual void ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event);

		private:
			void GoToInitSIP();

		protected:
			virtual void Initialize();
	};
#endif

	class InitSIP : public StateMachine::StateInterface {
		public:
			InitSIP();
			virtual ~InitSIP() {};

			virtual void ReceiveSIPEvent (const SIP::SIPEvent *Event);

		private:

		protected:
			virtual void Initialize();
	};

#ifdef _OPEN_CV
	class InitImageRecognize : public StateMachine::StateInterface {
		public:
		InitImageRecognize();
			virtual ~InitImageRecognize() {};

			virtual void ReceiveImageEvent (const ImageRecognition::ImageEvent *Event);
			virtual void ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event);

		private:
			ImageRecognition::CaptureDevice Device;

		protected:
			virtual void Initialize();
	};
#endif

} /* namespace EXState */

#endif /* INIT_H_ */
