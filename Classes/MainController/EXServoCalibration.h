/*
 * 		EXServoCalibration.h
 *
 *  	Created on: 04.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXSERVOCALIBRATION_H_
#define EXSERVOCALIBRATION_H_

#include "EXIKeyParser.h"

#define CALIB_DEBUG 1

namespace EXCalibration {

	class EXServoCalibration : public EXInterfaces::EXIKeyParser {

		public:
			EXServoCalibration(GPIO::InternalFunctions::GPIOServoFunction *Servo);
			virtual ~EXServoCalibration();

			bool Result(std::string Result);
			bool LiveKeys(KEY Key);

		private:

			#define CALIB_ON					"servo_calib on"
			#define CALIB_OFF					"servo_calib off"
			#define CALIB_SAVE					"s4ve"
			#define CALIB_SERVO_LEFT			"servo left"
			#define CALIB_SERVO_RIGHT			"servo right"
			#define CALIB_SERVO_POS_LEFT		"servo_pos left"
			#define CALIB_SERVO_POS_TOP			"servo_pos top"
			#define CALIB_SERVO_POS_RIGHT		"servo_pos right"

			enum EXCalibServo {
				EXCalibServoNone = -1,
				EXCalibServoLeft,
				EXCalibServoRight
			};
			enum EXCalibPos {
				EXCalibPosNone = -1,
				EXCalibPosLeft,
				EXCalibPosTop,
				EXCalibPosRight
			};

			EXCalibServo									_current_servo;
			EXCalibPos										_current_pos;
	};

} /* namespace EXCalibration */

#endif /* EXSERVOCALIBRATION_H_ */
