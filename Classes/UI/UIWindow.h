/*
 * 		UIWindow.h
 *
 *  	Created on: 20.05.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef UIWINDOW_H_
#define UIWINDOW_H_

#include <ncurses.h>
#include <menu.h>

#include <string>
#include <vector>

#include "../Event/EventHandler.h"

using namespace std;

namespace UI {

	enum UIServoInterfaceValueType {
		UIServoInterfaceValueTypeServoAPositon,
		UIServoInterfaceValueTypeServoBPosition,
	};

	enum UIMotionInterfaceValueType {
		UIMotionInterfaceValueTypeCurrentValue,
	};

	enum UISpeakInterfaceValueType {
		UISpeakInterfaceValueTypeRecognition,
	};

	enum UISIPInterfaceValueType {
		UISIPInterfaceValueTypeStatus,
	};

	enum UIUserAction {
		UIUserActionNone,
		UIUserActionStartAutomaticMode,
		UIUserActionStopAutomaticMode,
		UIUserActionStartManualMode,
		UIUserActionStopManualMode,
		UIUserActionMainMenu,
		UIUserActionIOControllerInitialize,
		UIUserActionIOControllerUninitialize,
		UIUserActionSpeakControllerInitialize,
		UIUserActionSpeakControllerUninitialize,
		UIUserActionSpeakControllerContinueRecognize,
		UIUserActionSpeakControllerStopRecognize,
		UIUserActionSIPControllerInitialize,
		UIUserActionSIPControllerUninitialize,
		UIUserActionExit,
	};

	enum CommandCenterMenuActions {
		CommandCenterMenuActionsNone,
		CommandCenterMenuActionsOpenMain,
		CommandCenterMenuActionsOpenAuto,
		CommandCenterMenuActionsOpenMan,
		CommandCenterMenuActionsOpenIOController,
		CommandCenterMenuActionsOpenIOControllerInitialize,
		CommandCenterMenuActionsOpenIOControllerUninitialize,
		CommandCenterMenuActionsOpenSpeakController,
		CommandCenterMenuActionsOpenSpeakControllerInitialize,
		CommandCenterMenuActionsOpenSpeakControllerUninitialize,
		CommandCenterMenuActionsOpenSpeakControllerContinueRecognize,
		CommandCenterMenuActionsOpenSpeakControllerStopRecognize,
		CommandCenterMenuActionsOpenSIPController,
		CommandCenterMenuActionsOpenSIPControllerInitialize,
		CommandCenterMenuActionsOpenSIPControllerUninitialize,
		CommandCenterMenuActionsOpenSettings,
		CommandCenterMenuActionsOpenExit,
		CommandCenterMenuActionsExit
	};

	enum WindowEventType {
		WindowEventTypeCore = 0,
		WindowEventTypeAction
	};

	enum WindowCoreEvent {
		WindowCoreEventNone = 0,
		WindowCoreEventInitialized,
		WindowCoreEventReady,
		WindowCoreEventBusy,
		WindowCoreEventNoneUnitialized
	};

	enum WindowActionEvent {
		WindowActionEventNone = 0
	};

	class WEvent {
		public:
			virtual ~WEvent() {};
	};

	class WindowEventCore : public WEvent {
		public:
			WindowEventCore(WindowCoreEvent _Event) : _Event(_Event) {};
			~WindowEventCore() {};

			const WindowCoreEvent _Event;
	};

	class WindowEventAction : public WEvent {
		public:
			WindowEventAction(WindowActionEvent _Event) : _Event(_Event) {};

			const WindowActionEvent _Event;
	};

	class WindowEvent : public Event {
		public:
			WindowEvent(WindowEventType _Type, WEvent *_Event) : _Type(_Type), _Event(_Event) {};

			const WindowEventType _Type;
			const WEvent *_Event;
	};

	struct UIRect {
		int x;
		int y;

		UIRect () : x(0), y(0) {};
		UIRect (int x, int y) : x(x), y(y) {};
		~UIRect() {};
	};

	struct UIMenuItem {
		string name;
		CommandCenterMenuActions action;

		UIMenuItem (string name, CommandCenterMenuActions action) : name(name), action(action) {};
	};

	struct UIMenu {
		MENU *menu;
		ITEM **items;
		vector<UIMenuItem> actions;

		UIMenu() : menu(0), items(0) {};
		UIMenu(MENU *menu, ITEM **items, vector<UIMenuItem> actions) : menu(menu), items(items), actions(actions) {};
		~UIMenu() {
			actions.clear();

			if (menu != 0) {
				unpost_menu(menu);
				free_menu(menu);
			}

			if (items != 0) {
				for(unsigned int i=0; i<=actions.size(); i++) {
					free_item(items[i]);
				}

				free(items);
			}
		};
		CommandCenterMenuActions GetAction(int Index) {
			return actions[Index].action;
		};
	};

	class UIWindow : public EventHandler {
	public:
		UIWindow();
		virtual ~UIWindow();

		void Initialize ();
		bool IsInitialized ();

		bool StartScreen ();
		bool EnableControllerView ();
		void TmpOutputData (string Data);

		// Command Center
		bool StartCommandCenter ();
		void UpdateCommandCenter ();
		UIUserAction RefreshCommandCenter (UIUserAction CurrentAction = UIUserActionNone);
		bool PaintCommandCenter (CommandCenterMenuActions Mode = CommandCenterMenuActionsOpenMain, bool _Debug = false);

		// Events


		// Controller Interface
		void Refresh ();
		bool StartServoInterface ();
		bool RemoveServoInterface ();
		bool SetServoInterfaceValue (UIServoInterfaceValueType Type, string Value);
		bool StartMotionInterface ();
		bool RemoveMotionInterface ();
		bool SetMotionInterfaceValue (UIMotionInterfaceValueType Type, string Value);
		bool StartSpeakInterface ();
		bool SpeakInterfaceLoaded ();
		bool RemoveSpeakInterface ();
		bool SetSpeakInterfaceValue (UISpeakInterfaceValueType Type, string Value);
		bool StartSIPInterface ();
		bool SIPInterfaceLoaded ();
		bool RemoveSIPInterface ();
		bool SetSIPInterfaceValue (UISIPInterfaceValueType Type, string Value);
		bool StartControlInterface ();
		bool RemoveControlInterface ();
		bool StartDebugView ();
		bool RemoveDebugView ();
		void SetDebugMessage (string Message);
		void SetSpeakControllerLoaded (bool Loaded);
		void SetWindowControllerLoaded (bool Loaded);
		void SetSIPControllerLoaded (bool Loaded);
		void SetIOControllerLoaded (bool Loaded);

		// Key Working
		int GetInputKey ();

	private:

		bool 	_Initialized;
		bool	_WindowsInitialized;
		bool	_IOCInitialized;
		bool	_SpeakInitialized;
		bool	_SIPInitialized;
		UIRect	_ScreenSize;
		WINDOW 	*_Screen;
		WINDOW 	*_ControlInterface;
		WINDOW 	*_MotionInterface;
		WINDOW	*_SpeakInterface;
		WINDOW 	*_ServoInterface;
		WINDOW	*_DebugView;
		WINDOW 	*_CommandCenter;
		UIMenu	*_CommandCenterMenu;

		CommandCenterMenuActions _CurrentKeyAction;

		vector<string>	_DebugInformation;
		string			_MotionInterfaceCurrentValue;
		string			_SpeakInterfaceRecognitionCurrentValue;
		string			_ServoInterfaceServoAValue;
		string			_ServoInterfaceServoBValue;

		void PaintScreen ();

		// Helper
		UIMenu* RefreshIOControllerMenu ();
		UIMenu* RefreshSpeakControllerMenu ();
		UIMenu* RefreshSIPControllerMenu ();
		bool PaintDebugView ();
		bool PaintServoInterface ();
		bool PaintMotionInterface ();
		bool PaintSpeakInterface ();
		bool PaintControlInterface ();

		UIMenu* CreateMenu (vector<UIMenuItem> Items);

		string PreSignOnString (string In, int Size, string Sign = "0");
	};

}

#endif /* UIWINDOW_H_ */
