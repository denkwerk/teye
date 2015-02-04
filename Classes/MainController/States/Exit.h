/*
 * 		Exit.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXIT_H_
#define EXIT_H_

#include "../../StateMachine/StateContext.h"

namespace EXState {

	class Exit : public StateMachine::StateInterface {
		public:
			Exit();
			virtual ~Exit();

		protected:
			virtual void Initialize();
	};

} /* namespace EXState */

#endif /* EXIT_H_ */
