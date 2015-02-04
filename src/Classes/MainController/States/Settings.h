/*
 * 		Settings.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Settings : public StateMachine::StateInterface {
		public:
			Settings(StateMachine::StateInterface *BackState = 0);
			virtual ~Settings();

			virtual void ReceiveKeyEvent (const KeyEvent *Event);

		private:
			StateMachine::StateInterface *BackState;

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* SETTINGS_H_ */
