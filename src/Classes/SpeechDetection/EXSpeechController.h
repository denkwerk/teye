/*
 * 		EXSpeechController.h
 *
 *  	Created on: 15.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 *
 *		Speech Recognition Class based on the PJSIP Engine.
 */

#ifndef EXSPEECHCONTROLLER_H_
#define EXSPEECHCONTROLLER_H_

#include <string>
#include <vector>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include <pocketsphinx/pocketsphinx.h>

#include <pthread.h>
#include <semaphore.h>

#include "../Event/EventHandler.h"

using namespace std;

namespace EXSpeech {


/* ------------------------------------------------------------------------------------
 * ------------------------------------ DEFINES ---------------------------------------
 * ------------------------------------------------------------------------------------ */

#define BUFFERSIZE 4096
//#define BUFFERSIZE 192000


/* ------------------------------------------------------------------------------------
 * ------------------------------------ ENUMERATOR TYPES ------------------------------
 * ------------------------------------------------------------------------------------ */

	namespace EXSpeechTypes {
		enum ThreadWorkingType {
			ThreadWorkingTypeInitFailed = -1,
			ThreadWorkingTypeNone = 0,
			ThreadWorkingTypeInit,
			ThreadWorkingTypeStartRecognize,
			ThreadWorkingTypeRecognize,
			ThreadWorkingTypeRecognizeFailed,
			ThreadWorkingTypeStopRecognize,
			ThreadWorkingTypeRecognizeStopped,
			ThreadWorkingTypeUnInitialize,
			ThreadWorkingTypeUnInitialized,
			ThreadWorkingTypeCalibrate,
			ThreadWorkingTypeCalibrated,
		};

		enum EXSpeechLanguage {
			EXSpeechLanguageNone = -1,
			EXSpeechLanguageEnglish,
			EXSpeechLanguageGerman,
		};

		enum EXSpeechRuntimeType {
			EXSpeechRuntimeTypeNone = -1,
			EXSpeechRuntimeTypeThreadRun,
			EXSpeechRuntimeTypeInitialized,
			EXSpeechRuntimeTypeUninitialized
		};


/* ------------------------------------------------------------------------------------
 * ------------------------------------ EVENT CLASSES ---------------------------------
 * ------------------------------------------------------------------------------------ */

		class SpeechEvent : public Event {
			public:
				SpeechEvent(ThreadWorkingType Type) : Type(Type) {};

				ThreadWorkingType Type;
		};

		class SpeechRecognizeEvent : public SpeechEvent {
			public:
				SpeechRecognizeEvent(string DetectedString) : SpeechEvent(ThreadWorkingTypeRecognize), DetectedString(DetectedString) {};

				string DetectedString;
		};


/* ------------------------------------------------------------------------------------
 * ------------------------------------ SPEAK CONFIGURATION STRUCTS -------------------
 * ------------------------------------------------------------------------------------ */

		struct EXSpeechConfig {
			int					LogOutput;
			string				ErrorFilename;
			string				LogFilename;
			string				ModelDir;
			EXSpeechLanguage	Language;
			string				Dictionary;
			string				AudioModel;
			int					SampelRate;
			int					BestPath;
			int					FWDFlat;

			EXSpeechConfig () : LogOutput(-1), Language(EXSpeechLanguageNone), SampelRate(-1), BestPath(1), FWDFlat(1) {};
		};

		struct EXception : public std::exception {
		   string		s;
		   EXception(std::string ss) : s(ss) {}
		   ~EXception() throw () {} // Updated
		   const char* what() const throw() { return s.c_str(); }
		};
	}



	using namespace EXSpeechTypes;


/* ------------------------------------------------------------------------------------
 * ------------------------------------ EXSpeechController CLASS -----------------------
 * ------------------------------------------------------------------------------------ */

	class EXSpeechController : public EventHandler {
		public:


		// Methods


			// Constructor

								EXSpeechController();
			virtual				~EXSpeechController();


			// Speak Engine

			void				SetAbsolutePath					(std::string AbsolutePath);
			bool				InitializeEngine				(EXSpeechConfig Config);
			bool				UninitializeEngine				();
			bool				IsInitialized					();


			// Testing

			bool				Test							();
			bool				DetectionModeOn					();
			bool				StartDetection					();
			bool				StopDetection					();


			// Results

			string				LastDetection					();
			string				LastDetectionAndDelete			();
			void				DeleteDetection					();
			string 				GetMessage						();

		private:


		// Members


			// General

			string							_ErrorMessage;
			bool							_LogOutput;
			string							_ErrorFilename;
			string							_LogFilename;
			ThreadWorkingType				_CurrentEXSpeechType;
			EXSpeechConfig					_Config;
			std::string						_AbsolutePath;


			// Recognition

			volatile EXSpeechRuntimeType	_Initialized;
			ps_decoder_t					*_ps;
			cmd_ln_t						*_config;
			ad_rec_t						*_ad;
		    cont_ad_t						*_cont;


		    // Buffers

		    int16							adbuf[BUFFERSIZE];
		    int32							k;
		    int32							ts;
		    int32							rem;
		    char const						*uttid;


			// Detection

			string 							_LastDetection;


			// Logging

			FILE * pErrFile;


			// Threads

			pthread_t						_thread;
			sem_t							_sem_wait_lock;
			pthread_mutex_t					_mutex_task_lock;
			pthread_mutex_t					_mutex_error_lock;
			pthread_mutex_t					_mutex_queue_access_wait;
			volatile int					_exit;
			volatile int					_exitFunction;
			vector<ThreadWorkingType>		_needUpdateQueue;


		// Methods


			// General

			bool				UninitializeIntern ();
			bool				InitializeSpeechEngine			(bool LogOutput = false, string ErrorFilename = "", string LogFilename = "");
			bool				PrepareListening				();
			bool				StartAudioRecording				();
			bool				CalibateVoiceDetection			();
			bool				StopAudioRecording				();
			void				ClearBuffer						();
			void				CreateBuffers					(int16 &Buffer);
			bool				RecognizeFromMicrophone			(int16 adbuf[BUFFERSIZE], int32 k, int32 ts, int32 rem, char const * uttid);
			void				EndListening					(EXSpeechController *SpeechController);
			void				MapConfig						(EXSpeechConfig Config);
			string				GetLanguage						(EXSpeechLanguage Language) throw();


			// Internal Worker Thread

			bool				StopThread						();
			void				StopCurrentFunction				();
			bool				ExecuteThread					();
			static void*		ThreadWorker					(void *self);


			// Threading Queue

			bool				AddToQueue						(ThreadWorkingType Type, void *Data = 0);
			ThreadWorkingType	PopFromQueue					();
			void 				DeleteAllFromQueue				(ThreadWorkingType Type, bool Locked = true);
			bool				IsInQueue						(ThreadWorkingType Type, bool Locked = true);
			bool				QueueEmpty						();

	};

} /* namespace EXSpeech */

#endif /* EXSPEECHCONTROLLER_H_ */
