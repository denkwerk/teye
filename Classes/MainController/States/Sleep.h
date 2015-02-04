/*
 * 		Sleep.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef SLEEP_H_
#define SLEEP_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Sleep : public StateMachine::StateInterface {
		public:
			Sleep();
			virtual ~Sleep();

			virtual void ReceiveMotionEvent (const MotionDetection::MotionEvent *Event);
			virtual void ReceiveSIPCallEvent (const SIP::SIPCallEvent *Event, bool Accepted);
			virtual void ReceiveKeyEvent (const KeyEvent *Event);

#ifdef _OPEN_CV
			virtual void ReceiveImageEvent (const ImageRecognition::ImageEvent *Event);
			virtual void ReceiveImageRecognizeEvent (const ImageRecognition::ImageRecognizeEvent *Event);
#endif

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* SLEEP_H_ */
