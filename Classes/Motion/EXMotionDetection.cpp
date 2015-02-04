/*
 * 		EXMotionDetection.cpp
 *
 * 		Created on: 08.07.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EXMotionDetection.h"

namespace MotionDetection {

EXMotionDetection::EXMotionDetection() : _BufferedValue(-1), _Calibration{0}, _CalibrationStep(CalibrationStepNone), _CalibrationDeviation(100), _Threshold(0), _Timeout(0), _TimeoutRange(1000), _CalibrationOffset(30), _CurrentStatus(MotionStatusNone), _MotionStatusIsNew(false), _LastTick(0), _CalibrationTimeout(10000), _CalibrationRunning(false) {
	// TODO Auto-generated constructor stub

	//_Calibration[CalibrationValueMax] = 0;
	//_Calibration[CalibrationValueMin] = 0;
}

EXMotionDetection::~EXMotionDetection() {
	// TODO Auto-generated destructor stub
}

void EXMotionDetection::RawMotionData(int RawMotionValue) {
	InsertNewValue(RawMotionValue);
}

void EXMotionDetection::StartCalibration() {
	_CalibrationRunning = true;
}

void EXMotionDetection::StopCalibration() {
	_CalibrationRunning = false;
}

bool EXMotionDetection::InsertNewValue (int Value) {

	if (_CalibrationStep == CalibrationStepDone) {
		bool _Init = (_BufferedValue == -1 ? true : false);

		return ComputeMotion(Value, _Init);
	} else {
		if (_CalibrationRunning == true) {
			CalibrationValue(Value);
		}
	}

	return false;
}

bool EXMotionDetection::SetCalibrationData (int Min, int Max) {
	if (Min == 0 || Max == 0) {
		_CalibrationStep = CalibrationStepMin;
		_CurrentStatus = MotionStatusCalibrationModus;
		SendEvent(new MotionEvent(MotionStatusCalibrationModus));
		SendEvent(new MotionCalibrationEvent(_CalibrationStep));
		return false;
	}
	_Calibration[CalibrationValueMin] = Min;
	_Calibration[CalibrationValueMax] = Max;
	_CalibrationStep = CalibrationStepDone;
	SendEvent(new MotionEvent(MotionStatusInitialized));
	return true;
}

int EXMotionDetection::GetCalibrationData (MotionDetection::CalibrationValue Value) {
	return _Calibration[Value];
}

CalibrationStep EXMotionDetection::GetCalibrationStatus () {
	return _CalibrationStep;
}

void EXMotionDetection::CalibrationDone() {
	if (_CalibrationStep == CalibrationStepFinished) {
		_CalibrationStep = CalibrationStepDone;
		SendEvent(new MotionCalibrationEvent(_CalibrationStep));
	}
}

int EXMotionDetection::GetCalibrationPercent () {
	if (_LastTick == 0) {
		return 0;
	}
	return (GetMillisecondTime() - _LastTick) / (_CalibrationTimeout / 100.0);
}

void EXMotionDetection::SetThreshold (int Threshold) {
	_Threshold = Threshold;
}

void EXMotionDetection::SetTimeout (long long Timeout) {
	_Timeout = Timeout;
}

void EXMotionDetection::SetCalibrationOffset (int Offset) {
	_CalibrationOffset = Offset;
}

bool EXMotionDetection::PeopleInRange () {
	return false;
}

MotionStatus EXMotionDetection::Status() {
	return _CurrentStatus;
}

bool EXMotionDetection::HasNewStatus () {
	return _MotionStatusIsNew;
}

void EXMotionDetection::CalibrationValue (int Value) {
	if (_CalibrationStep == CalibrationStepMin) {
		if (_Calibration[CalibrationValueMin] == 0 || ((_Calibration[CalibrationValueMin] + _CalibrationDeviation) > Value && (_Calibration[CalibrationValueMin] - _CalibrationDeviation) < Value)) {
			if (_Calibration[CalibrationValueMin] < Value) {
				_Calibration[CalibrationValueMin] = Value + _CalibrationOffset;
			}

			if (_LastTick == 0) {
				_LastTick = GetMillisecondTime();
			}

			if (_LastTick + _CalibrationTimeout <= GetMillisecondTime()) {
				_CalibrationStep = CalibrationStepMax;
				_LastTick = 0;
			}
		} else {
			_LastTick = 0;
			_Calibration[CalibrationValueMin] = 0;
		}
	} else if (_CalibrationStep == CalibrationStepMax) {
		if (((_Calibration[CalibrationValueMin] + 250) < Value) &&  (_Calibration[CalibrationValueMax] - _CalibrationDeviation) < Value) {
			if (_Calibration[CalibrationValueMax] < Value) {
				_Calibration[CalibrationValueMax] = Value - _CalibrationOffset;
			}

			if (_LastTick == 0) {
				_LastTick = GetMillisecondTime();
			}

			if (_LastTick + _CalibrationTimeout <= GetMillisecondTime()) {
				_CalibrationStep = CalibrationStepFinished;
				_CurrentStatus = MotionStatusInitialized;
				_LastTick = 0;
			}
		} else {
			_LastTick = 0;
		}
	}

	SendEvent(new MotionCalibrationEvent(_CalibrationStep));
}

bool EXMotionDetection::ComputeMotion (int Value, bool Init) {
	_MotionStatusIsNew = false;
	if (Init == false) {
		if (_BufferedValue > (Value + _Threshold) || _BufferedValue < (Value - _Threshold)) {
			if (_CurrentStatus != MotionStatusPeopleInRange) {
				_CurrentStatus = MotionStatusPeopleInRange;
				_MotionStatusIsNew = true;
			}
			_BufferedValue = Value;
			_LastTick = GetMillisecondTime();
		} else if (_Calibration[CalibrationValueMin] < Value && _Calibration[CalibrationValueMax] > Value) {
			if (_CurrentStatus != MotionStatusPeopleInRange) {
				_CurrentStatus = MotionStatusPeopleInRange;
				_MotionStatusIsNew = true;
			}
			_BufferedValue = Value;
			_LastTick = GetMillisecondTime();
		} else {
			long long CurrentStamp = GetMillisecondTime();
			if ((_LastTick + _Timeout) < CurrentStamp) {
				if (_CurrentStatus != MotionStatusNoPeopleInRange) {
					_CurrentStatus = MotionStatusNoPeopleInRange;
					_MotionStatusIsNew = true;
				}
			} else  if ((_LastTick + (_Timeout - _TimeoutRange)) < CurrentStamp) {
				if (_CurrentStatus != MotionStatusGoToNoPeopleInRange) {
					_CurrentStatus = MotionStatusGoToNoPeopleInRange;
					_MotionStatusIsNew = true;
				}
			}
		}
	} else {
		_BufferedValue = Value;
		_CurrentStatus = MotionStatusNoPeopleInRange;
		_LastTick = GetMillisecondTime();
	}

	if (_MotionStatusIsNew == true) {
		MotionEvent *Event = new MotionEvent(_CurrentStatus);
		SendEvent(Event);
		return true;
	}

	return false;
}

long long EXMotionDetection::GetMillisecondTime () {
	struct timeval  te;
	gettimeofday(&te, 0);

	return te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
}

} /* namespace MotionDetection */
