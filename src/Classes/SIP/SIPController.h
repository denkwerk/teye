/*
 * 		SIPController.h
 *
 *  	Created on:	14.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef SIPCONTROLLER_H_
#define SIPCONTROLLER_H_


/* ------------------------------------------------------------------------------------
 * ------------------------------------ DEFINES ---------------------------------------
 * ------------------------------------------------------------------------------------ */

//#define ENABLE_SIP_SILENT_LOG

#define PJ_IS_LITTLE_ENDIAN 1
#define PJ_IS_BIG_ENDIAN 1

/* Ringtones		    	US	       UK  */
#define RINGBACK_FREQ1	    440	    /* 400 */
#define RINGBACK_FREQ2	    480	    /* 450 */
#define RINGBACK_ON	    	2000    /* 400 */
#define RINGBACK_OFF	    4000    /* 200 */
#define RINGBACK_CNT	    1	    /* 2   */
#define RINGBACK_INTERVAL   4000    /* 2000 */

#define RING_FREQ1	    	800
#define RING_FREQ2	    	640
#define RING_ON		    	200
#define RING_OFF	    	100
#define RING_CNT	    	3
#define RING_INTERVAL		3000


/* ------------------------------------------------------------------------------------
 * ------------------------------------ INCLUDES --------------------------------------
 * ------------------------------------------------------------------------------------ */

#include <pjsua2.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../Event/EventHandler.h"


/* ------------------------------------------------------------------------------------
 * ------------------------------------ NAMESPACES ------------------------------------
 * ------------------------------------------------------------------------------------ */

using namespace std;
using namespace pj;


/* ------------------------------------------------------------------------------------
 * ------------------------------------ SIP IMPLEMENTATION ----------------------------
 * ------------------------------------------------------------------------------------ */

namespace SIP {


	// persistent store

	namespace PersistentStore {


		// servo settings class

		class ServoSettings : public PersistentObject {

			public:

				// de- and constructor

				ServoSettings(string Suffix = "") : _Calibration_Left(0), _Calibration_Top(90), _Calibration_Right(180), Suffix(Suffix) {};
				virtual ~ServoSettings() {};


				// public methods

				void SetSuffix(string InSuffix) {Suffix = InSuffix;};


				// public virtual methods

				virtual void readObject(const ContainerNode &node) throw(Error);
				virtual void writeObject(ContainerNode &node) const throw(Error);


				// public members

				int				_Calibration_Left;
				int				_Calibration_Top;
				int				_Calibration_Right;

				string			Suffix;

		};


		// application settings class

		class ApplicationSettings : public PersistentObject {

			public:

				// de- and constructor

				ApplicationSettings() : Changes(false),
										_Motion_Calibration_Min(0),
										_Motion_Calibration_Max(0),
										_Motion_Calibration_Threshold(0),
										_Motion_Calibration_Timeout(0),
										_Motion_Calibration_CalibrationOffset(0),
										_Motion_Port_CLK(-1),
										_Motion_Port_DOut(-1),
										_Motion_Port_DIn(-1),
										_Motion_Port_CS(-1),
										_Servo_Port_A(-1),
										_Servo_Port_B(-1),
										_Speak_LogOutput(0),
										_Speak_Language(-1),
										_Speak_SampelRate(0),
										_Speak_BestPath(0),
										_SIP_Config_Codec_Type(2),
										_SIP_Config_Ringtone(1) {

					_Speak_ErrorFilename = string();
					_Speak_LogFilename = string();
					_Speak_ModelDir = string();
					_Speak_Dictionary = string();
					_Speak_AudioModel = string();
					_SIP_Config_Ringtone_File = string("duckhunt.wav");
					_SIP_Config_Receiver = string("2000 <sip:2000@10.0.66.160>");
					_SIP_Config_Endpoint = EpConfig();
					_SIP_Config_Transport = TransportConfig();
					_SIP_Config_Account = AccountConfig();
					_Servo_Calibration_A.SetSuffix("A");
					_Servo_Calibration_B.SetSuffix("B");
				};

				virtual ~ApplicationSettings() {};


				// public virtual methods

			    virtual void readObject(const ContainerNode &node) throw(Error);
			    virtual void writeObject(ContainerNode &node) const throw(Error);


			    // public members

			    bool			Changes;
				int				_Motion_Calibration_Min;
				int				_Motion_Calibration_Max;
				int				_Motion_Calibration_Threshold;
				int				_Motion_Calibration_Timeout;
				int				_Motion_Calibration_CalibrationOffset;
				int				_Motion_Port_CLK;
				int				_Motion_Port_DOut;
				int				_Motion_Port_DIn;
				int				_Motion_Port_CS;
				int				_Servo_Port_A;
				int				_Servo_Port_B;
				ServoSettings	_Servo_Calibration_A;
				ServoSettings	_Servo_Calibration_B;
				int				_Speak_LogOutput;
				int				_Speak_Language;
				string			_Speak_ErrorFilename;
				string			_Speak_LogFilename;
				string			_Speak_ModelDir;
				string			_Speak_Dictionary;
				string			_Speak_AudioModel;
				int				_Speak_SampelRate;
				int				_Speak_BestPath;
				int				_SIP_Config_Codec_Type;
				int				_SIP_Config_Ringtone;
				string			_SIP_Config_Ringtone_File;
				string			_SIP_Config_Receiver;
				EpConfig		_SIP_Config_Endpoint;
				TransportConfig	_SIP_Config_Transport;
				AccountConfig	_SIP_Config_Account;
		};
	}


	using namespace PersistentStore;

	namespace Internal {

		class SIPCall : public Call, public EventHandler {

			public:


				// de- and constructors

				//SIPCall (Account& acc, int call_id = PJSUA_INVALID_ID);
				SIPCall (Account& acc, int call_id = PJSUA_INVALID_ID, void *_SIPAccount = 0);
				~SIPCall() {};


				// public virtual methods

			    virtual void onCallState(OnCallStateParam &prm);
			    virtual void onCallMediaState(OnCallMediaStateParam &prm);


			private:


			    // private members

			    void 				*_SIPAccount;


			    // private methods

			    void Ringtone (bool Start = true);
		};


		// Subclass to extend the Account and get notifications etc.

		class SIPAccount : public Account, public EventHandler {

			friend class SIPCall;

			public:

				// public members

				enum AudioType {
					AudioTypeNone			= -1,
					AudioTypeIncomingCall	= 1,
					AudioTypeOutgoingCall
				};


				// de- and constructors

				//SIPAccount () : Account(), _SIP_CurrentCall(0), _SIPController(0) {};
				SIPAccount (void *_SIPController = 0) : Account(), _SIP_CurrentCall(0), _AudioIncomingTone(0), _SIPController(_SIPController) {};


				// public methods

				bool			CheckCall				();
				bool			Connected				();
				bool			Call					(std::string Receiver);
				bool			HangUp					();
				SIPCall*		GetCall					();
				void			SetAudioPlayer			(AudioMediaPlayer *AudioPlayer, AudioType Type) {if (Type == AudioTypeIncomingCall) {_AudioIncomingTone = AudioPlayer;}};
				void			StartRingRing			(bool Start);


				// public virtual methods

				virtual void	onRegState				(OnRegStateParam &prm);
				virtual void	onIncomingCall			(OnIncomingCallParam &prm);
				virtual void	onRegStarted			(OnRegStartedParam &prm);
				virtual void	onIncomingSubscribe		(OnIncomingSubscribeParam &prm);
				virtual void	onInstantMessage		(OnInstantMessageParam &prm);
				virtual void	onInstantMessageStatus	(OnInstantMessageStatusParam &prm);
				virtual void	onTypingIndication		(OnTypingIndicationParam &prm);
				virtual void	onMwiInfo				(OnMwiInfoParam &prm);

			private:


				// private members

				SIPCall				*_SIP_CurrentCall;
				AudioMediaPlayer	*_AudioIncomingTone;

			protected:


				// protected methods

				void ReleaseCall();


				// protected members

				void 				*_SIPController;
		};

	}


	using namespace SIP::Internal;


	// enumerators

	enum SIPStatus {
		SIPStatusError							= -1,
		SIPStatusNone							= 0,
		SIPStatusStarting,
		SIPStatusStarted,
		SIPStatusEnding,
		SIPStatusCall,
		SIPStatusStateChange,
		SIPStatusStop
	};

	enum SIPCallState {
		SIPCallStateCall,
		SIPCallStateCallMedia,
	};

	enum SIPCallStateType {
		SIPCallStateTypeNone,
		SIPCallStateTypeConnecting,
		SIPCallStateTypeTSX,
		SIPCallStateTypeConnected,
		SIPCallStateTypeDisconnected,
		SIPCallStateTypeCalling
	};

	enum SIPAccountState {
		SIPAccountStateIncomingCall,
		SIPAccountStateIncomingReg,
		SIPAccountStateRegStarted,
		SIPAccountStateIncomingSubscribe,
		SIPAccountStateInstantMessage,
		SIPAccountStateInstantMessageStatus,
		SIPAccountStateTypingIndicator,
		SIPAccountStateMWIInfo,
		SIPAccountStateCallHangUp,
		SIPAccountStateCallAccepted,
		SIPAccountStateChange,
	};

	enum SIPConnectionState {
		SIPStateNone,
		SIPStateAvailable,
		SIPStateNotAvailable
	};

	enum SIPAnswerType {
		SIPAnswerTypeAccept,
		SIPAnswerTypeReject
	};


/* ------------------------------------------------------------------------------------
 * ------------------------------------ EVENTS ----------------------------------------
 * ------------------------------------------------------------------------------------ */

	class SIPEvent : public Event {

		public:

			// de- and constructors

			SIPEvent() : Status(SIPStatusNone) {};
			SIPEvent(SIPStatus Status) : Status(Status) {};


			// public members

			SIPStatus Status;
	};

	class SIPCallEvent : public SIPEvent {

		friend class SIPController;

		public:

			// de- and constructors
			SIPCallEvent(SIPAccountState CallState) : SIPEvent(SIPStatusStarted), CallState(CallState), Receiver(""), EvAccepted(false), Call(0) {};
			SIPCallEvent(SIPAccountState CallState, string Receiver, SIPCall *Call) : SIPEvent(SIPStatusCall), CallState(CallState), Receiver(Receiver), EvAccepted(false), Call(Call) {};

			// public methods
			void EventAccepted() {EvAccepted = true;};
			bool Accepted() {return EvAccepted;};

			// public members
			SIPAccountState		CallState;
			string				Receiver;

		private:

			// private members
			bool				EvAccepted;

		protected:

			// protected members
			SIPCall				*Call;

	};

	class SIPCallStateEvent : public SIPEvent {

		friend class SIPController;

		public:

			// de- and constructors
			SIPCallStateEvent(SIPCallState CallState, SIPCallStateType StateType, SIPCall *Call = 0) : SIPEvent(SIPStatusStarted), CallState(CallState), StateType(StateType), Call(Call) {};
			virtual ~SIPCallStateEvent() {};

			// public members
			SIPCallState		CallState;
			SIPCallStateType	StateType;

		protected:

			// protected members
			SIPCall				*Call;
	};

	class SIPCallInternalStateEvent : public SIPEvent {
		public:
			SIPCallInternalStateEvent(SIPCallState CallState, SIPCallStateType StateType, SIPCall *Call = 0) : SIPEvent(SIPStatusStarted), CallState(CallState), StateType(StateType), Call(Call) {};
			virtual ~SIPCallInternalStateEvent() {};

			SIPCallState CallState;
			SIPCallStateType StateType;

		protected:
			SIPCall *Call;
	};

	class SIPStateEvent : public SIPEvent {
		public:
			SIPStateEvent(SIPConnectionState SIPState) : SIPEvent(SIPStatusStateChange), SIPState(SIPState) {};

			SIPConnectionState SIPState;
	};

	class SIPCallAnswerEvent : public SIPEvent {
		public:
			SIPCallAnswerEvent(const SIPCallEvent *Event, SIPAnswerType AnswerType) : CallEvent(Event), AnswerType(AnswerType) {};

			const SIPCallEvent *CallEvent;
			SIPAnswerType AnswerType;
	};

	class SIPController : public EventHandler {

		public:
			SIPController();
			virtual ~SIPController();

			// Types

			enum SIPCodecType {
				SIPCodecTypeNone			= 0,
				SIPCodecTypePCMU8000		= 1,
				SIPCodecTypePCMA8000		= 2,
				SIPCodecTypeSpeex8000		= 3,
				SIPCodecTypeSpeex16000		= 4,
				SIPCodecTypeSpeex32000		= 5,
				SIPCodecTypeiLBC8000		= 6,
				SIPCodecTypeGSM8000			= 7,
				SIPCodecTypeG72216000		= 8,
				SIPCodecTypeL1680001		= 9,
				SIPCodecTypeL1680002		= 10,
				SIPCodecTypeL16160001		= 11,
				SIPCodecTypeL16160002		= 12,
				SIPCodecTypeL16441001		= 13,
				SIPCodecTypeL16441002		= 14,
			} _CurrentCodecType;

			// Initialize
			void SetAbsolutePath (std::string AbsolutePath);
			bool InitializeSIP (string SIPURL, string SIPREGISTRAR, string SIPREALM, string SIPUSER, string SIPPASSWD);
			bool InitializeSIP (ApplicationSettings &Settings);
			bool UninitializeSIP ();
			bool IsInitialized ();

			// Settings
			std::string GetCodec(SIPController::SIPCodecType Type = SIPCodecTypeNone);

			// Persistent
			bool LoadApplicationSettings (ApplicationSettings &Settings);
			bool SaveApplicationSettings (ApplicationSettings Settings);
			void SetFilename (string Filename);

			// Status
			bool SetAvailable ();
			bool SetNotAvailable ();

			// Calling
			bool SIPCallReady ();
			bool SIPCallConnected ();
			bool IPCallReady ();
			bool SIPCallToNumber (string Receiver);
			bool SIPHangUpCall ();
			SIPCall* SIPCurrentCall ();
			bool SIPAcceptIncomingCall (SIPCall *Call);
			bool SIPRejectIncomingCall (SIPCall *Call);

			// Additionals
			void StartRingRing (bool Start);

		protected:

		private:
			typedef std::map<SIPCodecType, std::string> CodecMap;

			// Variables
			bool				_Ringtone;
			std::string			_RingtoneFilename;
			bool				_Initialized;
			string				_StatusTextOffline;
			string				_StatusTextOnline;
			string				_SettingFilename;

			std::string			_AbsolutePath;
			CodecMap			_Codecs;

			EpConfig			_EndPoint_Config;
			TransportConfig 	_Transport_Config;
			AccountConfig		_Account_Config;


			// C++
			Endpoint 			*_SIP_EndPoint;
			SIPAccount			*_SIP_Account;
			TransportId			_SIP_TransportID;

			volatile SIPStatus	_SIP_Status;

			bool InitCPlusPlus ();
			void AddRingtones ();

			// Helper
			char* CopyString (string String);
			char* GetSIPA (string User, string Domain);
			char* GetSIPB (string Domain);
			bool FileExist (const std::string& Name);

			// Events
			void ReceiveCallInternalStateEvent (const SIPCallInternalStateEvent *Event);
			void ReceiveCallAnswerEvent (const SIPCallAnswerEvent *Event);

		protected:
	};

} /* namespace SIP */

#endif /* SIPCONTROLLER_H_ */
