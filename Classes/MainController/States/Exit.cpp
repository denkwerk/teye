/*
 * 		Exit.cpp
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Exit.h"

namespace EXState {

Exit::Exit() : StateInterface("Exit") {
	// TODO Auto-generated constructor stub

}

Exit::~Exit() {
	// TODO Auto-generated destructor stub
}

void Exit::Initialize() {
	SendEvent(new SystemEvent(SystemEventTypeExit));
}

} /* namespace EXState */
