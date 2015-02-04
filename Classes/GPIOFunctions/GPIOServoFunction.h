/*
 * 		GPIOServoFunction.h
 *
 *  	Created on: 16.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef GPIOSERVOFUNCTION_H_
#define GPIOSERVOFUNCTION_H_

#include "../GPIO/GPIOFunction.h"
#include <stdint.h>

#include <map>

#define SERVO_DEBUG 0
#define SERVO_PWM 2

#define SERVO_VOLTAGE 3.5

using namespace std;

namespace GPIO {

	namespace InternalFunctions {

		enum GPIOServoFunctionEventType {
			GPIOServoFunctionNone,
			GPIOServoFunctionInitDone,
			GPIOServoFunctionValueChanged,
			GPIOServoFunctionEmotionChanged,
			GPIOServoFunctionCalibrationChanged
		};

		enum GPIOServoFunctionEmotion {
			GPIOServoFunctionEmotionNone = -1,
			GPIOServoFunctionEmotionAngry = 0,
			GPIOServoFunctionEmotionSad = 1,
			GPIOServoFunctionEmotionEyeBlinkLeft = 2,
			GPIOServoFunctionEmotionEyeBlinkRight = 3,
			GPIOServoFunctionEmotionSleep = 4,
			GPIOServoFunctionEmotionHello = 5,
			GPIOServoFunctionEmotionCurios = 6
		};

		enum GPIOServoFunctionPosition {
			GPIOServoFunctionPositionNone = -1,
			GPIOServoFunctionPositionTop = 0,
			GPIOServoFunctionPositionLeft = 1,
			GPIOServoFunctionPositionRight = 2
		};

		enum GPIOServoFunctionServo {
			GPIOServoFunctionServoNone = -1,
			GPIOServoFunctionServoLeft = 0,
			GPIOServoFunctionServoRight = 1
		};

		struct GPIOServoFunctionContainer {
			public:
				GPIOServoFunctionContainer () : Left(-1), Right(-1), VLeft(-1), VRight(-1), Synch(false) {};
				GPIOServoFunctionContainer(int Left) : Left(Left), Right(-1), VLeft(-1), VRight(-1), Synch(true) {};
				GPIOServoFunctionContainer(int Left, int Right, bool Synch = false) : Left(Left), Right(Right), VLeft(-1), VRight(-1), Synch(Synch) {};
				GPIOServoFunctionContainer(const GPIOServoFunctionContainer& Copy) {
					Left	= Copy.Left;
					Right	= Copy.Right;
					VLeft 	= Copy.VLeft;
					VRight	= Copy.VRight;
					Synch	= Copy.Synch;
				};
				virtual ~GPIOServoFunctionContainer() {};

				static map<GPIOServoFunctionEmotion, GPIOServoFunctionContainer> GPIOServoFunctionContainerCreate() {
					map<GPIOServoFunctionEmotion, GPIOServoFunctionContainer> m;

					m[GPIOServoFunctionEmotionAngry]		= GPIOServoFunctionContainer(145);
					m[GPIOServoFunctionEmotionSad]			= GPIOServoFunctionContainer(35);
					m[GPIOServoFunctionEmotionEyeBlinkLeft] = GPIOServoFunctionContainer(90, 180);
					m[GPIOServoFunctionEmotionEyeBlinkRight]= GPIOServoFunctionContainer(0, 90);
					m[GPIOServoFunctionEmotionSleep]		= GPIOServoFunctionContainer(0);
					m[GPIOServoFunctionEmotionHello]		= GPIOServoFunctionContainer(90);
					m[GPIOServoFunctionEmotionCurios]		= GPIOServoFunctionContainer(45, 45, false);

					return m;
				};

				typedef map<GPIOServoFunctionEmotion, GPIOServoFunctionContainer> EmotionMap;

				bool operator== (GPIOServoFunctionContainer &m) {
					int a[2] = {Left, (Synch == true ? Left : Right)};
					int b[2] = {m.Left, (m.Synch == true ? m.Left : m.Right)};

					return (a[0] == b[0] && a[1] == b[1]);
				};

				static GPIOServoFunctionEmotion Emotion(GPIOServoFunctionContainer m) {
					EmotionMap _TMPEmotions = GPIOServoFunctionContainer::GPIOServoFunctionContainerCreate();
					EmotionMap::iterator it = _TMPEmotions.begin();
					while (it != _TMPEmotions.end()) {
						GPIOServoFunctionContainer Con = it->second;
						if (m == Con) {
							return it->first;
						}
						it++;
					}
					return GPIOServoFunctionEmotionNone;
				};

				int Left;
				int Right;
				int VLeft;
				int VRight;
				bool Synch;

			private:

		};

		struct GPIOServoFunctionCalibrationData {
			public:
				GPIOServoFunctionCalibrationData(GPIOServoFunctionServo Servo) : CurrentPosition(GPIOServoFunctionPositionNone), Servo(Servo), Position{90, 0, 180} {};
				virtual ~GPIOServoFunctionCalibrationData() {};

				void SetPosition (GPIOServoFunctionPosition Position) {CurrentPosition = Position;};
				GPIOServoFunctionPosition GetPosition () {return CurrentPosition;};
				GPIOServoFunctionServo GetServo () {return Servo;};
				bool SetValueForPosition (int Value) {if (CurrentPosition != GPIOServoFunctionPositionNone) {Position[CurrentPosition] = Value;return true;}return false;};
				int GetValueForPosition (GPIOServoFunctionPosition SPosition) {if (SPosition != GPIOServoFunctionPositionNone) {return Position[SPosition];}return -1;};

			private:
				GPIOServoFunctionPosition CurrentPosition;
				GPIOServoFunctionServo Servo;
				int Position[3];
		};

		class ServoControlEvent : public Event {
			public:
				ServoControlEvent(GPIOServoFunctionContainer ServoValueContainer) : ServoValueContainer(ServoValueContainer) {};
				GPIOServoFunctionContainer ServoValueContainer;
		};

		class ServoEmotionControlEvent : public Event {
			public:
				ServoEmotionControlEvent(GPIOServoFunctionEmotion Emotion) : Emotion(Emotion) {};
				GPIOServoFunctionEmotion Emotion;
		};

		class ServoEvent : public Event {
			public:
				ServoEvent() : ServoType(GPIOServoFunctionNone), Servo(GPIOServoFunctionServoNone) {};
				ServoEvent(GPIOServoFunctionEventType Type, GPIOServoFunctionServo Servo = GPIOServoFunctionServoNone) : ServoType(Type), Servo(Servo) {};

				GPIOServoFunctionEventType ServoType;
				GPIOServoFunctionServo Servo;
		};

		class ServoValueEvent : public ServoEvent {
			public:
			ServoValueEvent() : ServoEvent(GPIOServoFunctionValueChanged), ServoValueContainer(-1, -1, false) {};
			ServoValueEvent(int DegreeLeft, int DegreeRight) : ServoEvent(GPIOServoFunctionValueChanged), ServoValueContainer(DegreeLeft, DegreeRight, false) {};
			ServoValueEvent(int DegreeLeft, int DegreeRight, bool Synch) : ServoEvent(GPIOServoFunctionValueChanged), ServoValueContainer(DegreeLeft, DegreeRight, Synch) {};
			ServoValueEvent(int Degree) : ServoEvent(GPIOServoFunctionValueChanged), ServoValueContainer(Degree, -1, true) {};
			ServoValueEvent(GPIOServoFunctionContainer ServoValueContainer) : ServoEvent(GPIOServoFunctionValueChanged), ServoValueContainer(ServoValueContainer) {};

			GPIOServoFunctionContainer ServoValueContainer;
		};

		class ServoEmotionEvent : public ServoEvent {
			public:
			ServoEmotionEvent() : ServoEvent(GPIOServoFunctionEmotionChanged), _Emotion(GPIOServoFunctionEmotionNone) {};
			ServoEmotionEvent(GPIOServoFunctionEmotion Emotion) : ServoEvent(GPIOServoFunctionEmotionChanged), _Emotion(Emotion) {};

			GPIOServoFunctionEmotion _Emotion;
		};

		class GPIOServoFunction : public GPIOFunction {
			public:

				static GPIOServoFunction* GPIOServoFunctionWithPort (int PortA, int PortB);

				void ReceiveServoEvent (const ServoControlEvent *Event);
				void ReceiveServoEmotionEvent (const ServoEmotionControlEvent *Event);

				bool ClearAllJobs ();
				bool SetContainer (GPIOServoFunctionContainer Degree, bool Clean = false, bool Force = false);
				bool SetDegree (int Degree);
				bool SetSynchronDegree (int Degree);
				bool SetEmotion (GPIOServoFunctionEmotion Emotion);
				bool SetCalibrationPosition(GPIOServoFunctionServo Servo, GPIOServoFunctionPosition Position);
				bool SaveCalibrationPosition(GPIOServoFunctionServo Servo, int Value);
				void SetCalibration (GPIOServoFunctionCalibrationData *LeftMotor, GPIOServoFunctionCalibrationData *RightMotor);
				GPIOServoFunctionCalibrationData* GetCalibration (GPIOServoFunctionServo Servo);
				GPIOServoFunctionEmotion GetEmotion ();
				GPIOServoFunctionContainer GetDegree();

			private:

				// Internal Variables
				int _portA;
				int _portB;
				GPIOServoFunctionEmotion _CurrentEmotion;
				GPIOServoFunctionContainer *_CurrentDegree;
				GPIOServoFunctionContainer *_FutureDegree;
				GPIOServoFunctionCalibrationData *_CurrentCalibrationSets[2];

				float VoltageSpeed;

				const GPIOServoFunctionContainer::EmotionMap GPIOEmotions = GPIOServoFunctionContainer::GPIOServoFunctionContainerCreate();

				// Private De -and Constructors
				GPIOServoFunction();
				virtual ~GPIOServoFunction();

				// Servo Internal Methods
				int GetCleanValue (int Degree, int FutureValue, GPIOServoFunctionServo Servo, bool Clean = false);
				int GetVirtualValue (int Degree, GPIOServoFunctionServo Servo);
				void SetSynchPosition (int ServoA, int ServoB, int Degree, bool Smooth = false);
				void SetPosition (int ServoA, int ServoB, int DegreeA, int DegreeB, bool Smooth = false);
				long GetHight (int Degree);
				void Go (int Servo, long Hight);

				// GPIOFunction Interface Methods
				long long RunFunction(void *Data = 0);

				// Internal Methods
				void SetPorts (int PortA, int PortB);
		};

	} /* namespace InternalFunctions */
} /* namespace GPIO */

#endif /* GPIOSERVOFUNCTION_H_ */
