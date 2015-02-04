/*
 * 		EXServoManuallyDrive.h
 *
 *  	Created on: 05.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXSERVOMANUALDRIVE_H_
#define EXSERVOMANUALDRIVE_H_

#include "EXIKeyParser.h"

#define SERVODRIVE_DEBUG 1

namespace EXServo {

	class EXServoManuallyDrive : public EXInterfaces::EXIKeyParser {
		public:
			EXServoManuallyDrive(GPIO::InternalFunctions::GPIOServoFunction *Servo);
			virtual ~EXServoManuallyDrive();

			void PrintStartCommand ();

			bool Result(std::string Result);
			bool LiveKeys(KEY Key);

		private:

			enum EXServoInternalMode {
				EXServoInternalModeNone = -1,
				EXServoInternalModePosition,
				EXServoInternalModeEmotion,
				EXServoInternalModeDegree
			};

			#define SERVO_MANUALDRIVE_ON		"servo_drive on"
			#define SERVO_MANUALDRIVE_OFF		"servo_drive off"
			#define SERVO_MANUALDRIVE_MODE_POS	"mode position"
			#define SERVO_MANUALDRIVE_MODE_EMO	"mode emotion"
			#define SERVO_MANUALDRIVE_MODE_DEG	"mode degree"
			#define SERVO_MANUALDRIVE_MODE_EXIT	"exit"
			#define SERVO_MANUALDRIVE_LEFT 		"left"
			#define SERVO_MANUALDRIVE_RIGHT		"right"
			#define SERVO_MANUALDRIVE_TOP 		"top"

			#define SERVO_MANUALDRIVE_ANGRY 	"angry"
			#define SERVO_MANUALDRIVE_SAD		"sad"
			#define SERVO_MANUALDRIVE_BLINKL 	"blink left"
			#define SERVO_MANUALDRIVE_BLINKR 	"blink right"
			#define SERVO_MANUALDRIVE_SLEEP 	"sleep"
			#define SERVO_MANUALDRIVE_HELLO 	"hello"
			#define SERVO_MANUALDRIVE_CURIOUS 	"curious"

			EXServoInternalMode					_internal_mode;

	};

} /* namespace EXServo */

#endif /* EXSERVOMANUALDRIVE_H_ */
