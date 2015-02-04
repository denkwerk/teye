/*
 * 		EXMotionDetection.h
 *
 *  	Created on: 08.07.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXMOTIONDETECTION_H_
#define EXMOTIONDETECTION_H_

#include <sys/time.h>

#include "../GPIOFunctions/GPIOMCP3008Function.h"

#include "../Event/EventHandler.h"

using namespace GPIO::InternalFunctions;

namespace MotionDetection {

	enum MotionStatus {
		MotionStatusNone = -1,
		MotionStatusInitialized = 0,
		MotionStatusCalibrationModus,
		MotionStatusPeopleInRange,
		MotionStatusNoPeopleInRange,
		MotionStatusGoToNoPeopleInRange,
	};

	enum CalibrationStep {
		CalibrationStepNone,
		CalibrationStepMin,
		CalibrationStepMax,
		CalibrationStepFinished,
		CalibrationStepDone
	};

	enum CalibrationValue {
		CalibrationValueMax = 0,
		CalibrationValueMin = 1
	};

	class MotionEvent : public Event {
		public:
			MotionEvent (MotionStatus Status) : _MotionStatus(Status) {};
			MotionStatus _MotionStatus;
	};

	class MotionCalibrationEvent : public MotionEvent {
		public:
			MotionCalibrationEvent (CalibrationStep CalibrationStepEntry) : MotionEvent(MotionStatusCalibrationModus), CalibrationStepEntry(CalibrationStepEntry) {};
			CalibrationStep CalibrationStepEntry;
	};

	class EXMotionDetection : public EventHandler {
		public:
			EXMotionDetection();
			virtual ~EXMotionDetection();

			void RawMotionData(int RawMotionValue);

			void StartCalibration();
			void StopCalibration();

			bool InsertNewValue (int Value);
			bool SetCalibrationData (int Min, int Max);
			int GetCalibrationData (CalibrationValue Value);
			CalibrationStep GetCalibrationStatus ();
			void CalibrationDone();
			int GetCalibrationPercent ();
			void SetThreshold (int Threshold);
			void SetTimeout (long long Timeout);
			void SetCalibrationOffset (int Offset);
			bool PeopleInRange ();
			MotionStatus Status();
			bool HasNewStatus ();

		private:

			int				_BufferedValue;
			int				_Calibration[2];
			CalibrationStep	_CalibrationStep;
			int				_CalibrationDeviation;
			int 			_Threshold;
			int 			_Timeout;
			int				_TimeoutRange;
			int				_CalibrationOffset;
			MotionStatus	_CurrentStatus;
			bool			_MotionStatusIsNew;
			long long		_LastTick;
			long long		_CalibrationTimeout;
			bool			_CalibrationRunning;

			void CalibrationValue (int Value);
			bool ComputeMotion (int Value, bool Init = false);
			long long GetMillisecondTime ();
	};

} /* namespace MotionDetection */

#endif /* EXMOTIONDETECTION_H_ */
