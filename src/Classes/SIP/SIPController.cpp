/*
 * 		SIPController.cpp
 *
 *  	Created on: 14.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "SIPController.h"

using namespace SIP::Internal;

namespace SIP {

#define THIS_FILE "APP"


// -----------------------------------------------------------------------------------------------
// ServoSettings ---------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void ServoSettings::readObject(const ContainerNode &node) throw(Error) {

	ContainerNode this_node = node.readContainer(string("ServoConfig_").append(Suffix));

	NODE_READ_INT (this_node, _Calibration_Left);
	NODE_READ_INT (this_node, _Calibration_Top);
	NODE_READ_INT (this_node, _Calibration_Right);
}

void ServoSettings::writeObject(ContainerNode &node) const throw(Error) {

	ContainerNode this_node = node.writeNewContainer(string("ServoConfig_").append(Suffix));

	NODE_WRITE_INT(this_node, _Calibration_Left);
	NODE_WRITE_INT(this_node, _Calibration_Top);
	NODE_WRITE_INT(this_node, _Calibration_Right);
}

void ApplicationSettings::readObject(const ContainerNode &node) throw(Error) {
	_Motion_Calibration_Min = node.readInt("_Motion_Calibration_Min");
	_Motion_Calibration_Max = node.readInt("_Motion_Calibration_Max");
	_Motion_Calibration_Threshold = node.readInt("_Motion_Calibration_Threshold");
	_Motion_Calibration_Timeout = node.readInt("_Motion_Calibration_Timeout");
	_Motion_Calibration_CalibrationOffset = node.readInt("_Motion_Calibration_CalibrationOffset");
	_Motion_Port_CLK = node.readInt("_Motion_Port_CLK");
	_Motion_Port_DOut = node.readInt("_Motion_Port_DOut");
	_Motion_Port_DIn = node.readInt("_Motion_Port_DIn");
	_Motion_Port_CS = node.readInt("_Motion_Port_CS");
	_Servo_Port_A = node.readInt("_Servo_Port_A");
	_Servo_Port_B = node.readInt("_Servo_Port_B");
	node.readObject(_Servo_Calibration_A);
	node.readObject(_Servo_Calibration_B);
	_Speak_LogOutput = node.readInt("_Speak_LogOutput");
	_Speak_ErrorFilename = node.readString("_Speak_ErrorFilename");
	_Speak_LogFilename = node.readString("_Speak_LogFilename");
	_Speak_ModelDir = node.readString("_Speak_ModelDir");
	_Speak_Language = node.readInt("_Speak_Language");
	_Speak_Dictionary = node.readString("_Speak_Dictionary");
	_Speak_AudioModel = node.readString("_Speak_AudioModel");
	_Speak_SampelRate = node.readInt("_Speak_SampelRate");
	_Speak_BestPath = node.readInt("_Speak_BestPath");
	try {
		_SIP_Config_Codec_Type  = node.readInt("_SIP_Config_Codec_Type");
	} catch (Error &error) {
		_SIP_Config_Codec_Type = 2;
		Changes = true;
	}
	try {
		_SIP_Config_Ringtone  = node.readInt("_SIP_Config_Ringtone");
	} catch (Error &error) {
		_SIP_Config_Ringtone = 1;
		Changes = true;
	}
	try {
		_SIP_Config_Ringtone_File  = node.readString("_SIP_Config_Ringtone_File");
	} catch (Error &error) {
		_SIP_Config_Ringtone_File = "duckhunt.wav";
		Changes = true;
	}
	try {
		_SIP_Config_Receiver  = node.readString("_SIP_Config_Receiver");
	} catch (Error &error) {
		_SIP_Config_Receiver = "2000 <sip:2000@10.0.66.160>";
		Changes = true;
	}

	node.readObject(_SIP_Config_Endpoint);
	node.readObject(_SIP_Config_Transport);
	node.readObject(_SIP_Config_Account);
}

void ApplicationSettings::writeObject(ContainerNode &node) const throw(Error) {
	node.writeInt("_Motion_Calibration_Min", _Motion_Calibration_Min);
	node.writeInt("_Motion_Calibration_Max", _Motion_Calibration_Max);
	node.writeInt("_Motion_Calibration_Threshold", _Motion_Calibration_Threshold);
	node.writeInt("_Motion_Calibration_Timeout", _Motion_Calibration_Timeout);
	node.writeInt("_Motion_Calibration_CalibrationOffset", _Motion_Calibration_CalibrationOffset);
	node.writeInt("_Motion_Port_CLK", _Motion_Port_CLK);
	node.writeInt("_Motion_Port_DOut", _Motion_Port_DOut);
	node.writeInt("_Motion_Port_DIn", _Motion_Port_DIn);
	node.writeInt("_Motion_Port_CS", _Motion_Port_CS);
	node.writeInt("_Servo_Port_A", _Servo_Port_A);
	node.writeInt("_Servo_Port_B", _Servo_Port_B);
	node.writeObject(_Servo_Calibration_A);
	node.writeObject(_Servo_Calibration_B);
	node.writeInt("_Speak_LogOutput", _Speak_LogOutput);
	node.writeString("_Speak_ErrorFilename", _Speak_ErrorFilename);
	node.writeString("_Speak_LogFilename", _Speak_LogFilename);
	node.writeString("_Speak_ModelDir", _Speak_ModelDir);
	node.writeInt("_Speak_Language", _Speak_Language);
	node.writeString("_Speak_Dictionary", _Speak_Dictionary);
	node.writeString("_Speak_AudioModel", _Speak_AudioModel);
	node.writeInt("_Speak_SampelRate", _Speak_SampelRate);
	node.writeInt("_Speak_BestPath", _Speak_BestPath);
	node.writeInt("_SIP_Config_Codec_Type", _SIP_Config_Codec_Type);
	node.writeInt("_SIP_Config_Ringtone", _SIP_Config_Ringtone);
	node.writeString("_SIP_Config_Ringtone_File", _SIP_Config_Ringtone_File);
	node.writeString("_SIP_Config_Receiver", _SIP_Config_Receiver);
	node.writeObject(_SIP_Config_Endpoint);
	node.writeObject(_SIP_Config_Transport);
	node.writeObject(_SIP_Config_Account);
}


// -----------------------------------------------------------------------------------------------
// SIPCall ---------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

SIPCall::SIPCall(Account& acc, int call_id, void *_SIPAccount) : Call(acc, call_id), _SIPAccount(_SIPAccount) {

}

void SIPCall::onCallState(OnCallStateParam &prm) {
	CallInfo ci = getInfo();

	bool DeleteMe = false;

	SIPCallStateType Type = SIPCallStateTypeNone;

	if (ci.state == PJSIP_INV_STATE_CONNECTING) {
		Type = SIPCallStateTypeConnecting;
	} else if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
		Type = SIPCallStateTypeConnected;
	} else if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
		Type = SIPCallStateTypeDisconnected;
		DeleteMe = true;
	} else if (ci.state == PJSIP_INV_STATE_CALLING) {
		Type = SIPCallStateTypeCalling;
	} else if (ci.state == PJSIP_INV_STATE_INCOMING) {
	} else if (ci.state == PJSIP_INV_STATE_EARLY) {

	}

	this->SendEvent(new SIPCallStateEvent(SIPCallStateCall, Type));
	//this->SendEvent(new SIPCallInternalStateEvent(SIPCallStateCall, Type));

	if (DeleteMe == true) {
		if (_SIPAccount != 0) {
			SIPAccount *Account = (SIPAccount*)_SIPAccount;
			Account->ReleaseCall();
		}
	}

}

void SIPCall::onCallMediaState(OnCallMediaStateParam &prm) {

	CallInfo ci = getInfo();

	AudioMedia *aud_med = NULL;

	// Find out which media index is the audio
	for (unsigned i=0; i<ci.media.size(); ++i) {
		if (ci.media[i].type == PJMEDIA_TYPE_AUDIO) {
			aud_med = (AudioMedia *)this->getMedia(i);
			break;
		}
	}

	if (aud_med) {
		AudDevManager &AudioManager = Endpoint::instance().audDevManager();

		// This will connect the sound device/mic to the call audio media
		AudioMedia& cap_med = AudioManager.getCaptureDevMedia();
		cap_med.startTransmit(*aud_med);

		// And this will connect the call audio media to the sound device/speaker
		aud_med->startTransmit(AudioManager.getPlaybackDevMedia());
	}

	this->SendEvent(new SIPCallStateEvent(SIPCallStateCallMedia, SIPCallStateTypeNone, this));
}


// -----------------------------------------------------------------------------------------------
// SIPAccount ------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void SIPAccount::onRegState(OnRegStateParam &prm) {
	AccountInfo ai = getInfo();

	this->SendEvent(new SIPCallEvent(SIPAccountStateIncomingReg));
}

void SIPAccount::onIncomingCall(OnIncomingCallParam &iprm) {
	if (this->Connected()) {
		SIPCall *Call = new SIPCall(*this, iprm.callId, (void*)this);

		CallOpParam prm;
		prm.statusCode = PJSIP_SC_BUSY_HERE;
		Call->answer(prm);
	} else {
		_SIP_CurrentCall = new SIPCall(*this, iprm.callId, (void*)this);
		if (_SIP_CurrentCall != 0) {

			this->SendEvent(new SIPCallEvent(SIPAccountStateIncomingCall, _SIP_CurrentCall->getInfo().remoteUri, _SIP_CurrentCall));
		}
	}

}

void SIPAccount::onRegStarted(OnRegStartedParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateRegStarted));
}

void SIPAccount::onIncomingSubscribe(OnIncomingSubscribeParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateIncomingSubscribe));
}

void SIPAccount::onInstantMessage(OnInstantMessageParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateInstantMessage));
}

void SIPAccount::onInstantMessageStatus(OnInstantMessageStatusParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateInstantMessageStatus));
}

void SIPAccount::onTypingIndication(OnTypingIndicationParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateTypingIndicator));
}

void SIPAccount::onMwiInfo(OnMwiInfoParam &prm) {
	this->SendEvent(new SIPCallEvent(SIPAccountStateMWIInfo));
}

bool SIPAccount::CheckCall () {
	if (_SIP_CurrentCall == 0) {
		return true;
	}
	return !_SIP_CurrentCall->isActive();
}

bool SIPAccount::Connected () {
	if (_SIP_CurrentCall != 0) {
		if (_SIP_CurrentCall->isActive()) {
			return true;
		}
		return (_SIP_CurrentCall->getInfo().state == PJSIP_INV_STATE_CONFIRMED ? true : false);
	}
	return false;
}

bool SIPAccount::Call(std::string Receiver) {
	_SIP_CurrentCall = new SIPCall(*this, PJSUA_INVALID_ID, this);
	CallOpParam prm(true); // Use default call settings
	try {
		_SIP_CurrentCall->makeCall(Receiver, prm);
		return true;
	} catch(Error& err) {
		cout << err.info() << "\n" << endl;
	}
	return false;
}

bool SIPAccount::HangUp() {

	CallOpParam prm(true);

	try {
		_SIP_CurrentCall->hangup(prm);
		return true;
	} catch (Error &Err) {

	}

	return false;
}

SIPCall* SIPAccount::GetCall() {
	if (_SIP_CurrentCall != 0) {
		return _SIP_CurrentCall;
	}
	return 0;
}

void SIPAccount::StartRingRing (bool Start) {
	AudioMedia& MediaPlayer = Endpoint::instance().audDevManager().getPlaybackDevMedia();

	if (_AudioIncomingTone != 0) {
		if (Start == true) {
			try {
				_AudioIncomingTone->startTransmit(MediaPlayer);
			} catch(Error& err) {

			}
		} else {
			try {
				_AudioIncomingTone->stopTransmit(MediaPlayer);
				_AudioIncomingTone->setPos(0);
			} catch(Error& err) {

			}
		}
	}
}

void SIPAccount::ReleaseCall() {
	delete _SIP_CurrentCall;
	_SIP_CurrentCall = 0;
}


// -----------------------------------------------------------------------------------------------
// SIPController ---------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

SIPController::SIPController() : _CurrentCodecType(SIPCodecTypeSpeex8000), _Ringtone(true), _Initialized(false), _SIP_EndPoint(0), _SIP_Account(0), _SIP_TransportID(-1), _SIP_Status(SIPStatusNone) {
	// TODO Auto-generated constructor stub

	_Codecs[SIPCodecTypeNone]				= "";
	_Codecs[SIPCodecTypePCMU8000]			= "PCMU/8000/1";
	_Codecs[SIPCodecTypePCMA8000]			= "PCMA/8000/1";
	_Codecs[SIPCodecTypeSpeex8000]			= "speex/8000/1";
	_Codecs[SIPCodecTypeSpeex16000]			= "speex/16000/1";
	_Codecs[SIPCodecTypeSpeex32000]			= "speex/32000/1";
	_Codecs[SIPCodecTypeiLBC8000]			= "iLBC/8000/1";
	_Codecs[SIPCodecTypeGSM8000]			= "GSM/8000/1";
	_Codecs[SIPCodecTypeG72216000]			= "G722/16000/1";
	_Codecs[SIPCodecTypeL1680001]			= "L16/8000/1";
	_Codecs[SIPCodecTypeL1680002]			= "L16/8000/2";
	_Codecs[SIPCodecTypeL16160001]			= "L16/16000/1";
	_Codecs[SIPCodecTypeL16160002]			= "L16/16000/2";
	_Codecs[SIPCodecTypeL16441001]			= "L16/44100/1";
	_Codecs[SIPCodecTypeL16441002]			= "L16/44100/2";

	_StatusTextOffline		= string("AWAY");
	_StatusTextOnline		= string("HELLO");
}

SIPController::~SIPController() {
	// TODO Auto-generated destructor stub

	if (_Initialized == true) {
		UninitializeSIP();
	}

}


// Initializing

void SIPController::SetAbsolutePath (std::string AbsolutePath) {
	_AbsolutePath = AbsolutePath;
}

bool SIPController::InitializeSIP (ApplicationSettings &Settings) {
	if (_SIP_EndPoint == 0) {
		if (_SIP_Status == SIPStatusNone || _SIP_Status == SIPStatusStop) {
			_SIP_Status = SIPStatusStarting;
			SendEvent(new SIPEvent(SIPStatusStarting));

			_EndPoint_Config = Settings._SIP_Config_Endpoint;
			_Transport_Config = Settings._SIP_Config_Transport;
			_Account_Config = Settings._SIP_Config_Account;
			_CurrentCodecType = (SIPCodecType)Settings._SIP_Config_Codec_Type;
			_Ringtone = (bool)Settings._SIP_Config_Ringtone;
			_RingtoneFilename = Settings._SIP_Config_Ringtone_File;

			if (InitCPlusPlus() == false) {
				_SIP_Status = SIPStatusNone;
				SendEvent(new SIPEvent(SIPStatusError));
				return false;
			}

			const CodecInfoVector &Codecs = _SIP_EndPoint->codecEnum();
			CodecInfoVector::const_iterator it = Codecs.begin();
			if (_CurrentCodecType != SIPCodecTypeNone) {
				while (it != Codecs.end()) {
					if ((*it)->codecId == _Codecs[_CurrentCodecType]) {
						_SIP_EndPoint->codecSetPriority((*it)->codecId, 255);
					} else {
						_SIP_EndPoint->codecSetPriority((*it)->codecId, 0);
					}
					string description = (*it)->codecId;
					it++;
				}
			}


			RegisterEventFunc(this, &SIPController::ReceiveCallAnswerEvent);
			RegisterEventFunc(this, &SIPController::ReceiveCallInternalStateEvent);

			return true;
		}
	}

	return false;
}

bool SIPController::InitializeSIP (string SIPURL, string SIPREGISTRAR, string SIPREALM, string SIPUSER, string SIPPASSWD) {
	if (_SIP_EndPoint == 0) {
		if (_SIP_Status == SIPStatusNone || _SIP_Status == SIPStatusStop) {
			_SIP_Status = SIPStatusStarting;
			SendEvent(new SIPEvent(SIPStatusStarting));

			_Transport_Config.port = 0;

			_Account_Config.idUri = SIPURL;
			_Account_Config.regConfig.registrarUri = SIPREGISTRAR;
			AuthCredInfo cred("digest", SIPREALM, SIPUSER, 0, SIPPASSWD);
			_Account_Config.sipConfig.authCreds.push_back(cred);


			if (InitCPlusPlus() == false) {
				_SIP_Status = SIPStatusNone;
				SendEvent(new SIPEvent(SIPStatusError));
				return false;
			}
		}
	}

	return false;
}

bool SIPController::InitCPlusPlus () {

	_SIP_EndPoint = new Endpoint();

	try {
		_SIP_EndPoint->libCreate();
	} catch (Error &Err) {
		UninitializeSIP();
		return false;
	}

	if (_SIP_EndPoint->libGetState() != PJSUA_STATE_NULL) {

		_SIP_EndPoint->libInit(_EndPoint_Config);

		if (_SIP_EndPoint->libGetState() != PJSUA_STATE_NULL) {
			// Create SIP transport. Error handling sample is shown

			try {
				_SIP_TransportID = _SIP_EndPoint->transportCreate(PJSIP_TRANSPORT_UDP, _Transport_Config);
			} catch (Error &Err) {
				UninitializeSIP();
				return false;
			}

			// Start the library (worker threads etc)
			try {
				_SIP_EndPoint->libStart();

				// Initialize Audio Device
				AudDevManager &AudioManager = _SIP_EndPoint->audDevManager();
				//int CaptureDevice = AudioManager.getCaptureDev();
				//int PlaybackDevice = AudioManager.getPlaybackDev();
				std::vector<AudioDevInfo*> AudioDeviceList = AudioManager.enumDev();
				std::vector<AudioDevInfo*>::iterator AudioDeviceIterator = AudioDeviceList.begin();

				int ItDevice = 0;
				while (AudioDeviceIterator != AudioDeviceList.end()) {
					AudioDevInfo *AudioInfo = *AudioDeviceIterator;
					if (AudioInfo->name == "default:CARD=Device") {
						try {
							//AudioManager.setPlaybackDev(AudioManager.lookupDev(AudioInfo->driver, AudioInfo->name));
						} catch (Error &Err) {

						}
					}
					if (AudioInfo->name == "default:CARD=Device") {
						try {
							//AudioManager.setCaptureDev(AudioManager.lookupDev(AudioInfo->driver, AudioInfo->name));
						} catch (Error &Err) {

						}
					}
					AudioDeviceIterator++;
					ItDevice++;
				}

				//CaptureDevice = AudioManager.getCaptureDev();
				//PlaybackDevice = AudioManager.getPlaybackDev();

				AudioManager.setInputVolume(100, true);
				AudioManager.setOutputVolume(100, true);

				AccountConfig a_cfg;
				a_cfg.idUri = _Account_Config.idUri;
				a_cfg.regConfig.timeoutSec = _Account_Config.regConfig.timeoutSec;
				a_cfg.regConfig.registrarUri = _Account_Config.regConfig.registrarUri;
				a_cfg.sipConfig.authCreds.push_back(_Account_Config.sipConfig.authCreds.back());


				// Create the account
				_SIP_Account = new SIPAccount();
				_SIP_Account->create(a_cfg, true);

				this->AddRingtones();

				_Initialized = true;
				_SIP_Status = SIPStatusStarted;

			} catch (Error &Err) {
				UninitializeSIP();
				return false;
			}
		}
	}

	SendEvent(new SIPEvent(SIPStatusStarted));
	return true;
}

void SIPController::AddRingtones () {

	if (_RingtoneFilename.length() > 0) {
		std::string AbsolutePath = _AbsolutePath;
		std::string Ringtone = AbsolutePath.append("data/").append(_RingtoneFilename);
		if (this->FileExist(Ringtone) && _Ringtone == true) {
			AudioMediaPlayer *AudioIncomingTone = new AudioMediaPlayer();
			AudioIncomingTone->createPlayer(Ringtone);

			_SIP_Account->SetAudioPlayer(AudioIncomingTone, SIPAccount::AudioTypeIncomingCall);
		}
	}
}


bool SIPController::LoadApplicationSettings (ApplicationSettings &Settings) {
	JsonDocument jDoc;
	try {
		std::string AbsolutePath = _AbsolutePath;
		jDoc.loadFile(AbsolutePath.append(_SettingFilename));
		jDoc.readObject(Settings);
		return true;
	} catch (Error &Err) {

	}
	return false;
}

bool SIPController::SaveApplicationSettings (ApplicationSettings Settings) {
	JsonDocument jDoc;
	try {
		std::string AbsolutePath = _AbsolutePath;
		jDoc.writeObject(Settings);
		jDoc.saveFile(AbsolutePath.append(_SettingFilename));
		return true;
	} catch (Error &Err) {

	}
	return false;
}

void SIPController::SetFilename (string Filename) {
	_SettingFilename = Filename;
	_SettingFilename.append(".js");
}



bool SIPController::SetAvailable () {
	if (_SIP_Account != 0) {
		PresenceStatus Status;
		Status.status = PJSUA_BUDDY_STATUS_ONLINE;
		Status.statusText = _StatusTextOnline;
		Status.activity = PJRPID_ACTIVITY_UNKNOWN;
		try {
			_SIP_Account->setOnlineStatus(Status);
			SendEvent(new SIPStateEvent(SIPStateAvailable));
			return true;
		} catch (Error &Err) {

		}
	}

	return false;
}

bool SIPController::SetNotAvailable () {
	if (_SIP_Account != 0) {
		PresenceStatus Status;
		Status.status = PJSUA_BUDDY_STATUS_OFFLINE;
		Status.statusText = _StatusTextOffline;
		Status.activity = PJRPID_ACTIVITY_AWAY;
		try {
			_SIP_Account->setOnlineStatus(Status);
			SendEvent(new SIPStateEvent(SIPStateNotAvailable));
			return true;
		} catch (Error &Err) {

		}
	}

	return false;
}

bool SIPController::UninitializeSIP () {

	_SIP_EndPoint->hangupAllCalls();

	if (_SIP_Account != 0) {
		if (_SIP_Account->Connected()) {
			_SIP_Account->HangUp();
		}
		this->SetNotAvailable();
		delete _SIP_Account;
		_SIP_Account = 0;
	}

	try {
		_SIP_EndPoint->libDestroy();
		_SIP_EndPoint->libStopWorkerThreads();
		/*delete _SIP_EndPoint;*/
		_SIP_EndPoint = 0;
	} catch (Error &Err) {
		printf("Error Uninitializing SIP");
	}

	if (_SIP_EndPoint == 0) {
		_Initialized = false;
		return true;
	}

	return false;
}

bool SIPController::IsInitialized () {
	return _Initialized;
}


// Settings
std::string SIPController::GetCodec(SIPController::SIPCodecType Type) {
	if (Type != SIPCodecTypeNone) {
		return _Codecs[Type];
	}
	return _Codecs[_CurrentCodecType];
}


// SIP Calling

bool SIPController::SIPCallReady () {
	return (_SIP_Account != 0 && _SIP_Account->CheckCall());
}

bool SIPController::SIPCallConnected () {
	return _SIP_Account->Connected();
}

bool SIPController::SIPCallToNumber (string Receiver) {

	if (_Initialized == true && (_SIP_Account != 0 && _SIP_Account->CheckCall())) {
		return _SIP_Account->Call(Receiver);
	}

	return false;
}

bool SIPController::SIPHangUpCall () {
	if (_Initialized == true && _SIP_Account->Connected()) {
		return _SIP_Account->HangUp();
	}
	return false;
}

SIPCall* SIPController::SIPCurrentCall () {
	if (_Initialized == true && _SIP_Account->Connected()) {
		return _SIP_Account->GetCall();
	}
	return 0;
}

bool SIPController::SIPAcceptIncomingCall (SIPCall *Call) {
	if (Call != 0) {
		CallOpParam prm;
		prm.statusCode = PJSIP_SC_OK;
		Call->answer(prm);
		return true;
	}
	return false;
}

bool SIPController::SIPRejectIncomingCall (SIPCall *Call) {
	if (Call != 0) {
		CallOpParam prm;
		prm.statusCode = PJSIP_SC_BUSY_HERE;
		Call->answer(prm);
		return true;
	}
	return false;
}


// Additionals

void SIPController::StartRingRing (bool Start) {
	if (_SIP_Account != 0) {
		_SIP_Account->StartRingRing(Start);
	}
}


// Helper

char* SIPController::CopyString (string String) {
	char *cstr = new char[String.length() + 1];
	strcpy(cstr, String.c_str());
	delete [] cstr;
	return cstr;
}

char* SIPController::GetSIPA (string User, string Domain) {
	string SIPA = "sip:";
	SIPA.append(User);
	SIPA.append("@");
	SIPA.append(Domain);
	return this->CopyString(SIPA);
}

char* SIPController::GetSIPB (string Domain) {
	string SIPB = "sip:";
	SIPB.append(Domain);
	return this->CopyString(SIPB);
}

bool SIPController::FileExist (const std::string& Name) {
	std::ifstream f(Name.c_str());
	return f.is_open();
}


// Events

void SIPController::ReceiveCallInternalStateEvent (const SIPCallInternalStateEvent *Event) {
	 if (Event->StateType == SIPCallStateTypeDisconnected) {

	 }
}

void SIPController::ReceiveCallAnswerEvent (const SIPCallAnswerEvent *Event) {
	if (Event->AnswerType == SIPAnswerTypeAccept) {
		SIPCall *Call = Event->CallEvent->Call;

		this->SIPAcceptIncomingCall(Call);
	} else if (Event->AnswerType == SIPAnswerTypeReject) {
		SIPCall *Call = Event->CallEvent->Call;

		this->SIPRejectIncomingCall(Call);
	}
}

} /* namespace SIP */
