/*
 * 		RingRing.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef RINGRING_H_
#define RINGRING_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class RingRing : public StateMachine::StateInterface {
		public:
			RingRing();
			virtual ~RingRing();

			virtual void ReceiveSIPEvent (const SIP::SIPEvent *Event);
			virtual void ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted);
			virtual void ReceiveSIPCallStateEvent (const SIP::SIPCallStateEvent *Event);
			virtual void ReceiveSIPStateEvent (const SIP::SIPStateEvent *Event);
#ifndef _WITHOUT_SPEECH_RECOGNITION
			virtual void ReceiveSpeakRecognizeEvent (const EXSpeech::SpeechRecognizeEvent *Event);
#endif
			virtual void ReceiveKeyEvent (const KeyEvent *Event);
			virtual void ReceiveServoValueEvent (const GPIO::InternalFunctions::ServoValueEvent *Event);

		private:
			int RingCount;

		protected:
			virtual void Initialize();
			virtual void GoAway ();
	};

} /* namespace EXState */

#endif /* RINGRING_H_ */
