/*
 * 		EXSpeechController.cpp
 *
 *  	Created on: 15.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */


/* ------------------------------------------------------------------------------------
 * ------------------------------------ INCLUDES --------------------------------------
 * ------------------------------------------------------------------------------------ */

#include "../SpeechDetection/EXSpeechController.h"

#include <stdio.h>

#include <signal.h>
#include <setjmp.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>



namespace EXSpeech {


/* ------------------------------------------------------------------------------------
 * ------------------------------------ THREADING STRUCTS -----------------------------
 * ------------------------------------------------------------------------------------ */

struct ThreadContainer {
	EXSpeechController *Controller;
	ThreadWorkingType Type;

	ThreadContainer (EXSpeechController *Controller, ThreadWorkingType Type) : Controller(Controller), Type(Type) {};
};



/* ------------------------------------------------------------------------------------
 * ------------------------------------ CONSTRUCTION AND INITIALIZING -----------------
 * ------------------------------------------------------------------------------------ */

EXSpeechController::EXSpeechController() : _LogOutput(false), _CurrentEXSpeechType(EXSpeechTypes::ThreadWorkingTypeNone), _Initialized(EXSpeechRuntimeTypeNone), _ps(0), _config(0), _ad(0), _cont(0), pErrFile(0), _thread(0), _exit(false), _exitFunction(false) {
	// TODO Auto-generated constructor stub

    memset(&adbuf, 0, BUFFERSIZE);
    k = 0;
    ts = 0;
    rem = 0;
    uttid = 0;

	_LastDetection = string("");
	sem_init(&_sem_wait_lock, 0, 0);
	pthread_mutex_init(&_mutex_queue_access_wait, NULL);
	pthread_mutex_init(&_mutex_task_lock, NULL);
	pthread_mutex_init(&_mutex_error_lock, NULL);

	_Config.LogOutput 		= false;
	_Config.ErrorFilename 	= "";
	_Config.LogFilename		= "";
	_Config.ModelDir		= "/home/pi/applications/EXX2/recognition/models";
	_Config.Language		= EXSpeechLanguageGerman;
	_Config.Dictionary		= "dictionary_light.dic";
	_Config.AudioModel		= "voxforge.lm.DMP";
	_Config.SampelRate		= 16000;
	_Config.BestPath		= true;

	this->ExecuteThread();
}


/* ------------------------------------------------------------------------------------
 * ------------------------------------ DECONSTRUCTION AND UNINITIALIZING -------------
 * ------------------------------------------------------------------------------------ */

EXSpeechController::~EXSpeechController() {
	// TODO Auto-generated destructor stub
	this->UninitializeIntern();

	sem_destroy(&_sem_wait_lock);
	pthread_mutex_destroy(&_mutex_queue_access_wait);
	pthread_mutex_destroy(&_mutex_task_lock);
	pthread_mutex_destroy(&_mutex_error_lock);
}

bool EXSpeechController::UninitializeIntern () {

	SendEvent(new SpeechEvent(ThreadWorkingTypeUnInitialize));

	if (StopThread()) {
		if (_ps != 0) {
			ps_free(_ps);
			_ps = 0;
			_Initialized = EXSpeechRuntimeTypeNone;
			SendEvent(new SpeechEvent(ThreadWorkingTypeUnInitialized));
			return true;
		}
	}

	return false;
}


/* ------------------------------------------------------------------------------------
 * ------------------------------------ GENERAL METHODS -------------------------------
 * ------------------------------------------------------------------------------------ */

void EXSpeechController::SetAbsolutePath (std::string AbsolutePath) {
	_AbsolutePath = AbsolutePath;
}

bool EXSpeechController::InitializeEngine (EXSpeechConfig Config) {
	bool Success = false;

	if (_Initialized == EXSpeechRuntimeTypeNone) {
		this->ExecuteThread();
	}

	if (_Initialized == EXSpeechRuntimeTypeThreadRun) {

		MapConfig(Config);

		pthread_mutex_lock(&_mutex_task_lock);

		_LogOutput = _Config.LogOutput;
		_ErrorFilename = _Config.ErrorFilename;
		_LogFilename = _Config.LogFilename;

		AddToQueue(ThreadWorkingTypeInit);

		pthread_mutex_unlock(&_mutex_task_lock);
	}

	return Success;
}

bool EXSpeechController::UninitializeEngine () {
	return UninitializeIntern();
}

bool EXSpeechController::IsInitialized () {
	return (_Initialized > EXSpeechRuntimeTypeThreadRun ? true : false);
}

bool EXSpeechController::DetectionModeOn () {
	return (_CurrentEXSpeechType == ThreadWorkingTypeRecognize ? true : false);
}

bool EXSpeechController::StartDetection () {
	if (_Initialized > EXSpeechRuntimeTypeNone) {
		if (_CurrentEXSpeechType != EXSpeechTypes::ThreadWorkingTypeRecognize && this->IsInQueue(EXSpeechTypes::ThreadWorkingTypeRecognize) == false) {
			AddToQueue(EXSpeechTypes::ThreadWorkingTypeRecognize);
			return true;
		}
	}
	return false;
}

bool EXSpeechController::StopDetection () {
	if (_Initialized == EXSpeechRuntimeTypeInitialized) {
		DeleteAllFromQueue(EXSpeechTypes::ThreadWorkingTypeRecognize);
		if (_CurrentEXSpeechType == EXSpeechTypes::ThreadWorkingTypeRecognize) {
			this->StopCurrentFunction();
			return true;
		}
	}

	return false;
}

string EXSpeechController::LastDetection () {
	string Detection = "";
	if (_LastDetection.size() != 0) {
		pthread_mutex_lock(&_mutex_task_lock);
		Detection =  _LastDetection;
		pthread_mutex_unlock(&_mutex_task_lock);
	}

	return Detection;
}

string EXSpeechController::LastDetectionAndDelete () {
	string Detection = LastDetection();
	DeleteDetection();
	return Detection;
}

void EXSpeechController::DeleteDetection () {
	if (_LastDetection.size() != 0) {
		pthread_mutex_lock(&_mutex_task_lock);
		_LastDetection = string("");
		pthread_mutex_unlock(&_mutex_task_lock);
	}
}

string EXSpeechController::GetMessage () {
	return _ErrorMessage;
}


/* ------------------------------------------------------------------------------------
 * ------------------------------------ THREAD WORKER ---------------------------------
 * ------------------------------------------------------------------------------------ */

bool EXSpeechController::StopThread () {

	if (_thread != 0) {
		_exit = 1;

		sem_post(&_sem_wait_lock);

		void *status;
		int error = pthread_join(_thread, &status);

		if (error == 0) {
			_CurrentEXSpeechType = EXSpeechTypes::ThreadWorkingTypeNone;
			_thread = 0;
			_exit = 0;
			_Initialized = EXSpeechRuntimeTypeNone;
			return true;
		}
	}
	return false;
}

void EXSpeechController::StopCurrentFunction () {
	_exitFunction = 1;
}

bool EXSpeechController::ExecuteThread () {

	if (_thread == 0) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		int error = pthread_create(&_thread, NULL, &ThreadWorker, this);

		pthread_attr_destroy(&attr);

		if (error == 0) {
			_Initialized = EXSpeechRuntimeTypeThreadRun;
			return true;
		}
		_thread = 0;
	}

	return false;
}

void *EXSpeechController::ThreadWorker (void *self) {
	EXSpeechController *SpeechController = (EXSpeechController*)self;

	while (!SpeechController->_exit) {

		SpeechController->_exitFunction = 0;

		sem_wait(&SpeechController->_sem_wait_lock);

		ThreadWorkingType Type = SpeechController->PopFromQueue();

		if (Type == ThreadWorkingTypeRecognize) {
			if (SpeechController->IsInitialized() == true) {
				SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeStartRecognize));

				SpeechController->_CurrentEXSpeechType = ThreadWorkingTypeRecognize;

				bool Recognize = true;
				int FailCount = 0;

				while(Recognize) {

					if (SpeechController->StartAudioRecording()) {
						FailCount = 0;
						SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeRecognize));
						if ((Recognize = SpeechController->RecognizeFromMicrophone(SpeechController->adbuf, SpeechController->k, SpeechController->ts, SpeechController->rem, SpeechController->uttid)) == false) {
							SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeStopRecognize));
							SpeechController->StopAudioRecording();
							SpeechController->ClearBuffer();
						}
					} else {
						SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeRecognizeFailed));

						FailCount++;

						if (FailCount == 5) {
							Recognize = false;
						}
					}

				}

				SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeRecognizeStopped));

				//SpeechController->ClearBuffer();
				//SpeechController->StopAudioRecording();

			}
		} else if (Type == ThreadWorkingTypeInit) {
			if (SpeechController->IsInitialized() == false) {
				SpeechController->_CurrentEXSpeechType = ThreadWorkingTypeInit;
				bool Success = true;
				if ((Success &= SpeechController->InitializeSpeechEngine(SpeechController->_LogOutput, SpeechController->_ErrorFilename, SpeechController->_LogFilename))) {
					if ((Success &= SpeechController->PrepareListening())) {
						SpeechController->_Initialized = EXSpeechRuntimeTypeInitialized;

						if (SpeechController->StartAudioRecording()) {
							if (SpeechController->CalibateVoiceDetection()) {
								SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeCalibrated));
							}
							SpeechController->StopAudioRecording();
						}
					}
				}

				if (Success == true) {
					SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeInit));
				} else {
					SpeechController->SendEvent(new EXSpeechTypes::SpeechEvent(ThreadWorkingTypeInitFailed));
				}
			}
		} else if (Type == ThreadWorkingTypeUnInitialize) {
			if (SpeechController->IsInitialized() == true) {
				SpeechController->StopAudioRecording();
				SpeechController->_CurrentEXSpeechType = ThreadWorkingTypeUnInitialize;
				SpeechController->EndListening(SpeechController);
			}
		} else if (Type == ThreadWorkingTypeCalibrate) {
			if (SpeechController->IsInitialized() == true) {
				if (SpeechController->StartAudioRecording()) {
					SpeechController->CalibateVoiceDetection();
					SpeechController->StopAudioRecording();
				}
			}
		}

		SpeechController->_CurrentEXSpeechType = ThreadWorkingTypeNone;
	}

	SpeechController->_thread = 0;
	SpeechController->_CurrentEXSpeechType = ThreadWorkingTypeNone;
	pthread_exit(NULL);
}


/* ------------------------------------------------------------------------------------
 * ------------------------------------ SPEAK ENGINE ----------------------------------
 * ------------------------------------------------------------------------------------ */

bool EXSpeechController::InitializeSpeechEngine (bool LogOutput, string ErrorFilename, string LogFilename) {

		const char *log = NULL;

		if (LogOutput == true) {
			if (ErrorFilename.length() > 0) {
				pErrFile = fopen(ErrorFilename.c_str(), "r");
				err_set_logfp (pErrFile);
			}

			if (LogFilename.length() > 0) {
				log = LogFilename.c_str();
				err_set_logfile (log);
			}
		} else {
			err_set_debug_level(0);
			err_set_logfp (NULL);
			err_set_logfile (NULL);
		}

		string ModelDir = _Config.ModelDir;
		string HMM		= ModelDir;
		string DICT		= ModelDir;
		string LM		= ModelDir;
		string SAMPRATE	= to_string(_Config.SampelRate);

		HMM.append("/hmm/");
		DICT.append("/lm/");
		LM.append("/lm/");

		try {
			HMM.append(GetLanguage(_Config.Language));
			DICT.append(GetLanguage(_Config.Language));
			LM.append(GetLanguage(_Config.Language));
			DICT.append("/");
			LM.append("/");

			DICT.append(_Config.Dictionary);
			LM.append(_Config.AudioModel);

			_config = cmd_ln_init(0, ps_args(), 1,
					"-hmm", HMM.c_str(),
					"-lw", "10",
					"-feat", "1s_c_d_dd",
					"-beam", "1e-80",
					"-wbeam", "1e-40",
					"-dict", DICT.c_str(),
					"-lm", LM.c_str(),
					"-wip", "0.2",
					"-agc", "none",
					"-varnorm", "no",
					"-samprate", SAMPRATE.c_str(),
					"-cmn", "current",
					"-fwdflat", (_Config.FWDFlat == 1 ? "yes" : "no"),
					"-bestpath", (_Config.BestPath == 1 ? "yes" : "no"), NULL);

			if (_config == NULL) {
				return false;
			}

			_ps = ps_init(_config);
			if (_ps == NULL) {
				return false;
			}
		} catch (EXception &Error) {
			return false;
		}


		return true;
}

bool EXSpeechController::PrepareListening () {

    if ((_ad = ad_open_sps((int)cmd_ln_float32_r(_config, "-samprate"))) == NULL) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to open audio device");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    }

#define NORMAL_MODE

#ifdef NORMAL_MODE
    if ((_cont = cont_ad_init(_ad, ad_read)) == NULL) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to initialize voice activity detection");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    }

#else
    if ((_cont = cont_ad_init_rawmode(_ad, ad_read)) == NULL) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to initialize voice activity detection");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    } else {
    	//cont_ad_set_rawmode(_cont, 1);
    }
#endif

    err_set_debug_level(5);
	err_set_logfp (NULL);
	err_set_logfile (NULL);

    return true;
}

bool EXSpeechController::StartAudioRecording() {
    if (ad_start_rec(_ad) < 0) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to start recording");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    }

    return true;
}

bool EXSpeechController::CalibateVoiceDetection() {
    if (cont_ad_calib(_cont) < 0) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to calibrate voice activity detection");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    }

    return false;
}

bool EXSpeechController::StopAudioRecording() {
    if (ad_stop_rec(_ad) < 0) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to start recording");
    	pthread_mutex_unlock(&_mutex_error_lock);
        return false;
    }
    return true;
}

void EXSpeechController::ClearBuffer() {
	memset(&adbuf, 0, BUFFERSIZE);
}

bool EXSpeechController::RecognizeFromMicrophone (int16 adbuf[BUFFERSIZE], int32 k, int32 ts, int32 rem, char const * uttid) {

	if (_ps == 0 || _ad == 0) {
		return false;
	}

    while ((k = cont_ad_read(_cont, adbuf, BUFFERSIZE)) == 0) {
    	if (_exit || _exitFunction) {

    		pthread_mutex_lock(&_mutex_task_lock);
    		_LastDetection = "";
    		pthread_mutex_unlock(&_mutex_task_lock);

    		return false;
    	}
    	usleep(1000);
    }

    if (k < 0) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to read audio");
    	pthread_mutex_unlock(&_mutex_error_lock);
    	return false;
    }

    if (ps_start_utt(_ps, NULL) < 0) {
    	pthread_mutex_lock(&_mutex_error_lock);
    	_ErrorMessage = string("Failed to start utterance");
    	pthread_mutex_unlock(&_mutex_error_lock);
    	return false;
    }

    if (ps_process_raw(_ps, adbuf, k, 0, 0) > 0) {

    	ts = _cont->read_ts;

    	for (;;) {
    		if ((k = cont_ad_read(_cont, adbuf, BUFFERSIZE)) < 0) {
    			pthread_mutex_lock(&_mutex_error_lock);
    			_ErrorMessage = string("Failed to read audio");
    			pthread_mutex_unlock(&_mutex_error_lock);
    			return false;
    		}

    		if (k == 0) {
    			if ((_cont->read_ts - ts) > _Config.SampelRate) {
    				break;
    			}
    		} else {
    			ts = _cont->read_ts;
    		}

    		rem = ps_process_raw(_ps, adbuf, k, 0, 0);

    		if ((rem == 0) && (k == 0)) {
    			usleep(20000);
    		}
    	}

    	this->StopAudioRecording();
    	while (ad_read(_ad, adbuf, BUFFERSIZE) >= 0) {
    		cont_ad_reset(_cont);
    	}

    	ps_end_utt(_ps);

    	pthread_mutex_lock(&_mutex_task_lock);
    	const char *Det = ps_get_hyp(_ps, NULL, &uttid);
    	if (Det != 0) {
    		_LastDetection = string(Det);
    		if (_LastDetection.size() != 0) {
    			SendEvent(new EXSpeechTypes::SpeechRecognizeEvent(_LastDetection));
    		}
    	}
    	pthread_mutex_unlock(&_mutex_task_lock);

		return true;
    }

    return false;
}

void EXSpeechController::EndListening (EXSpeechController *SpeechController) {
    cont_ad_close(SpeechController->_cont);
    ad_close(SpeechController->_ad);
}

void EXSpeechController::MapConfig (EXSpeechConfig Config) {
	std::string AbsolutePath = _AbsolutePath;
	_Config.LogOutput 		= (Config.LogOutput != -1 ? Config.LogOutput : _Config.LogOutput);
	_Config.ErrorFilename 	= AbsolutePath.append((Config.ErrorFilename.size() > 0 ? Config.ErrorFilename : _Config.ErrorFilename));
	_Config.LogFilename		= AbsolutePath.append((Config.LogFilename.size() > 0 ? Config.LogFilename : _Config.LogFilename));
	_Config.ModelDir		= AbsolutePath.append((Config.ModelDir.size() > 0 ? Config.ModelDir : _Config.ModelDir));
	_Config.Language		= (Config.Language != EXSpeechLanguageNone ? Config.Language : _Config.Language);
	_Config.Dictionary		= (Config.Dictionary.size() > 0 ? Config.Dictionary : _Config.Dictionary);
	_Config.AudioModel		= (Config.AudioModel.size() > 0 ? Config.AudioModel : _Config.AudioModel);
	_Config.SampelRate		= (Config.SampelRate != -1 ? Config.SampelRate : _Config.SampelRate);
	_Config.BestPath		= (Config.BestPath != -1 ? Config.BestPath : _Config.BestPath);
}

string EXSpeechController::GetLanguage (EXSpeechLanguage Language) throw() {
	switch ((int)Language) {
		case EXSpeechLanguageEnglish:
			return "en_EN";
		case EXSpeechLanguageGerman:
			return "de_DE";
	}

	throw(EXception("Not Found"));

	return string();
}


/* ------------------------------------------------------------------------------------
 * ------------------------------------ QUEUE -----------------------------------------
 * ------------------------------------------------------------------------------------ */

bool EXSpeechController::AddToQueue (ThreadWorkingType Type, void *Data) {

	pthread_mutex_lock(&_mutex_queue_access_wait);
	_needUpdateQueue.insert(_needUpdateQueue.begin(), Type);
	pthread_mutex_unlock(&_mutex_queue_access_wait);

	sem_post(&_sem_wait_lock);

	return true;
}

ThreadWorkingType EXSpeechController::PopFromQueue () {

	ThreadWorkingType Container = ThreadWorkingTypeNone;

	pthread_mutex_lock(&_mutex_queue_access_wait);
	if (_needUpdateQueue.size() > 0) {
		//int Size = _needUpdateQueue.size();
		Container = _needUpdateQueue.back();
		_needUpdateQueue.pop_back();
	}
	//int Size = _needUpdateQueue.size();
	pthread_mutex_unlock(&_mutex_queue_access_wait);

	return Container;
}

void EXSpeechController::DeleteAllFromQueue (ThreadWorkingType Type, bool Locked) {
	if (Locked == true) {
		pthread_mutex_lock(&_mutex_queue_access_wait);
	}

	vector<ThreadWorkingType>::iterator it = _needUpdateQueue.end();

	while (it != _needUpdateQueue.begin()) {
		if ((*it) == Type) {
			_needUpdateQueue.erase(it);
		}
		it--;
	}

	if (Locked == true) {
		pthread_mutex_unlock(&_mutex_queue_access_wait);
	}
}

bool EXSpeechController::IsInQueue (ThreadWorkingType Type, bool Locked) {
	if (Locked == true) {
		pthread_mutex_lock(&_mutex_queue_access_wait);
	}

	vector<ThreadWorkingType>::iterator it = _needUpdateQueue.end();
	bool Found = false;

	while (it != _needUpdateQueue.begin()) {
		if ((*it) == Type) {
			Found = true;
			break;
		}
		it--;
	}

	if (Locked == true) {
		pthread_mutex_unlock(&_mutex_queue_access_wait);
	}

	return Found;
}

bool EXSpeechController::QueueEmpty () {
	pthread_mutex_lock(&_mutex_queue_access_wait);
	bool Return (_needUpdateQueue.size() > 0 ? false : true);
	pthread_mutex_unlock(&_mutex_queue_access_wait);
	return Return;
}

} /* namespace EXSpeech */
