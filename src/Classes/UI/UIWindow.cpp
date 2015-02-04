/*
 * 		UIWindow.cpp
 *
 *  	Created on: 20.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "UIWindow.h"
#include <iostream>

namespace UI {

UIWindow::UIWindow() : _Initialized(false), _WindowsInitialized(false), _IOCInitialized(false), _SpeakInitialized(false), _SIPInitialized(false), _CurrentKeyAction(CommandCenterMenuActionsNone), _MotionInterfaceCurrentValue("0"), _ServoInterfaceServoAValue("0"), _ServoInterfaceServoBValue("0") {
	// TODO Auto-generated constructor stub

	_CommandCenterMenu = 0;
	_CommandCenter = 0;
	_Screen = 0;
	_ControlInterface = 0;
	_MotionInterface = 0;
	_SpeakInterface = 0;
	_ServoInterface = 0;
	_DebugView = 0;

	// nodelay(stdscr, true); // Initialisieren des Bildschirms !??!
	// keypad(stdscr, true); // Keyboard initialisieren ?!?!?
	// curs_set(0); // Cursor unsichtbar

	//attron(COLOR_PAIR(1)); // Farbe benutzen

	//WINDOW *win = newwin(100,100,20,20); // Neues Fenster initialisieren
	//wrefresh(win); // Fnster neuzeichnen
	//delwin(win); // Fenser löschen

	//noecho();
	//raw();

	//move(20, 20); // Pointer bewegen
	//mvprintw(); // Pointer bewegen und rausschreiben
	//attron(A_BOLD); // Bold schreiben AN
	// A_BOLD		Bold
	// A_REVERSE	Invertiert
	//printw("Hello World"); // Rausschreiben
	//attroff(A_BOLD); // Bold schreiben AUS
	//refresh(); // Bildschirm refreshen
	//attroff(COLOR_PAIR(1)); // Farbe ausschalten

	//int c;
	//while ((c=getch()) != 27) {
		//move(10,0);
		//printw("Keycode: %d, and the character: %c", c,c);
		//move(0,0);
		//printw("Write something (ESC to Escape): ");
		//refresh();
	//}
	//getch();

}

UIWindow::~UIWindow() {
	// TODO Auto-generated destructor stub

	if (_Initialized == true) {
		endwin(); // Bildschirm uninitialisieren

		SendEvent(new WindowEvent(WindowEventTypeCore, new WindowEventCore(WindowCoreEventNoneUnitialized)));
	}
}

void UIWindow::Initialize () {
	_Screen = initscr(); // Bildschirm initialisieren

	getmaxyx(_Screen, _ScreenSize.y, _ScreenSize.x); // Bildschirmgröße abfragen

	start_color(); // Farbe
	init_pair(1, COLOR_RED, COLOR_BLACK); // Farbe initialisieren

	start_color();
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);

	start_color();
	init_pair(3, COLOR_YELLOW, COLOR_RED);

	start_color();
	init_pair(4, COLOR_WHITE, COLOR_BLACK);

	start_color();
	init_pair(5, COLOR_GREEN, COLOR_BLACK);

	start_color();
	init_pair(6, COLOR_BLACK, COLOR_WHITE);

	nodelay(_Screen, true); // Initialisieren des Bildschirms !??!
	curs_set(0); // Cursor unsichtbar
	keypad(_Screen, true); // Keyboard initialisieren ?!?!?

	SendEvent(new WindowEvent(WindowEventTypeCore, new WindowEventCore(WindowCoreEventInitialized)));

	_Initialized = true;
}

bool UIWindow::IsInitialized () {
	return _Initialized;
}

void UIWindow::PaintScreen () {

}

bool UIWindow::StartScreen () {
	if (_Initialized == true) {
		string Welcome = "EXX2 V1 Emotion Control Phone Software V1\n";
		move(1, (_ScreenSize.x * 0.5) - (Welcome.size() * 0.5));
		printw(Welcome.data());
		move(2, (_ScreenSize.x * 0.5) - (Welcome.size() * 0.5));
		printw("=========================================\n");

		this->SetWindowControllerLoaded(true);
		return true;
	}

	return false;
}

bool UIWindow::EnableControllerView () {
	//move(1,1);
	//printw("Key: %d", this->GetInputKey());
	return false;
}

void UIWindow::TmpOutputData (string Data) {
	move(1,1);
	printw(Data.data());
	refresh();
}


// Command Center

bool UIWindow::StartCommandCenter () {
	if (_CommandCenter == 0) {
		_CommandCenter = newwin(30, 30, (_ScreenSize.y * 0.5) - 15,  (_ScreenSize.x * 0.5) - 15);
		//wbkgd(_ControlInterface, COLOR_PAIR(3));


		return this->PaintCommandCenter();
	}
	return false;
}

void UIWindow::UpdateCommandCenter () {
	this->PaintCommandCenter(_CurrentKeyAction);
}

UIUserAction UIWindow::RefreshCommandCenter (UIUserAction CurrentAction) {
	UIUserAction Action = UIUserActionNone;

	int ch = this->GetInputKey();

	    switch(ch) {
	      case KEY_DOWN:
	    	  if (_CommandCenterMenu != 0) {
	    		  menu_driver(_CommandCenterMenu->menu, REQ_DOWN_ITEM);
	    	  }
	    	  break;
	      case KEY_UP:
	    	  if (_CommandCenterMenu != 0) {
	    		  menu_driver(_CommandCenterMenu->menu, REQ_UP_ITEM);
	    	  }
	    	  break;
	      case 0xA: {
	    	  int Index = 0;
	    	  CommandCenterMenuActions KeyAction = CommandCenterMenuActionsNone;
	    	  if (_CommandCenterMenu != 0) {
	    		  Index = item_index(current_item(_CommandCenterMenu->menu));
	    		  KeyAction = _CommandCenterMenu->GetAction(Index);
	    	  }

	    	  switch ((int)KeyAction) {
	    	  	  case CommandCenterMenuActionsOpenMain: {
	    	  		  Action = UIUserActionMainMenu;
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenAuto: {
	    	  		  Action = UIUserActionStartAutomaticMode;
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenMan: {
	    	  		  Action = UIUserActionStartManualMode;
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSettings: {
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenExit: {
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsExit: {
	    	  		  Action = UIUserActionExit;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenIOController: {
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenIOControllerInitialize: {
	    	  		  Action = UIUserActionIOControllerInitialize;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenIOControllerUninitialize: {
	    	  		  Action = UIUserActionIOControllerUninitialize;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSpeakController: {
	    	  		this->PaintCommandCenter(KeyAction);
	    	  		break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSpeakControllerInitialize: {
	    	  		  Action = UIUserActionSpeakControllerInitialize;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSpeakControllerUninitialize: {
	    	  		  Action = UIUserActionSpeakControllerUninitialize;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSpeakControllerContinueRecognize: {
	    	  		  Action = UIUserActionSpeakControllerContinueRecognize;
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSpeakControllerStopRecognize: {
	    	  		  Action = UIUserActionSpeakControllerStopRecognize;
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSIPController: {
	    	  		  this->PaintCommandCenter(KeyAction);
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSIPControllerInitialize: {
	    	  		  Action = UIUserActionSIPControllerInitialize;
	    	  		  break;
	    	  	  }
	    	  	  case CommandCenterMenuActionsOpenSIPControllerUninitialize: {
	    	  		  Action = UIUserActionSIPControllerUninitialize;
	    	  		  break;
	    	  	  }
	    	  }

	    	  _CurrentKeyAction = KeyAction;
	        break;
	      }
	    }

	    wrefresh(_CommandCenter);

	    return Action;
}

bool UIWindow::PaintCommandCenter (CommandCenterMenuActions Mode, bool _Debug) {
	if (_CommandCenter != 0) {

		switch ((int)Mode) {
			case CommandCenterMenuActionsOpenMain: {


				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("Automatic Mode", CommandCenterMenuActionsOpenAuto));
				Items.insert(Items.end(), UIMenuItem("Manual Mode", CommandCenterMenuActionsOpenMan));
				Items.insert(Items.end(), UIMenuItem("Settings", CommandCenterMenuActionsOpenSettings));
				Items.insert(Items.end(), UIMenuItem("Exit", CommandCenterMenuActionsOpenExit));

				delete _CommandCenterMenu;
				_CommandCenterMenu = 0;
				_CommandCenterMenu = this->CreateMenu(Items);
				break;
			}
			case CommandCenterMenuActionsOpenAuto: {

				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMain));
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->CreateMenu(Items);

				break; }
			case CommandCenterMenuActionsOpenMan: {

				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("IOController", CommandCenterMenuActionsOpenIOController));
				Items.insert(Items.end(), UIMenuItem("SpeechController", CommandCenterMenuActionsOpenSpeakController));
				Items.insert(Items.end(), UIMenuItem("SIPController", CommandCenterMenuActionsOpenSIPController));
				Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMain));
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->CreateMenu(Items);

				break; }
			case CommandCenterMenuActionsOpenSettings: {

				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMain));
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->CreateMenu(Items);

				break; }
			case CommandCenterMenuActionsOpenExit: {

				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("NO", CommandCenterMenuActionsOpenMain));
				Items.insert(Items.end(), UIMenuItem("YES", CommandCenterMenuActionsExit));
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->CreateMenu(Items);

				break; }
			case CommandCenterMenuActionsOpenIOController: {

				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshIOControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenIOControllerInitialize: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshIOControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenIOControllerUninitialize: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshIOControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenSpeakController: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshSpeakControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenSpeakControllerInitialize: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshSpeakControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenSpeakControllerUninitialize: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshSpeakControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenSpeakControllerContinueRecognize: {
				vector<UIMenuItem> Items;
				Items.insert(Items.end(), UIMenuItem("Stop Recognize", CommandCenterMenuActionsOpenSpeakControllerStopRecognize));
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->CreateMenu(Items);

				break;
			}
			case CommandCenterMenuActionsOpenSpeakControllerStopRecognize: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshSpeakControllerMenu();
				break;
			}
			case CommandCenterMenuActionsOpenSIPController: {
				delete _CommandCenterMenu;
				_CommandCenterMenu = this->RefreshSIPControllerMenu();
				break;
			}
			default:
				break;
		}

		if (_Debug == false) {
			if (_CommandCenterMenu != 0) {
				set_menu_mark(_CommandCenterMenu->menu, "-->");
				set_menu_win (_CommandCenterMenu->menu, _CommandCenter);
				set_menu_sub (_CommandCenterMenu->menu, derwin(_CommandCenter, 4, 28, 3, 2));
				post_menu(_CommandCenterMenu->menu);
			}

			box(_CommandCenter, 0, 0);
			mvwhline(_CommandCenter, 0, 0, ACS_BULLET, 20);

			refresh();
			wrefresh(_CommandCenter);
		}

		return true;
	}
	return false;
}

UIMenu* UIWindow::RefreshIOControllerMenu () {
	vector<UIMenuItem> Items;
	if (_IOCInitialized == true) {
		Items.insert(Items.end(), UIMenuItem("Uninitialize", CommandCenterMenuActionsOpenIOControllerUninitialize));
	} else {
		Items.insert(Items.end(), UIMenuItem("Initialize", CommandCenterMenuActionsOpenIOControllerInitialize));
	}
	Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMan));
	return this->CreateMenu(Items);
}

UIMenu* UIWindow::RefreshSpeakControllerMenu () {
	vector<UIMenuItem> Items;
	if (_SpeakInitialized == true) {
		Items.insert(Items.end(), UIMenuItem("Uninitialize", CommandCenterMenuActionsOpenSpeakControllerUninitialize));
		Items.insert(Items.end(), UIMenuItem("Start Recognize", CommandCenterMenuActionsOpenSpeakControllerContinueRecognize));
	} else {
		Items.insert(Items.end(), UIMenuItem("Initialize", CommandCenterMenuActionsOpenSpeakControllerInitialize));
	}
	Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMan));
	return this->CreateMenu(Items);
}

UIMenu* UIWindow::RefreshSIPControllerMenu () {
	vector<UIMenuItem> Items;
		if (_SIPInitialized == true) {
			Items.insert(Items.end(), UIMenuItem("Uninitialize", CommandCenterMenuActionsOpenSIPControllerUninitialize));
			//Items.insert(Items.end(), UIMenuItem("Start Recognize", CommandCenterMenuActionsOpenSpeakControllerContinueRecognize));
		} else {
			Items.insert(Items.end(), UIMenuItem("Initialize", CommandCenterMenuActionsOpenSIPControllerInitialize));
		}
		Items.insert(Items.end(), UIMenuItem("Back", CommandCenterMenuActionsOpenMan));
		return this->CreateMenu(Items);
}


// Controller Interface

void UIWindow::Refresh () {
	refresh();
}

bool UIWindow::StartServoInterface () {
	if (_ServoInterface == 0) {
		_ServoInterface = newwin(15, 40, 20, 0);
		//wbkgd(_ControlInterface, COLOR_PAIR(3));

		box(_ServoInterface, 0, 0);
		mvwhline(_ServoInterface, 0, 0, ACS_BULLET, 20);

		return this->PaintServoInterface();
	}
	return false;
}

bool UIWindow::RemoveServoInterface () {
	if (_ServoInterface != 0) {
		delwin(_ServoInterface);
		_ServoInterface = 0;
		refresh();
		return true;
	}
	return false;
}

bool UIWindow::SetServoInterfaceValue (UIServoInterfaceValueType Type, string Value) {
	if (_ServoInterface != 0) {
		if (Type == UIServoInterfaceValueTypeServoAPositon) {
			if (_ServoInterfaceServoAValue != Value) {
				_ServoInterfaceServoAValue = Value;
				this->PaintServoInterface();
				return true;
			}
		} else if (Type == UIServoInterfaceValueTypeServoBPosition) {
			if (_ServoInterfaceServoBValue != Value) {
				_ServoInterfaceServoBValue = Value;
				this->PaintServoInterface();
				return true;
			}
		}
	}

	return false;
}

bool UIWindow::StartMotionInterface () {
	if (_MotionInterface == 0) {
		_MotionInterface = newwin(15, 40, 5,  0);
		//wbkgd(_ControlInterface, COLOR_PAIR(3));

		box(_MotionInterface, 0, 0);
		mvwhline(_MotionInterface, 0, 0, ACS_BULLET, 20);

		return this->PaintMotionInterface();
	}
	return false;
}

bool UIWindow::RemoveMotionInterface () {
	if (_MotionInterface != 0) {
		delwin(_MotionInterface);
		_MotionInterface = 0;
		refresh();
		return true;
	}
	return false;
}

bool UIWindow::SetMotionInterfaceValue (UIMotionInterfaceValueType Type, string Value) {
	if (_MotionInterface != 0) {
		if (Type == UIMotionInterfaceValueTypeCurrentValue) {
			if (_MotionInterfaceCurrentValue != Value) {
				Value = PreSignOnString(Value, 4);
				//cout << _MotionInterfaceCurrentValue.data() << endl;
				_MotionInterfaceCurrentValue = Value;
				this->PaintMotionInterface();
				return true;
			}
		}
	}

	return false;
}

bool UIWindow::StartSpeakInterface () {
	if (_SpeakInterface == 0) {

		SetSpeakControllerLoaded(true);
		SetDebugMessage("SpeechController Initialized");
		UpdateCommandCenter();

		_SpeakInterface = newwin(15, 40, 35, 0);
			//wbkgd(_ControlInterface, COLOR_PAIR(3));

			box(_SpeakInterface, 0, 0);
			mvwhline(_SpeakInterface, 0, 0, ACS_BULLET, 20);

			return this->PaintSpeakInterface();
		}
		return false;
}

bool UIWindow::SpeakInterfaceLoaded () {
	return (_SpeakInterface == 0 ? false : true);
}

bool UIWindow::RemoveSpeakInterface () {
	if (_SpeakInterface != 0) {

		SetSpeakControllerLoaded(false);
		SetDebugMessage("SpeechController Uninitialized");
		UpdateCommandCenter();

		delwin(_SpeakInterface);
		_SpeakInterface = 0;
		refresh();
		return true;
	}
	return false;
}

bool UIWindow::SetSpeakInterfaceValue (UISpeakInterfaceValueType Type, string Value) {
	if (_SpeakInterface != 0) {
		if (Type == UISpeakInterfaceValueTypeRecognition) {
			if (_SpeakInterfaceRecognitionCurrentValue != Value) {
				Value = Value.substr (0, 15);
				Value = PreSignOnString(Value, 15, " ");
				_SpeakInterfaceRecognitionCurrentValue = Value;
				this->PaintSpeakInterface();
				return true;
			}
		}
	}

	return false;
}

bool UIWindow::StartSIPInterface () {
	return false;
}

bool UIWindow::SIPInterfaceLoaded () {
	return false;
}

bool UIWindow::RemoveSIPInterface () {
	return false;
}

bool UIWindow::SetSIPInterfaceValue (UISIPInterfaceValueType Type, string Value) {
	return false;
}

bool UIWindow::StartControlInterface () {
	if (_ControlInterface == 0) {
		_ControlInterface = newwin(15, 40, 5,  _ScreenSize.x - 40);
		//wbkgd(_ControlInterface, COLOR_PAIR(3));


		return this->PaintControlInterface();
	}
	return false;
}

bool UIWindow::RemoveControlInterface () {
	if (_ControlInterface != 0) {
		delwin(_ControlInterface);
		_ControlInterface = 0;
		refresh();
		return true;
	}
	return false;
}

bool UIWindow::StartDebugView () {
	if (_DebugView == 0) {
		_DebugView = newwin(35, 40, 20,  _ScreenSize.x - 40);
		//wbkgd(_DebugView, COLOR_PAIR(6));

		box(_DebugView, 0, 0);
		mvwhline(_DebugView, 0, 0, ACS_BULLET, 20);

		return this->PaintDebugView();
	}
	return false;
}

bool UIWindow::RemoveDebugView () {
	if (_DebugView != 0) {
		delwin(_DebugView);
		_DebugView = 0;
		refresh();
		return true;
	}
	return false;
}

void UIWindow::SetDebugMessage (string Message) {
	_DebugInformation.insert(_DebugInformation.end(), Message);

	if (_DebugInformation.size() > 30) {
		_DebugInformation.erase(_DebugInformation.begin());
	}

	this->PaintDebugView();
}

void UIWindow::SetSpeakControllerLoaded (bool Loaded) {
	_SpeakInitialized = Loaded;
	this->PaintControlInterface();
}

void UIWindow::SetWindowControllerLoaded (bool Loaded) {
	_WindowsInitialized = Loaded;
	this->PaintControlInterface();
}

void UIWindow::SetSIPControllerLoaded (bool Loaded) {
	_SIPInitialized = Loaded;
	this->PaintControlInterface();
}

void UIWindow::SetIOControllerLoaded (bool Loaded) {
	_IOCInitialized = Loaded;
	this->PaintControlInterface();
}


// Key Working

int UIWindow::GetInputKey () {
	return wgetch(_Screen);
}


// Helper

bool UIWindow::PaintDebugView () {
	if (_DebugView != 0) {

		string Title		= "Debug View";

		wattron(_DebugView, COLOR_PAIR(2));
		mvwaddstr(_DebugView, 1, 20 - (Title.size() * 0.5), Title.data());
		wattroff(_DebugView, COLOR_PAIR(2));

		vector<string>::iterator it = _DebugInformation.begin();


		wattron(_DebugView, COLOR_PAIR(5));

		int i = 0;
		while (it != _DebugInformation.end()) {
			mvwaddstr(_DebugView, 4+i, 3, (*it).data());
			it++;
			i++;
		}

		wattroff(_DebugView, COLOR_PAIR(5));

		refresh();
		wrefresh(_DebugView);

		return true;
	}
	return false;
}

bool UIWindow::PaintServoInterface () {
	if (_ServoInterface != 0) {

			string Title		= "Servo Interface";

			string Opt1			= "CurrentValueA:";
			string Opt2			= "CurrentValueB:";


			wattron(_ServoInterface, COLOR_PAIR(2));
			mvwaddstr(_ServoInterface, 1, 20 - (Title.size() * 0.5), Title.data());
			wattroff(_ServoInterface, COLOR_PAIR(2));

			mvwaddstr(_ServoInterface, 4, 2, Opt1.data());
			mvwaddstr(_ServoInterface, 5, 2, Opt2.data());

			wattron(_ServoInterface, COLOR_PAIR(5));
			mvwaddstr(_ServoInterface, 4, 34, (_ServoInterfaceServoAValue == "0" ? "0000" : _ServoInterfaceServoAValue.data()));
			mvwaddstr(_ServoInterface, 5, 34, (_ServoInterfaceServoBValue == "0" ? "0000" : _ServoInterfaceServoBValue.data()));
			wattroff(_ServoInterface, COLOR_PAIR(5));

			//refresh();
			wrefresh(_ServoInterface);

			return true;
		}

		return false;
}

bool UIWindow::PaintMotionInterface () {
	if (_MotionInterface != 0) {

		touchwin(_MotionInterface);

		string Title		= "Motion Interface";

		string Opt1			= "CurrentValue:";


		wattron(_MotionInterface, COLOR_PAIR(2));
		mvwaddstr(_MotionInterface, 1, 20 - (Title.size() * 0.5), Title.data());
		wattroff(_MotionInterface, COLOR_PAIR(2));

		mvwaddstr(_MotionInterface, 4, 2, Opt1.data());

		wattron(_MotionInterface, COLOR_PAIR(5));
		mvwaddstr(_MotionInterface, 4, 34, (_MotionInterfaceCurrentValue == "0" ? "0000" : _MotionInterfaceCurrentValue.data()));

		wattroff(_MotionInterface, COLOR_PAIR(5));

		//refresh();
		wrefresh(_MotionInterface);

		return true;
	}

	return false;
}

bool UIWindow::PaintSpeakInterface () {
	if (_SpeakInterface != 0) {

			touchwin(_SpeakInterface);

			string Title		= "Speak Interface";

			string Opt1			= "CurrentDetection:";


			wattron(_SpeakInterface, COLOR_PAIR(2));
			mvwaddstr(_SpeakInterface, 1, 20 - (Title.size() * 0.5), Title.data());
			wattroff(_SpeakInterface, COLOR_PAIR(2));

			mvwaddstr(_SpeakInterface, 4, 2, Opt1.data());

			wattron(_SpeakInterface, COLOR_PAIR(5));
			mvwaddstr(_SpeakInterface, 4, 20, (_SpeakInterfaceRecognitionCurrentValue == "0" ? "" : _SpeakInterfaceRecognitionCurrentValue.data()));

			wattroff(_SpeakInterface, COLOR_PAIR(5));

			//refresh();
			wrefresh(_SpeakInterface);

			return true;
		}

		return false;
}

bool UIWindow::PaintControlInterface () {
	if (_ControlInterface != 0) {
		string Title		= "Control Interface";

		string WCTitle		= "WindowController";
		string SCTitle		= "SpeechController";
		string SIPTitle		= "SIPController";
		string IOCTitle		= "IOController";

		string WCInit		= "None";
		string SCInit		= "None";
		string SIPInit		= "None";
		string IOCInit		= "None";

		int WCColor			= 0;
		int SCColor			= 0;
		int SIPColor		= 0;
		int IOCColor		= 0;

		wattron(_ControlInterface, COLOR_PAIR(2));
		mvwaddstr(_ControlInterface, 1, 20 - (Title.size() * 0.5), Title.data());
		wattroff(_ControlInterface, COLOR_PAIR(2));

		mvwaddstr(_ControlInterface, 4, 2, WCTitle.data());
		mvwaddstr(_ControlInterface, 6, 2, SCTitle.data());
		mvwaddstr(_ControlInterface, 8, 2, SIPTitle.data());
		mvwaddstr(_ControlInterface, 10, 2, IOCTitle.data());

		if (_WindowsInitialized == true) {
			WCInit = "Done";
			WCColor = 5;
		}

		if (_SpeakInitialized == true) {
			SCInit = "Done";
			SCColor = 5;
		}

		if (_SIPInitialized == true) {
			SIPInit = "Done";
			SIPColor = 5;
		}

		if (_IOCInitialized == true) {
			IOCInit = "Done";
			IOCColor = 5;
		}

		(WCColor == 0 ? wattron(_ControlInterface, COLOR_PAIR(1)) : wattron(_ControlInterface, COLOR_PAIR(5)));
		mvwaddstr(_ControlInterface, 4, 34, WCInit.data());
		(WCColor == 0 ? wattroff(_ControlInterface, COLOR_PAIR(2)) : wattroff(_ControlInterface, COLOR_PAIR(5)));

		(SCColor == 0 ? wattron(_ControlInterface, COLOR_PAIR(1)) : wattron(_ControlInterface, COLOR_PAIR(5)));
		mvwaddstr(_ControlInterface, 6, 34, SCInit.data());
		(SCColor == 0 ? wattroff(_ControlInterface, COLOR_PAIR(2)) : wattroff(_ControlInterface, COLOR_PAIR(5)));

		(SIPColor == 0 ? wattron(_ControlInterface, COLOR_PAIR(1)) : wattron(_ControlInterface, COLOR_PAIR(5)));
		mvwaddstr(_ControlInterface, 8, 34, SIPInit.data());
		(SIPColor == 0 ? wattroff(_ControlInterface, COLOR_PAIR(2)) : wattroff(_ControlInterface, COLOR_PAIR(5)));

		(IOCColor == 0 ? wattron(_ControlInterface, COLOR_PAIR(1)) : wattron(_ControlInterface, COLOR_PAIR(5)));
		mvwaddstr(_ControlInterface, 10, 34, IOCInit.data());
		(IOCColor == 0 ? wattroff(_ControlInterface, COLOR_PAIR(2)) : wattroff(_ControlInterface, COLOR_PAIR(5)));

		box(_ControlInterface, 0, 0);
		mvwhline(_ControlInterface, 0, 0, ACS_BULLET, 20);

		//refresh();
		wrefresh(_ControlInterface);

		return true;
	}
	return false;
}

UIMenu* UIWindow::CreateMenu (vector<UIMenuItem> Items) {
	ITEM **it = (ITEM **)calloc(Items.size() + 1, sizeof(ITEM *));

	vector<UIMenuItem>::iterator iterator = Items.begin();
	int i = 0;
	while (iterator != Items.end()) {
		it[i] = new_item((*iterator).name.data(), "");
		i++;
		iterator++;
	}
	it[i] = 0;

	return new UIMenu(new_menu(it), it, Items);
}

string UIWindow::PreSignOnString (string In, int Size, string Sign) {
	string buf = In;
	int InSize = buf.size();
	for (int i = InSize; i < Size; i++) {
		buf.insert(0, Sign);
	}
	return buf;
}

} // Namespace
