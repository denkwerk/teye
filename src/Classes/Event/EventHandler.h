/*
 * 		EventHandler.h
 *
 *  	Created on: 14.07.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include "TypeInfo.h"
#include <map>
#include <vector>

#include <string.h>

#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

//#define OUTPUT_RAW
//#define RAW_TEST
#define EVENT_DEBUG_OUTPUT

enum SystemEventType {
	SystemEventTypeNone,
	SystemEventTypeStartup,
	SystemEventTypeRun,
	SystemEventTypeExit,
	SystemEventTypeShutdown,
};

enum KeyEventType {
	KeyEventTypeNone,
	KeyEventTypePressed,
	KeyEventTypeRelease
};

enum KEY {
	KEY_EM = 0,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_EQUAL,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_a,
	KEY_b,
	KEY_c,
	KEY_d,
	KEY_e,
	KEY_f,
	KEY_g,
	KEY_h,
	KEY_i,
	KEY_j,
	KEY_k,
	KEY_l,
	KEY_m,
	KEY_n,
	KEY_o,
	KEY_p,
	KEY_q,
	KEY_r,
	KEY_s,
	KEY_t,
	KEY_u,
	KEY_v,
	KEY_w,
	KEY_x,
	KEY_y,
	KEY_z,
	KEY_ACTION_ENTER,
	KEY_ACTION_BACKSPACE,
	KEY_ACTION_SPACE,
	KEY_ACTION_UARROW,
	KEY_ACTION_DARROW,
	KEY_ACTION_RARROW,
	KEY_ACTION_LARROW,
	KEY_ACTION_PASTE,
	KEY_ACTION_ERASE,
	KEY_ACTION_POS1,
	KEY_ACTION_END,
	KEY_ACTION_PICUP,
	KEY_ACTION_PICDOWN,
	KEY_ACTION_PAUSE,
	KEY_ACTION_TAB,
	KEY_ACTION_FACTOR,
	KEY_ACTION_MULTIPLIER,
	KEY_ACTION_MINUS,
	KEY_ACTION_UNDERSCORE,
	KEY_ACTION_PLUS,
	KEY_ACTION_COMMA,
	KEY_ACTION_POINT,
	KEY_ACTION_NUMBERSIGN,
	KEY_ACTION_COLON,
	KEY_ACTION_SEMICOLON,
};

namespace Keyboard {
	struct KEYMAP {

		public:
			KEYMAP() : MAP{0}, COUNT(0) {};
			KEYMAP(unsigned int A, unsigned int B, unsigned int C, unsigned int D, unsigned int E, std::string K = std::string("")) : MAP{A, B, C, D, E}, KEYSYMBOL(K), COUNT(5) {};

			bool INSERT(unsigned int V) {if (COUNT < 5) {MAP[COUNT] = V;COUNT++;return true;}return false;};
			bool FULL() {if (COUNT == 5) {return true;}return false;};
			unsigned int SIZE() {return COUNT;};
			void RESET() {for(int i=0;i<5;i++){MAP[i]=0;}COUNT=0;};
			std::string SYMBOL() {return KEYSYMBOL;};

			bool operator< (const KEYMAP &m) const {return (MAP[0] < m.MAP[0] || MAP[1] < m.MAP[1] || MAP[2] < m.MAP[2] || MAP[3] < m.MAP[3] || MAP[4] < m.MAP[4]);}
			bool operator<= (const KEYMAP &m) const {return (MAP[0] < m.MAP[0] || MAP[1] < m.MAP[1] || MAP[2] < m.MAP[2] || MAP[3] < m.MAP[3] || MAP[4] < m.MAP[4]);}
			bool operator> (const KEYMAP &m) const {return (MAP[0] > m.MAP[0] || MAP[1] > m.MAP[1] || MAP[2] > m.MAP[2] || MAP[3] > m.MAP[3] || MAP[4] > m.MAP[4]);}
			bool operator== (const KEYMAP &m) const {return (MAP[0] == m.MAP[0] && MAP[1] == m.MAP[1] && MAP[2] == m.MAP[2] && MAP[3] == m.MAP[3] && MAP[4] == m.MAP[4]);}

		private:
			unsigned int	MAP[5];
			std::string		KEYSYMBOL;

			unsigned int	COUNT;
	};
}

typedef std::map<KEY, Keyboard::KEYMAP> KEY_MAP;
typedef std::vector<int> ACTION_MAP;

/*struct Key {
	static const int F1 = Key::F1;
};*/

class SystemPayload {
	public:
		virtual ~SystemPayload() {};
};

class Event {
	protected:
		virtual ~Event() {};

	public:
		std::string DebugInfo (void) {return std::string("NoDebug Info");};
};

class SystemEvent : public Event {
	public:
		SystemEvent (SystemEventType Type) : _EventType(Type), _Payload(0) {};
		SystemEvent (SystemEventType Type, SystemPayload *_Payload) : _EventType(Type), _Payload(_Payload) {};

		SystemEventType _EventType;
		SystemPayload *_Payload;
};

class KeyEvent : public Event {
	public:
		KeyEvent (KeyEventType _Event, KEY Key, bool ActionKey, std::string Symbol = std::string("")) : _Event(_Event), Key(Key), ActionKey(ActionKey), Symbol(Symbol) {};

		KeyEventType _Event;
		KEY Key;
		bool ActionKey;
		std::string Symbol;
};

class TimeEvent {

	public:
		TimeEvent(long long WaitTime, const Event *_Event, void *EventQueue);
		~TimeEvent() {this->Destroy(); _Event = 0; _EventQueue = 0; StartTime = 0; IndexValue = 0;};

		const std::string GetIdentification();

		bool IsActive ();
		bool Destroy ();

		long long		StartTime;
		long long		IndexValue;
		const Event 	*_Event;
	private:

		std::string MD5String (long long CurrentTime, long long StartTime, int EventAddress);

		void 			*_EventQueue;
		std::string		_md5;
};

class EventHandlerFunctionBase {
	public:
		virtual ~EventHandlerFunctionBase() {};
		void exec(const Event* event) {call(event);}

	private:
		virtual void call(const Event*) = 0;
};

template <class T, class EventT>
class MemberFunctionHandler : public EventHandlerFunctionBase {
	public:
		typedef void (T::*MemberFunc)(EventT*);
		MemberFunctionHandler(T* instance, MemberFunc memFn) : _instance(instance), _function(memFn) {};

		void call(const Event* event) {
			(_instance->*_function)(static_cast<EventT*>(event));
		}

	private:
		T* _instance;
		MemberFunc _function;
};

class EventHandlerSingleton {
	public:
		static EventHandlerSingleton& GetInstance();

		typedef std::map<TypeInfo, EventHandlerFunctionBase*> Handlers;
		typedef std::vector<const Event*> EventQueue;
		typedef std::map<long long, TimeEvent*> EventTimeQueue;

		Handlers _handlers;
		EventQueue _queue;
		EventTimeQueue _timeQueue;
		sem_t _sem_wait_lock;
		pthread_mutex_t _mutex_task_lock;
		bool _lifecyle_run;

		// KeyEvent Thread
		pthread_t _key_thread;
		pthread_mutex_t _mutex_key_thread_lock;
		volatile int _key_thread_lifecycle_stop;
		Keyboard::KEYMAP _received_key;
		KEY_MAP _keymap;
		ACTION_MAP _actionmap;

		// TimeCritical Thread
		pthread_t _time_thread;
		pthread_mutex_t _mutex_time_thread_lock;
		volatile int _time_thread_lifecycle_stop;
		sem_t _time_sem_wait_lock;

		struct termios oldtio;

		void MapKeys ();
		bool StartKeyThread ();
		bool StopKeyThread ();
		static void *RunInputThread (void *self);

		bool IsActionKey (int Key);

		bool StartTimeThread ();
		bool StopTimeThread ();
		static void *RunTimeThread (void *self);

		void DeleteAllEvents ();

		static long long GetMillisecondTime ();
		bool AddToQueue (const Event *_Event);
		const Event *PopFromQueue ();

		bool IsActiveTimeEvent (const std::string Identifier);
		bool RemoveTimeEvent(const std::string Identifier, bool All = false);
		const std::string AddToTimeQueue (const Event *_Event, long long Timecritical);
		TimeEvent* GetFromTimeQueue ();
		TimeEvent* PopFromTimeQueue ();
		bool RemoveFromTimeQueue (TimeEvent *_TEvent);
		bool RemoveFirstFromTimeQueue ();

		~EventHandlerSingleton();

	private:
		EventHandlerSingleton ();
		EventHandlerSingleton (const EventHandlerSingleton& EventSingleton);
		EventHandlerSingleton& operator = (const EventHandlerSingleton&);

	protected:

};

class EventHandler {
	friend class EventHandlerSingleton;
	public:
		EventHandler();
		~EventHandler();
		void SendEvent (const Event *event);
		const std::string SendEvent (const Event *_Event, long long Timecritical);

		bool EventActive (const std::string Identifier);
		bool RemoveActiveEvent (const std::string Identifier, bool All = false);

		template <class T, class EventT>
		void RegisterEventFunc(T*, void (T::*memFn)(EventT*));

		template <class T, class EventT>
		bool UnRegisterEventFunc(T*, void (T::*memFn)(EventT*));

		int RunEventHandling();
		void ReceiveSystemEvent (const SystemEvent *Event);

	private:
		int _exit;
		EventHandlerSingleton *_EventHandlerData;

		void handleEvent(const Event*);

		const std::string SendForceEvent (const Event *_Event, long long Timecritical);

};

template <class T, class EventT>
void EventHandler::RegisterEventFunc(T* obj, void (T::*memFn)(EventT*)) {
	_EventHandlerData->_handlers[TypeInfo(typeid(EventT))] = new MemberFunctionHandler<T, EventT>(obj, memFn);
}

template <class T, class EventT>
bool EventHandler::UnRegisterEventFunc(T*, void (T::*memFn)(EventT*)) {
	EventHandlerSingleton::Handlers::iterator it = _EventHandlerData->_handlers.find(TypeInfo(typeid(EventT)));
	if (it != _EventHandlerData->_handlers.end()){
		_EventHandlerData->_handlers.erase(it);
		return true;
	}
	return false;
}

#endif /* EVENTHANDLER_H_ */
