/*
 * 		DetectMotion.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef DETECTMOTION_H_
#define DETECTMOTION_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class DetectMotion : public StateMachine::StateInterface {
		public:
			DetectMotion();
			virtual ~DetectMotion();

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* DETECTMOTION_H_ */
