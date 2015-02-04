/*
 * 		EXIKeyParser.h
 *
 *  	Created on: 05.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXIKEYPARSER_H_
#define EXIKEYPARSER_H_

#include <string>
#include "../GPIOFunctions/GPIOServoFunction.h"

namespace EXInterfaces {

class EXIKeyParser {
	public:
		EXIKeyParser(GPIO::InternalFunctions::GPIOServoFunction *Servo) : _wait_for_servo_responce(false), _current_mode(EXKeyParserModeStandby), _servo(Servo) {};
		virtual ~EXIKeyParser() {};

		virtual bool Result(std::string Result) = 0;
		virtual bool LiveKeys(KEY Key) = 0;
		bool Active() {return (_current_mode > EXKeyParserModeStandby ? true : false);};
		bool Inactive() {return (_current_mode < EXKeyParserModeActive? true : false);};
		bool ServoResponce() {	if (_wait_for_servo_responce == true) {_wait_for_servo_responce = false;return true;}return false;};
	private:

		enum EXKeyParserMode {
			EXKeyParserModeNone = -1,
			EXKeyParserModeStandby,
			EXKeyParserModeActive
		};

		bool											_wait_for_servo_responce;
		EXKeyParserMode									_current_mode;

	protected:
		GPIO::InternalFunctions::GPIOServoFunction		*_servo;

		bool WaitForServerResponce(bool Active) {_wait_for_servo_responce = Active; return _wait_for_servo_responce;};
		bool Wait() {return _wait_for_servo_responce;};
		bool Ready() {return !_wait_for_servo_responce;};
		bool SetActive() {if (_current_mode == EXKeyParserModeStandby) {_current_mode = EXKeyParserModeActive; return true;}return false;};
		bool SetInactive() {if (_current_mode == EXKeyParserModeActive) {_current_mode = EXKeyParserModeStandby; return true;}return false;};
		int ToNumber(std::string Result) {return atoi(Result.c_str());};
};

} /* namespace EXInterfaces */

#endif /* EXIKEYPARSER_H_ */
