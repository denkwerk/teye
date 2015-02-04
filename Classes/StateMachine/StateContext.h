/*
 * 		StateContext.h
 *
 *  	Created on: 26.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef STATECONTEXT_H_
#define STATECONTEXT_H_


/* ------------------------------------------------------------------------------------
 * ------------------------------------ INCLUDES --------------------------------------
 * ------------------------------------------------------------------------------------ */

#include "../Event/EventHandler.h"

#include "../UI/UIWindow.h"

#ifndef _WITHOUT_SPEECH_RECOGNITION
#include "../SpeechDetection/EXSpeechController.h"
#endif

#ifdef _OPEN_CV
#include "../ImageRecognition/EXImageRecognition.h"
#endif

#include "../SIP/SIPController.h"
#include "../GPIO/GPIOController.h"

#include "../GPIOFunctions/GPIOServoFunction.h"
#include "../GPIOFunctions/GPIOMCP3008Function.h"

#include "../Motion/EXMotionDetection.h"

#include <sstream>
#include <iostream>
#include <vector>


/* ------------------------------------------------------------------------------------
 * ------------------------------------ STATE MACHINE ---------------------------------
 * ------------------------------------------------------------------------------------ */

namespace StateMachine {


	// acquaint class

	class StateContext;


	// state interface class

	class StateInterface : public EventHandler {

		friend class StateContext;


		public:


			// destructor

			virtual ~StateInterface() {};


			// virtual event methods

#ifndef _WITHOUT_SPEECH_RECOGNITION
			virtual void ReceiveSpeakEvent 				(const EXSpeech::EXSpeechTypes::SpeechEvent *Event)			{};
			virtual void ReceiveSpeakRecognizeEvent		(const EXSpeech::EXSpeechTypes::SpeechRecognizeEvent *Event)	{};
#endif
#ifdef _OPEN_CV
			virtual void ReceiveImageEvent				(const ImageRecognition::ImageEvent *Event)					{};
			virtual void ReceiveImageRecognizeEvent		(const ImageRecognition::ImageRecognizeEvent *Event)		{};
#endif
			virtual void ReceiveRawMotionData			(const GPIO::InternalFunctions::MotionRawEvent *Event)		{if (MotionDetection != 0) {MotionDetection->RawMotionData(Event->RawMotionValue);}};
			virtual void ReceiveMotionEvent				(const MotionDetection::MotionEvent *Event)					{};
			virtual void ReceiveMotionCalibrationEvent	(const MotionDetection::MotionCalibrationEvent *Event)		{};
			virtual void ReceiveServoEvent				(const ServoEvent *Event)									{};
			virtual void ReceiveServoValueEvent			(const ServoValueEvent *Event)								{};
			virtual void ReceiveServoEmotionEvent		(const ServoEmotionEvent*Event)								{};
			virtual void ReceiveSystemEvent				(const SystemEvent *Event)									{};
			virtual void ReceiveKeyEvent				(const KeyEvent *Event)										{};
			virtual void ReceiveSIPEvent				(const SIP::SIPEvent *Event)								{};
			virtual void ReceiveSIPCallEvent			(const SIP::SIPCallEvent *Event, bool Accepted = false)		{if (Accepted == false && SIPController != 0 && Event->CallState == SIP::SIPAccountStateIncomingCall) {SIPController->SIPRejectIncomingCall(SIPController->SIPCurrentCall());}};
			virtual void ReceiveSIPCallStateEvent		(const SIP::SIPCallStateEvent *Event)						{};
			virtual void ReceiveSIPStateEvent			(const SIP::SIPStateEvent *Event)							{};
			virtual void ReceiveWindowEvent				(const UI::WindowEvent *Event)								{};


			// speech command

			std::string GetSpeechCommandsAsString(std::string Separator = std::string(", ")) {
				RecognitionCommandMap::iterator it = ListeningMap.begin();
				std::string s;
				while (it != ListeningMap.end()) {
					std::string Value = it->first;
					it++;
					s.append(std::string("\"").append(Value).append("\"").append((it == ListeningMap.end() ? "" : Separator)));
				}
				return s;
			}


		private:

			std::string ClassDebugName;


		protected:


			// constructors

			StateInterface(std::string Name) : StateInterface() {ClassDebugName = Name;};
			StateInterface() : Context(0), WindowController(0), SIPController(0), IOController(0), MCP3008Function(0), ServoFunction(0), MotionDetection(0), ApplicationSettings(0) {

#ifndef _WITHOUT_SPEECH_RECOGNITION
				SpeechController = 0;
#endif
#ifdef _OPEN_CV
				ImageRController = 0;
#endif
			};


			// protected virtual methods

			virtual void Initialize() = 0;
			virtual void GoAway()			{};
			virtual void Uninitialize()		{};
			void ComputeDebug() {
				if (ClassDebugName.size() > 0) {
					NewLine();
					PrintText(std::string("###DEBUG### || Enter State: \"").append(ClassDebugName).append("\""), true);
				}
			};


			// typedefs

			typedef map<string, string> RecognitionCommandMap;


			// protected members

			StateContext									*Context;

			UI::UIWindow									*WindowController;
#ifndef _WITHOUT_SPEECH_RECOGNITION
			EXSpeech::EXSpeechController						*SpeechController;
#endif
#ifdef _OPEN_CV
			ImageRecognition::EXImageRecognition			*ImageRController;
#endif
			SIP::SIPController								*SIPController;
			GPIO::GPIOController							*IOController;

			GPIO::InternalFunctions::GPIOMCP3008Function	*MCP3008Function;
			GPIO::InternalFunctions::GPIOServoFunction		*ServoFunction;

			MotionDetection::EXMotionDetection				*MotionDetection;

			SIP::PersistentStore::ApplicationSettings		*ApplicationSettings;

			RecognitionCommandMap							ListeningMap;

			string _string_buffer;


			// protected methods

			bool MapInput(const KeyEvent *Event, string &Result, bool Clear = false, bool Display = true) {
				if (Clear == false) {
					if (Event->Key == KEY_ACTION_ENTER) {
						//printf("\n");
						Result = _string_buffer;
						_string_buffer.clear();
						return true;
					} else if (Event->Key == KEY_ACTION_BACKSPACE) {
						// Remove Sign on Terminal
						if (_string_buffer.size() > 0) {
							if (Display == true) {
								printf("\b \b");
							}
						}
						// Remove Sign in Buffer
						_string_buffer = _string_buffer.substr(0, _string_buffer.length() -1);
					} else {
						if (Display == true) {
							printf(Event->Symbol.c_str());
						}
						_string_buffer.append(Event->Symbol);
					}
				} else {
					_string_buffer.clear();
					return true;
				}

				return false;
			}

			void CleanUp (std::string Text) {
				if (Text.length() > 0) {
					for (unsigned int i = 0; i < Text.length(); i++) {
						printf("\b \b");
					}
				}
			}

			static void S_NewLine() {
				printf("\n");
			}

			void NewLine() {
				printf("\n");
			}

			static void S_DeleteLastSign () {
				printf("\b \b");
			}

			void DeleteLastSign () {
				printf("\b \b");
			}

			static void S_PrintText(std::string Text, bool NL = false) {
				printf(Text.c_str());
				if (NL == true) {
					StateMachine::StateInterface::S_NewLine();
				}
			}

			void PrintText(std::string Text, bool NL = false) {
				printf(Text.c_str());
				if (NL == true) {
					NewLine();
				}
			}

			int ToNumber(std::string Result) {return atoi(Result.c_str());};
			bool ToSaveNumber(std::string Result, int &Number) {char *xx = 0;Number = std::strtol(Result.c_str(), &xx, 10);if (*xx == '\0') {return true;}return false;};
			std::vector<std::string> SplitString(std::string String, char Token) {std::vector<std::string> strings;std::istringstream f(String);std::string s;while (std::getline(f, s, Token)) {strings.push_back(s);}return strings;};

			bool ComputeSpeechCommands (string *Command, string Detection, RecognitionCommandMap Map, bool Output) {
				if (Detection.size() > 0) {
					*Command = "";

					map<string, string>::iterator RecognitionCommandIterator;
					RecognitionCommandIterator = Map.find(Detection);
					if (RecognitionCommandIterator != Map.end()) {
						*Command = RecognitionCommandIterator->second;
					}

					if (*Command == "") {
						//RecognitionIterator = _RecognitionMap.find(EXMainModeGlobal);
						//*Command = FindCommandInMode(RecognitionIterator, Detection);
						}

					if (WindowController != 0 && WindowController->IsInitialized() == true && Output == true) {
						WindowController->SetSpeakInterfaceValue(UI::UISpeakInterfaceValueTypeRecognition, Detection);
					} else {
						if (Detection.size() != 0) {
							string OUT = string("Word Detected: ");
							OUT.append(Detection);
							OUT.append("\n");
							printf(OUT.c_str());
						}
					}

					if (*Command != "") {
						return true;
					}
				}

				return false;
			}
	};


	// state context class

	class StateContext {

		friend class StateInterface;


		public:


			// constructors

#ifdef _WITHOUT_SPEECH_RECOGNITION
#ifdef _OPEN_CV
			StateContext(UI::UIWindow *WC, SIP::SIPController *SIC, GPIO::GPIOController *IOC, ImageRecognition::EXImageRecognition *IRC) : _State(0), _WindowController(WC), _SIPController(SIC), _IOController(IOC), _IRController(IRC), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), _ApplicationSettings(0) {
#else
			StateContext(UI::UIWindow *WC, SIP::SIPController *SIC, GPIO::GPIOController *IOC) : _State(0), _WindowController(WC), _SIPController(SIC), _IOController(IOC), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), _ApplicationSettings(0) {
#endif
#else
#ifdef _OPEN_CV
			StateContext(UI::UIWindow *WC, EXSpeech::EXSpeechController *SC, SIP::SIPController *SIC, GPIO::GPIOController *IOC, ImageRecognition::EXImageRecognition *IRC) : _State(0), _WindowController(WC), _SpeechController(SC), _SIPController(SIC), _IOController(IOC), _IRController(IRC), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), _ApplicationSettings(0) {
#else
			StateContext(UI::UIWindow *WC, EXSpeech::EXSpeechController *SC, SIP::SIPController *SIC, GPIO::GPIOController *IOC) : _State(0), _WindowController(WC), _SpeechController(SC), _SIPController(SIC), _IOController(IOC), _MCP3008Function(0), _ServoFunction(0), _MotionDetection(0), _ApplicationSettings(0) {
#endif
#endif
			};


			// destructors

			virtual ~StateContext() {};


			// public methods

			void SetApplicationSettings (SIP::PersistentStore::ApplicationSettings *APPSettings) {_ApplicationSettings = APPSettings;};
			void SetIO (GPIO::InternalFunctions::GPIOMCP3008Function *MCP3008, GPIO::InternalFunctions::GPIOServoFunction *Servo, MotionDetection::EXMotionDetection *Motion) {_MCP3008Function = MCP3008; _ServoFunction = Servo; _MotionDetection = Motion;};
			bool SaveSettings() {return _SIPController->SaveApplicationSettings(*_ApplicationSettings);};

			StateInterface* Context() {return _State;};

			void SetState (StateInterface *NewState, bool Released = true) {
				if ((_State != 0 && _State != NewState) || _State == 0) {
					NewState->Context = this;
					NewState->WindowController		= _WindowController;
	#ifndef _WITHOUT_SPEECH_RECOGNITION
					NewState->SpeechController		= _SpeechController;
	#endif
	#ifdef _OPEN_CV
					NewState->ImageRController		= _IRController;
	#endif
					NewState->SIPController			= _SIPController;
					NewState->IOController			= _IOController;
					NewState->MCP3008Function		= _MCP3008Function;
					NewState->ServoFunction			= _ServoFunction;
					NewState->ApplicationSettings	= _ApplicationSettings;
					NewState->MotionDetection		= _MotionDetection;
					if (_State != 0) {
						_State->GoAway();
					}
					if (Released == true) {
						if (_State != 0) {
							_State->Uninitialize();
							delete _State;
						}
					}
					_State = NewState;
				}
				NewState->ComputeDebug();
				NewState->Initialize();
			};


		private:

			// private member

			StateInterface									*_State;


		protected:


			// protected member

			UI::UIWindow									*_WindowController;
#ifndef _WITHOUT_SPEECH_RECOGNITION
			EXSpeech::EXSpeechController						*_SpeechController;
#endif
			SIP::SIPController								*_SIPController;
			GPIO::GPIOController							*_IOController;
#ifdef _OPEN_CV
			ImageRecognition::EXImageRecognition			*_IRController;
#endif

			GPIO::InternalFunctions::GPIOMCP3008Function	*_MCP3008Function;
			GPIO::InternalFunctions::GPIOServoFunction		*_ServoFunction;

			MotionDetection::EXMotionDetection				*_MotionDetection;

			SIP::PersistentStore::ApplicationSettings		*_ApplicationSettings;
	};

} /* namespace StateMachine */

#endif /* STATECONTEXT_H_ */
