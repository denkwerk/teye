/*
 * 		Listening.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef LISTENING_H_
#define LISTENING_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Listening : public StateMachine::StateInterface {
		public:
			Listening();
			virtual ~Listening();

#ifndef _WITHOUT_SPEECH_RECOGNITION
			virtual void ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event);
			virtual void ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event);
#endif
			virtual void ReceiveServoValueEvent (const ServoValueEvent *Event);
			virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);
			virtual void ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted);
			virtual void ReceiveKeyEvent (const KeyEvent *Event);

#ifdef _OPEN_CV
			virtual void ReceiveImageEvent (const ImageRecognition::ImageEvent *Event);
			virtual void ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event);
#endif

		private:

		protected:
			virtual void Initialize();
			virtual void GoAway();

			std::string			_DetectedString;
	};

} /* namespace EXState */

#endif /* LISTENING_H_ */
