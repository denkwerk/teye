/*
 * 		GPIOServoFunction.cpp
 *
 *  	Created on: 16.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "GPIOServoFunction.h"

#include <unistd.h>

#include  <math.h>

namespace GPIO {
namespace InternalFunctions {


// Helper Static Constructor

GPIOServoFunction* GPIOServoFunction::GPIOServoFunctionWithPort (int PortA, int PortB){
	GPIOServoFunction *Function = new GPIOServoFunction();

	Function->SetPorts(PortA, PortB);
	return Function;
}


// Private De -and Constructors

GPIOServoFunction::GPIOServoFunction() : GPIOFunction(), _portA(0), _portB(0), _CurrentEmotion(GPIOServoFunctionEmotionNone), _CurrentDegree(0), _FutureDegree(0), _CurrentCalibrationSets{0}, VoltageSpeed(0) {
	// TODO Auto-generated constructor stub
	_CurrentCalibrationSets[GPIOServoFunctionServoLeft] = new GPIOServoFunctionCalibrationData(GPIOServoFunctionServoLeft);
	_CurrentCalibrationSets[GPIOServoFunctionServoRight] = new GPIOServoFunctionCalibrationData(GPIOServoFunctionServoRight);

	_FutureDegree = new GPIOServoFunctionContainer();
	_CurrentDegree = new GPIOServoFunctionContainer();

	RegisterEventFunc(this, &GPIOServoFunction::ReceiveServoEvent);
	RegisterEventFunc(this, &GPIOServoFunction::ReceiveServoEmotionEvent);

	// Servo Speed Function
	float VoltageSpeedReferences[2][2] = {{6.0, 0.08}, {4.8, 0.1}};

	/* formula
	 * 		y2 -y1
	 * 	m =	------- * (x - x1) + y1
	 * 		x2 - x1
	*/

	float m = (VoltageSpeedReferences[1][1] - VoltageSpeedReferences[0][1]) / (VoltageSpeedReferences[0][0] - VoltageSpeedReferences[1][0]);
	VoltageSpeed = (fabs(SERVO_VOLTAGE - VoltageSpeedReferences[0][0]) * m) + VoltageSpeedReferences[0][1];

	// ---------------
}

GPIOServoFunction::~GPIOServoFunction() {
	// TODO Auto-generated destructor stub
	_portA = -1;
	_portB = -1;
	_CurrentEmotion = GPIOServoFunctionEmotionNone;
}


// Event

void GPIOServoFunction::ReceiveServoEvent (const ServoControlEvent *Event) {
	this->SetContainer(Event->ServoValueContainer);
}

void GPIOServoFunction::ReceiveServoEmotionEvent (const ServoEmotionControlEvent *Event) {
	this->SetEmotion(Event->Emotion);
}


// Servo Methods

bool GPIOServoFunction::ClearAllJobs () {
	if (this->IsInitialized()) {
		return this->ClearMeFromQueue();
	}

	return false;
}

bool GPIOServoFunction::SetContainer (GPIOServoFunctionContainer Degree, bool Clean, bool Force) {
	if (this->IsInitialized()) {

#if SERVO_DEBUG == 1
		std::string in("SERVO IN LEFT: ");
		in.append(to_string(Degree.Left));
		in.append(" RIGHT: ");
		in.append(to_string(Degree.Right));
		in.append(" CURRENT LEFT: ");
		in.append(to_string(_CurrentDegree->Left));
		in.append(" CURRENT RIGHT: ");
		in.append(to_string(_CurrentDegree->Right));
		in.append(" FUTURE LEFT: ");
		in.append(to_string(_FutureDegree->Left));
		in.append(" FUTURE RIGHT: ");
		in.append(to_string(_FutureDegree->Right));
		in.append("\n");
		printf(in.c_str());
#endif

		if (_CurrentDegree->Left != Degree.Left || _CurrentDegree->Right != Degree.Right) {
			GPIOServoFunctionContainer *DegreeContainer = new GPIOServoFunctionContainer();

			if (Degree.Synch == true/* && Degree.Right == -1*/) {
				Degree.Right = Degree.Left;
			}

			if (Degree.Right != -1 && _CurrentDegree->Right != Degree.Right && _FutureDegree->Right != Degree.Right) {
				DegreeContainer->Right = (Degree.Synch == true ? 180 - Degree.Right : Degree.Right);
				DegreeContainer->VRight = GetCleanValue(DegreeContainer->Right, _FutureDegree->VRight,  GPIOServoFunctionServoRight, Clean);
				_FutureDegree->Right = DegreeContainer->Right;
				_FutureDegree->VRight = (DegreeContainer->VRight == -1 ? _FutureDegree->VRight : DegreeContainer->VRight);
			}

			if (Degree.Left != -1 &&  _CurrentDegree->Left != Degree.Left && _FutureDegree->Left != Degree.Left) {
				DegreeContainer->Left = Degree.Left;
				DegreeContainer->VLeft = GetCleanValue(Degree.Left, _FutureDegree->VLeft, GPIOServoFunctionServoLeft, Clean);
				_FutureDegree->Left = DegreeContainer->Left;
				_FutureDegree->VLeft = (DegreeContainer->VLeft == -1 ? _FutureDegree->Left : DegreeContainer->VLeft);
			}

			if (Degree.Synch == true) {
				DegreeContainer->VRight = (DegreeContainer->VRight == -1 ? DegreeContainer->VLeft : DegreeContainer->VRight);
				DegreeContainer->Right = (DegreeContainer->VRight == -1 ? DegreeContainer->Left : DegreeContainer->Right);
				DegreeContainer->VLeft = (DegreeContainer->VLeft == -1 ? DegreeContainer->VRight : DegreeContainer->VLeft);
				DegreeContainer->Left = (DegreeContainer->VLeft == -1 ? DegreeContainer->Right : DegreeContainer->Left);
			}

#if SERVO_DEBUG == 1
			std::string out("SERVO OUT LEFT: ");
			out.append(to_string(DegreeContainer->Left));
			out.append(" RIGHT: ");
			out.append(to_string(DegreeContainer->Right));
			out.append("\n");
			printf(out.c_str());
#endif

			if (DegreeContainer->VLeft != -1 || DegreeContainer->VRight != -1) {
				DegreeContainer->Synch = Degree.Synch;
				this->NeedsUpdate((void*)DegreeContainer);
				return true;
			} else if (Force == true) {
				return true;
			}
		}
	}
	return false;
}

bool GPIOServoFunction::SetDegree (int Degree) {
	return this->SetContainer(GPIOServoFunctionContainer(Degree, Degree, true));
}

bool GPIOServoFunction::SetSynchronDegree (int Degree) {
	return this->SetContainer(GPIOServoFunctionContainer(Degree, Degree, false));
}

bool GPIOServoFunction::SetEmotion (GPIOServoFunctionEmotion Emotion) {
	if (_CurrentEmotion != Emotion) {
		GPIOServoFunctionContainer::EmotionMap::const_iterator it = GPIOEmotions.find(Emotion);
		if (it != GPIOEmotions.end()) {
			const GPIOServoFunctionContainer &Degree = it->second;

			if (Degree.Right > -1 || Degree.Left > -1) {
				return SetContainer(Degree);
			}
		}
	}

	return false;
}

bool GPIOServoFunction::SetCalibrationPosition(GPIOServoFunctionServo Servo, GPIOServoFunctionPosition Position) {
	if (Servo != GPIOServoFunctionServoNone && Position != GPIOServoFunctionPositionNone) {
		_CurrentCalibrationSets[Servo]->SetPosition(Position);
		int Pos = _CurrentCalibrationSets[Servo]->GetValueForPosition(Position);
		return SetContainer(GPIO::InternalFunctions::GPIOServoFunctionContainer((Servo == GPIOServoFunctionServoLeft ? Pos : -1), (Servo == GPIOServoFunctionServoRight ? Pos : -1), false), true, true);
	}
	return false;
}

bool GPIOServoFunction::SaveCalibrationPosition(GPIOServoFunctionServo Servo, int Value) {
	if (Servo != GPIOServoFunctionServoNone) {
		_CurrentCalibrationSets[Servo]->SetValueForPosition(Value);
		SendEvent(new ServoEvent(GPIOServoFunctionCalibrationChanged, Servo));
		return true;
	}
	return false;
}

void GPIOServoFunction::SetCalibration (GPIOServoFunctionCalibrationData *LeftMotor, GPIOServoFunctionCalibrationData *RightMotor) {
	_CurrentCalibrationSets[GPIOServoFunctionServoLeft] = LeftMotor;
	_CurrentCalibrationSets[GPIOServoFunctionServoRight] = RightMotor;
}

GPIOServoFunctionCalibrationData* GPIOServoFunction::GetCalibration (GPIOServoFunctionServo Servo) {
	if (Servo != GPIOServoFunctionServoNone) {
		return _CurrentCalibrationSets[Servo];
	}
	return 0;
}

GPIO::InternalFunctions::GPIOServoFunctionEmotion GPIOServoFunction::GetEmotion () {
	return _CurrentEmotion;
}

GPIOServoFunctionContainer GPIOServoFunction::GetDegree() {
	return *_CurrentDegree;
}


// Servo Internal Methods

int GPIOServoFunction::GetCleanValue (int Degree, int FutureValue, GPIOServoFunctionServo Servo, bool Clean) {
	if (Degree != -1) {
		double Pos[3] = {-1};
		int Buffered = Degree;

		if (Clean == false) {
			Pos[0] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionLeft);
			Pos[1] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionTop);
			Pos[2] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionRight);

			if (Degree < 90) {
				double Range = Pos[1] - Pos[0];
				double Tick = Range / 90.0;

				Buffered = Pos[0] + (((double)Degree * Tick) + 0.5);
			} else if (Degree > 90) {
				double Range = Pos[2] - Pos[1];
				double Tick = Range / 90.0;

				Buffered = Pos[0] + (((double)Degree * Tick) + 0.5);
			} else {
				Buffered = Pos[1];
			}

			Buffered = (Buffered >= Pos[2] ? Pos[2] : (Buffered <= Pos[0] ? Pos[0] : Buffered));
		}

		//if (FutureValue != Buffered) {
			return Buffered;
		//}
	}
	return -1;
}

int GPIOServoFunction::GetVirtualValue (int Degree, GPIOServoFunctionServo Servo) {
	if (Degree != -1) {
		double Pos[3] = {-1};
		int Buffered = Degree;

		Pos[0] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionLeft);
		Pos[1] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionTop);
		Pos[2] = _CurrentCalibrationSets[Servo]->GetValueForPosition(GPIOServoFunctionPositionRight);

		if (Degree < Pos[1]) {
			double Range = Pos[1] - Pos[0];
			double Tick = Range / 100.0;

			double Ff = (((double)Degree - Pos[0]) / Tick) + 0.5;

			Buffered = ((90.0 / 100.0) * Ff);
		} else if (Degree > Pos[1]) {
			double Range = Pos[2] - Pos[1];
			double Tick = Range / 100.0;

			double Ff = (((double)Degree - Pos[1]) / Tick) + 0.5;

			Buffered = 90.0 + ((90.0 / 100.0) * Ff);
		} else {
			Buffered = 90.0;
		}

		return Buffered;
	}
	return -1;
}


// Threaded Position Setter

void GPIOServoFunction::SetSynchPosition (int ServoA, int ServoB, int Degree, bool Smooth) {

	SetPosition(ServoA, ServoB, Degree, (180 - Degree), Smooth);

#if SERVO_PWM == 1
#endif

#if SERVO_PWM == 2
#else
	 int rep;
	 long hightA = 0;
	 long hightB = 0;
	 hightA = (DegreeA != -1 ? this->GetHight(DegreeA) : 0);
	 hightB = (DegreeB != -1 ? this->GetHight(DegreeB) : 0);
	 for (rep = 0; rep < 30; rep++) {
		 if (DegreeA != -1) {
			 this->Go(_portA, hightA);
		 }
		 if (DegreeB != -1 ) {
			 this->Go(_portB, hightB);
		 }
	 }
#endif
}

void GPIOServoFunction::SetPosition (int ServoA, int ServoB, int DegreeA, int DegreeB, bool Smooth) {
	if (DegreeA != -1 || DegreeB != -1) {
		int Steps[2] = {1};
		if (Smooth == true) {
			Steps[0] = abs(_CurrentDegree->Left - DegreeA);
			Steps[1] = abs(_CurrentDegree->Right - DegreeB);
		}

		for (int i = Steps[0]; i > 0; i--) {
			if (i != Steps[0]) {
				usleep(1000);
			}
			/*int IntDegree[2] = {DegreeA, DegreeB};
			if (Smooth == true) {
				IntDegree[0] = abs(_CurrentDegree->Left + (_CurrentDegree->Left - i));
				IntDegree[1] = abs(_CurrentDegree->Right + (_CurrentDegree->Right - i));
			}*/


#if SERVO_PWM == 1
			// Calculate the Range ----
			float Frequency = 50;
			float Range = (1 / Frequency / 0.0001) / 2;
			// ------------------------

			float PWM_MIN = 5.0;
			float PWM_MAX = 25.0;

			float PWM_RNG = PWM_MAX - PWM_MIN;

			float PWM_ONE = PWM_RNG / 180.0;

			float PWM_VL1 = PWM_ONE * Degree;
			float PWM_VL2 = PWM_ONE * (180 - Degree);

			float PWM = 50.00;
			float MIN_W = 0.001;
			float MAX_W = 0.002;

			float x1 = (1 / PWM) / 100.0;

			float low = MIN_W / x1;
			float high = MAX_W / x1;

			float uno = (high - low) / 100.0;

			//this->GPIOCommandF(ServoA, (float)Degree / 10.0 + 2.5, true);
			Degree = (Degree == 15 ? 0 : (Degree == 164 ? 180 : Degree));

			float baaaam = uno * (Degree / (180.0 / 100.0));

			this->GPIOCommandF(ServoA, PWM_MIN + PWM_VL1/*baaaam + low*/);
			this->GPIOCommandF(ServoB, PWM_MIN + PWM_VL2/*baaaam + low*/);
#endif

#if SERVO_PWM == 2
			float PWM_PER = 180.0 / 100.0;
			if (DegreeA != -1) {
				this->GPIOCommandF(ServoA, DegreeA / PWM_PER);
			}
			if (DegreeB != -1) {
				this->GPIOCommandF(ServoB, DegreeB / PWM_PER);
			}
#else
			int rep;
			long hightA = this->GetHight(IntDegree);
			long hightB =  this->GetHight(180 - IntDegree);

			for (rep = 0; rep < 30; rep++) {
				this->Go(ServoA, hightA);
				this->Go(ServoB, hightB);
			}
#endif
		}
	}
}

long GPIOServoFunction::GetHight (int Degree) {
	return 450 + (long)(2000.0 / 180.0 * (double)Degree);
}

void GPIOServoFunction::Go (int Servo, long Hight) {
    this->GPIOCommand(Servo, GPIO::HIGH);
    usleep(Hight);
    //delayMicroseconds(50);
    this->GPIOCommand(Servo, GPIO::LOW);
    usleep(18000/*100000*/);
}


// GPIOFunction Interface Methods

long long GPIOServoFunction::RunFunction(void *Data) {
	if (Data != 0) {
		GPIOServoFunctionContainer *DegreeContainer = (GPIOServoFunctionContainer*)Data;
		//if (DegreeContainer->Synch == true) {
			//this->SetSynchPosition(_portA, _portB, DegreeContainer->VLeft, false);
		//} else {
			this->SetPosition(_portA, _portB, DegreeContainer->VLeft, DegreeContainer->VRight);
		//}

		//float VirtualValueSA = GetVirtualValue(DegreeContainer->VLeft, GPIOServoFunctionServoLeft);
		//float VirtualValueSB = GetVirtualValue(DegreeContainer->VRight, GPIOServoFunctionServoRight);

		long long Duration = 0;
		if (_CurrentDegree->Left == -1 && _CurrentDegree->Right == -1) {
			Duration = 500;
		} else {

			int Values[2] = {0};

			//float t1 = abs(_CurrentDegree->VLeft - DegreeContainer->VLeft);
			//float t2 = abs(_CurrentDegree->VRight - DegreeContainer->VRight);

			int Distance = ((Values[0] = abs(_CurrentDegree->VLeft - DegreeContainer->VLeft)) > (Values[1] = abs(_CurrentDegree->VRight - DegreeContainer->VRight)) ? Values[0] : Values[1]);
			Duration = ((Distance / (180.0 / 100.00)) * (VoltageSpeed * 3.0) * 10.0);
		}

		Duration = 500;

		_CurrentDegree->Left = (DegreeContainer->Left != -1 ? DegreeContainer->Left : _CurrentDegree->Left);
		_CurrentDegree->Right = (DegreeContainer->Right != -1 ? DegreeContainer->Right : _CurrentDegree->Right);
		_CurrentDegree->Synch = DegreeContainer->Synch;
		_CurrentEmotion = GPIOServoFunctionContainer::Emotion(*_CurrentDegree);

		SendEvent(new ServoValueEvent(*_CurrentDegree), Duration);

		return Duration;
	}
	return 0;
}

// Internal Methods

void GPIOServoFunction::SetPorts (int PortA, int PortB) {

	_portA = PortA;
	_portB = PortB;

	vector<GPIOFunctionPortStruct> Ports;
#if SERVO_PWM == 1
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortA, GPIOFunctionPortStructDirection::UNDEFINE, GPIOType::GPIOTypePWM));
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortB, GPIOFunctionPortStructDirection::UNDEFINE, GPIOType::GPIOTypePWM));
#endif

#if SERVO_PWM == 2
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortA, GPIOFunctionPortStructDirection::UNDEFINE, GPIOType::GPIOTypePWMRaw));
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortB, GPIOFunctionPortStructDirection::UNDEFINE, GPIOType::GPIOTypePWMRaw));
#else
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortA, GPIOFunctionPortStructDirection::OUTPUT));
	Ports.insert(Ports.end(), GPIOFunctionPortStruct(PortB, GPIOFunctionPortStructDirection::OUTPUT));
#endif

	this->MapPorts(Ports);
}

} /* namespace InternalFunctions */
} /* namespace GPIO */
