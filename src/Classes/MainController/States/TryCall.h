/*
 * 		TryCall.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef TRYCALL_H_
#define TRYCALL_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class TryCall : public StateMachine::StateInterface {
		public:
			TryCall(GPIO::InternalFunctions::GPIOServoFunctionEmotion Emotion = GPIO::InternalFunctions::GPIOServoFunctionEmotionEyeBlinkRight);
			virtual ~TryCall();

			virtual void ReceiveSIPEvent (const SIP::SIPEvent *Event);
			virtual void ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted);
			virtual void ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event);
			virtual void ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event);

			virtual void ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event);

#ifndef _WITHOUT_SPEECH_RECOGNITION
			virtual void ReceiveSpeakEvent (const EXSpeech::SpeechEvent *Event);
			virtual void ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event);
#endif
			virtual void ReceiveKeyEvent (const KeyEvent *Event);

		private:
			GPIO::InternalFunctions::GPIOServoFunctionEmotion StandardEmotion;
			int RingCount;

		protected:
			virtual void Initialize();

			std::string			_DetectedString;
	};

} /* namespace EXState */

#endif /* TRYCALL_H_ */
