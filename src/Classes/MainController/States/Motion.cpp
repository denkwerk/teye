/*
 * 		Motion.cpp
 *
 *  	Created on: 11.09.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "Motion.h"
#include "Settings.h"

namespace EXState {
namespace ApplicationSettings {


// -----------------------------------------------------------------------------------------------
// Motion ----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

Motion::Motion(StateMachine::StateInterface *BackState) : BackState(BackState) {
	// TODO Auto-generated constructor stub

}

Motion::~Motion() {
	// TODO Auto-generated destructor stub
}

void Motion::Initialize() {
	PrintText("hello -> motion", true);
	PrintText("Command (calibration/settings/live/back/<-) : ");
}


// Events

void Motion::ReceiveKeyEvent (const KeyEvent *Event) {
	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "back";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (Result == "calibration") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::MotionCalibration(this, this), false);
			} else if (Result == "settings") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::MotionSettings());
			} else if (Result == "live") {
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::MotionLive());
			} else if (Result == "back") {
				NewLine();
				if (BackState != 0) {
					Context->SetState(BackState);
				} else {
					Context->SetState(new EXState::Settings());
				}
			} else {
				CleanUp(Result);
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}


// -----------------------------------------------------------------------------------------------
// MotionLive ------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

MotionLive::MotionLive(StateMachine::StateInterface *BackState) : BackState(BackState), _CurrentPercent(0), _OldValue(0.0) {
	// TODO Auto-generated constructor stub

}

MotionLive::~MotionLive() {
	// TODO Auto-generated destructor stub
}

void MotionLive::Initialize() {

	float Min = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMin);
	float Max = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMax);

	PrintText("hello -> motion", true);
	PrintText(std::string("MinValue: \"").append(to_string(Min)).append("\" MaxValue: \"").append(to_string(Max)).append("\""), true);
	PrintText("Command (back/<-) : ");

	_CurrentPercent = 0;
}


// Events

void MotionLive::ReceiveRawMotionData(const MotionRawEvent *Event) {
	float Min = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMin);
	float Max = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMax);
	float CurrentValue = MCP3008Function->GetValue();
	float Percent = CurrentValue / ((Max - Min) / 100.0);
	MotionCalibration::Repaint(Percent, _CurrentPercent, 25/*, to_string(CurrentValue), to_string(_OldValue)*/);
	_OldValue = CurrentValue;
}

void MotionLive::ReceiveKeyEvent (const KeyEvent *Event) {
	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "back";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (Result == "back") {
				NewLine();
				if (BackState != 0) {
					Context->SetState(BackState);
				} else {
					Context->SetState(new EXState::ApplicationSettings::Motion());
				}
			} else {
				CleanUp(Result);
			}
		}
	}

	StateInterface::ReceiveKeyEvent(Event);
}


// -----------------------------------------------------------------------------------------------
// MotionCalibration -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

MotionCalibration::MotionCalibration(StateMachine::StateInterface *SuccessBackState, StateMachine::StateInterface *FailedBackState) : SuccessBackState(SuccessBackState), FailedBackState(FailedBackState), Started(false), _CurrentPercent(0), Step(MotionDetection::CalibrationStepNone) {
	// TODO Auto-generated constructor stub

}

MotionCalibration::~MotionCalibration() {
	// TODO Auto-generated destructor stub
	MotionDetection->SetCalibrationData(0, 0);
}

void MotionCalibration::Initialize() {
	MotionDetection->StartCalibration();
	PrintText("hello -> motion calibration", true);
	PrintText("Go to the farthest Distance or type \"back\" or \"<-\": ");
}


// Events

void MotionCalibration::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {

}

void MotionCalibration::ReceiveMotionCalibrationEvent (const MotionDetection::MotionCalibrationEvent *Event) {
	if (Started == false) {
		NewLine();
		PrintText("Max: ");
		Started = true;
	}

	int Percent = 0;

	if (Event->CalibrationStepEntry == MotionDetection::CalibrationStepMin) {
		Step = Event->CalibrationStepEntry;
		Percent = MotionDetection->GetCalibrationPercent();
	} if (Event->CalibrationStepEntry == MotionDetection::CalibrationStepMax) {
		Percent = MotionDetection->GetCalibrationPercent();
	}

	if (Step == MotionDetection::CalibrationStepMin && Event->CalibrationStepEntry == MotionDetection::CalibrationStepMax) {
		Step = Event->CalibrationStepEntry;
		PrintText(" [\x1b[32mdone\x1b[39m]", true);
		PrintText("Min: ");
		_CurrentPercent = 0;
	} else if (Step == MotionDetection::CalibrationStepMax && Event->CalibrationStepEntry == MotionDetection::CalibrationStepFinished) {
		Step = Event->CalibrationStepEntry;
		PrintText(" [\x1b[32mdone\x1b[39m]", true);
		PrintText("Save? (yes/no) : ");
		_CurrentPercent = 0;
	}

	if (Event->CalibrationStepEntry != MotionDetection::CalibrationStepFinished) {
		Repaint(Percent, _CurrentPercent, 25);
	}
}

void MotionCalibration::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		bool Display = (Step != MotionDetection::CalibrationStepMax || Step != MotionDetection::CalibrationStepMin);

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result, false, Display)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "back";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (SuccessBackState != 0 || FailedBackState != 0) {
				MotionDetection->StopCalibration();
				NewLine();
				if (MotionDetection->GetCalibrationStatus() == MotionDetection::CalibrationStepDone && MotionDetection->Status() != MotionDetection::MotionStatusCalibrationModus) {
					if (SuccessBackState != 0) {
						Context->SetState(SuccessBackState);
					}
				} else {
					if (FailedBackState != 0) {
						Context->SetState(FailedBackState);
					}
				}
			} else {
				MotionDetection->StopCalibration();
				NewLine();
				Context->SetState(new EXState::ApplicationSettings::Motion());
			}
		} else if (Result == "yes") {
			if (Step == MotionDetection::CalibrationStepFinished) {
				ApplicationSettings->_Motion_Calibration_Max = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMax);
				ApplicationSettings->_Motion_Calibration_Min = MotionDetection->GetCalibrationData(MotionDetection::CalibrationValueMin);
				if (SIPController->SaveApplicationSettings(*ApplicationSettings)) {
					NewLine();
					PrintText("Command (exit/restart) : ");
				} else {
					NewLine();
					PrintText("###Error### Not Saved.", true);
					PrintText("Command (back/restart) : ");
				}
				MotionDetection->CalibrationDone();
			}
		} else if (Result == "no") {
			if (Step == MotionDetection::CalibrationStepFinished) {
				Started = false;
				MotionDetection->SetCalibrationData(0, 0);
			}
		} else if (Result == "restart") {
			Started = false;
			MotionDetection->SetCalibrationData(0, 0);
		} else {
			if (Display == true) {
				CleanUp(Result);
			}
		}
	}
}


// Internal

void MotionCalibration::Repaint (int Percent, int &Current, int MaxDistance, std::string OptionalValue, std::string OldValue) {

	int CurrentDistance = ceilf(Current / (100 / MaxDistance));
	int NewDistance = ceilf(Percent / (100 / MaxDistance));

	Current = Percent;
	NewDistance = NewDistance - CurrentDistance;

	if (OptionalValue.length() > 0) {
		if (OldValue.length() > 0) {
			for (unsigned int i = 0; i < OldValue.length() + 1; i++) {
				StateMachine::StateInterface::S_DeleteLastSign();
			}
		}
		StateMachine::StateInterface::S_PrintText(std::string(" ").append(OptionalValue));
	}

	if (NewDistance < 0) {
		for (int i = 0; i < abs((int)NewDistance); i++) {
			StateMachine::StateInterface::S_DeleteLastSign();
		}
	} else {
		for (int i = 0; i < NewDistance; i++) {
			StateMachine::StateInterface::S_PrintText("#");
		}
	}
}


// -----------------------------------------------------------------------------------------------
// MotionSettings --------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

MotionSettings::MotionSettings(StateMachine::StateInterface *BackState) : BackState(BackState) {
	// TODO Auto-generated constructor stub

}

MotionSettings::~MotionSettings() {
	// TODO Auto-generated destructor stub
}

void MotionSettings::Initialize() {
	PrintText("hello -> motion settings", true);
	PrintText("Command (list/set {setting}={value}/get {setting}/back/<-) : ");
}


// Events

void MotionSettings::ReceiveMotionEvent (const MotionDetection::MotionEvent *Event) {

}

void MotionSettings::ReceiveKeyEvent (const KeyEvent *Event) {

	if (Event->_Event == KeyEventTypePressed) {

		std::string Result;
		bool GoOn = false;

		if (this->MapInput(Event, Result)) {
			GoOn = true;
		} else {
			if (Event->Key == KEY_ACTION_LARROW) {
				Result = "back";
				GoOn = true;
			}
		}

		if (GoOn == true) {
			if (Result == "back") {
				NewLine();
				if (BackState != 0) {
					Context->SetState(BackState);
				} else {
					Context->SetState(new EXState::ApplicationSettings::Motion());
				}
			} else if (Result == "list") {
				NewLine();
				PrintText("Settings List", true);
				NewLine();
				PrintText("- calib_min", true);
				PrintText("- calib_max", true);
				PrintText("- calib_treshold", true);
				PrintText("- calib_timeout", true);
				PrintText("- calib_offset", true);
				PrintText("- port_clk", true);
				PrintText("- port_dout", true);
				PrintText("- port_din", true);
				PrintText("- port_cs", true);
				NewLine();
				PrintText("Command (list/set {setting}={value}/get {setting}/back/<-) : ");
			} else {
				if (Result != "") {
					if (this->ParseSetting(Result)) {
						PrintText("Command (list/set {setting}={value}/get {setting}/back/<-) : ");
					} else {
						CleanUp(Result);
					}
				}
			}
		}
	}
}


// Internal

bool MotionSettings::ParseSetting (std::string Result) {
	vector<string> Split = SplitString(Result, ' ');
	if (Split.size() == 2 && Split[0] == "set") {
		vector<string> Parts = SplitString(Split[1], '=');
		if (Parts.size() == 2) {
			int Number = 0;
			if (ToSaveNumber(Parts[1], Number)) {
				SettingWorker(Parts[0], Number, false);
				return true;
			}
		}
	} else if (Split.size() == 2 && Split[0] == "get") {
		SettingWorker(Split[1], 0, true);
		return true;
	}
	return false;
}

std::string MotionSettings::SettingWorker (std::string Setting, int Value, bool Get) {
	std::string GetString = "";
	if (Setting == "calib_min") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Calibration_Min);
		} else {
			ApplicationSettings->_Motion_Calibration_Min = Value;
			MotionDetection->SetCalibrationData(ApplicationSettings->_Motion_Calibration_Min, ApplicationSettings->_Motion_Calibration_Max);
		}
	} else if (Setting == "calib_max") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Calibration_Max);
		} else {
			ApplicationSettings->_Motion_Calibration_Max = Value;
			MotionDetection->SetCalibrationData(ApplicationSettings->_Motion_Calibration_Min, ApplicationSettings->_Motion_Calibration_Max);
		}
	} else if (Setting == "calib_treshold") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Calibration_Threshold);
		} else {
			ApplicationSettings->_Motion_Calibration_Threshold = Value;
			MotionDetection->SetThreshold(ApplicationSettings->_Motion_Calibration_Threshold);
		}
	} else if (Setting == "calib_timeout") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Calibration_Timeout);
		} else {
			ApplicationSettings->_Motion_Calibration_Timeout = Value;
			MotionDetection->SetTimeout(ApplicationSettings->_Motion_Calibration_Timeout);
		}
	} else if (Setting == "calib_offset") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Calibration_CalibrationOffset);
		} else {
			ApplicationSettings->_Motion_Calibration_CalibrationOffset = Value;
			MotionDetection->SetCalibrationOffset(ApplicationSettings->_Motion_Calibration_CalibrationOffset);
		}
	} else if (Setting == "port_clk") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Port_CLK);
		} else {
			ApplicationSettings->_Motion_Port_CLK = Value;
		}
	} else if (Setting == "port_dout") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Port_DOut);
		} else {
			ApplicationSettings->_Motion_Port_DOut = Value;
		}
	} else if (Setting == "port_din") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Port_DIn);
		} else {
			ApplicationSettings->_Motion_Port_DIn = Value;
		}
	} else if (Setting == "port_cs") {
		if (Get) {
			GetString = to_string(ApplicationSettings->_Motion_Port_CS);
		} else {
			ApplicationSettings->_Motion_Port_CS = Value;
		}
	}

	if (Get == false) {
		if (SIPController->SaveApplicationSettings(*ApplicationSettings)) {
			PrintText(std::string(" Success: "), true);
		} else {
			PrintText(std::string("Failed: "), true);
		}
	} else {
		PrintText(std::string(" Value: ").append(GetString), true);
	}

	return GetString;
}

} /* namespace ApplicationSettings */
} /* namespace EXState */
