/*
= * 	EventHandler.cpp
 *
 * 		Created on: 14.07.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EventHandler.h"

#include <iostream>

#include <ctime>
#include <sys/time.h>

#include <signal.h>

#include <thread>
#include <chrono>

#include "../../Additionals/md5/md5.h"


TimeEvent::TimeEvent(long long WaitTime, const Event *_Event, void *EventQueue) : StartTime(0), IndexValue(0), _Event(_Event), _EventQueue(EventQueue) {
	volatile long long CurrentTime = EventHandlerSingleton::GetMillisecondTime();
	StartTime = CurrentTime + WaitTime;

	_md5 = Crypt::md5(MD5String(CurrentTime, StartTime, (int)&_Event)).hexdigest();
}

const std::string TimeEvent::GetIdentification() {
	const std::string a = _md5;
	return a;
}

bool TimeEvent::IsActive () {
	if (_EventQueue != 0) {
		EventHandlerSingleton::EventTimeQueue *TimeQueue = (EventHandlerSingleton::EventTimeQueue*)_EventQueue;
		EventHandlerSingleton::EventTimeQueue::iterator it = TimeQueue->find(IndexValue);
		if (it != TimeQueue->end()) {
			if (it->second == this) {
				return true;
			}
		}
	}
	return false;
}

bool TimeEvent::Destroy () {
	if (_EventQueue != 0) {
		EventHandlerSingleton::EventTimeQueue *TimeQueue = (EventHandlerSingleton::EventTimeQueue*)_EventQueue;
		EventHandlerSingleton::EventTimeQueue::iterator it = TimeQueue->find(IndexValue);
		if (it != TimeQueue->end()) {
			TimeQueue->erase(it);
			return true;
		}
	}
	return false;
}

std::string TimeEvent::MD5String (long long CurrentTime, long long StartTime, int EventAddress) {
	std::string md5_string = to_string(CurrentTime);
	md5_string.append(to_string(StartTime));
	md5_string.append(to_string(EventAddress));
	return md5_string;
}

EventHandlerSingleton& EventHandlerSingleton::GetInstance() {
	static EventHandlerSingleton _instance;

	return _instance;
}

EventHandlerSingleton::EventHandlerSingleton () : _lifecyle_run(false), _key_thread(0), _key_thread_lifecycle_stop(0), _time_thread(0), _time_thread_lifecycle_stop(0) {
	sem_init(&_sem_wait_lock, 0, 0);
	pthread_mutex_init(&_mutex_task_lock, NULL);
	pthread_mutex_init(&_mutex_key_thread_lock, NULL);

	sem_init(&_time_sem_wait_lock, 0, 0);
	pthread_mutex_init(&_mutex_time_thread_lock, NULL);

	MapKeys();
	StartKeyThread();
	StartTimeThread();

	AddToQueue(new SystemEvent(SystemEventTypeStartup));
}

EventHandlerSingleton::~EventHandlerSingleton () {
	StopKeyThread();
	StopTimeThread();

	sem_destroy(&_sem_wait_lock);
	pthread_mutex_destroy(&_mutex_task_lock);
	pthread_mutex_destroy(&_mutex_key_thread_lock);

	sem_destroy(&_time_sem_wait_lock);
	pthread_mutex_destroy(&_mutex_time_thread_lock);

	EventHandlerSingleton::Handlers::iterator it = _handlers.begin();
	while(it != _handlers.end()) {
		delete it->second;
		++it;
	}
	_handlers.clear();
}

void EventHandlerSingleton::MapKeys () {

	_actionmap.push_back(27);
	_actionmap.push_back(127);
	_actionmap.push_back(32);
	_actionmap.push_back(10);
	_actionmap.push_back(9);
	_actionmap.push_back(47);
	_actionmap.push_back(42);
	_actionmap.push_back(45);
	_actionmap.push_back(95);
	_actionmap.push_back(43);
	_actionmap.push_back(44);
	_actionmap.push_back(46);
	_actionmap.push_back(35);
	_actionmap.push_back(58);
	_actionmap.push_back(59);

	_keymap[KEY_F1]					=	Keyboard::KEYMAP(	27,		91,		91, 	65,		0,		""/*"F1"*/);
	_keymap[KEY_F2]					=	Keyboard::KEYMAP(	27,		91,		91,		66,		0,		""/*"F2"*/);
	_keymap[KEY_F3]					=	Keyboard::KEYMAP(	27,		91,		91,		67,		0,		""/*"F3"*/);
	_keymap[KEY_F4]					=	Keyboard::KEYMAP(	27,		91,		91,		68,		0,		""/*"F4"*/);
	_keymap[KEY_F5]					=	Keyboard::KEYMAP(	27,		91,		91,		69,		0,		""/*"F5"*/);
	_keymap[KEY_F6]					=	Keyboard::KEYMAP(	27,		91,		49,		55,		126,	""/*"F6"*/);
	_keymap[KEY_F7]					=	Keyboard::KEYMAP(	27,		91,		49,		56,		126,	""/*"F7"*/);
	_keymap[KEY_F8]					=	Keyboard::KEYMAP(	27,		91,		49,		57,		126,	""/*"F8"*/);
	_keymap[KEY_F9]					=	Keyboard::KEYMAP(	27,		91,		50,		48,		126,	""/*"F9"*/);
	_keymap[KEY_F10]				=	Keyboard::KEYMAP(	27,		91,		50,		49,		126,	""/*"F10"*/);
	_keymap[KEY_F11]				=	Keyboard::KEYMAP(	27,		91,		50,		51,		126,	""/*"F11"*/);
	_keymap[KEY_F12]				=	Keyboard::KEYMAP(	27,		91,		50,		52,		126,	""/*"F12"*/);

	_keymap[KEY_0]					=	Keyboard::KEYMAP(	48,		0,		0,		0,		0,		"0");
	_keymap[KEY_1]					=	Keyboard::KEYMAP(	49,		0,		0,		0,		0,		"1");
	_keymap[KEY_2]					=	Keyboard::KEYMAP(	50,		0,		0,		0,		0,		"2");
	_keymap[KEY_3]					=	Keyboard::KEYMAP(	51,		0,		0,		0,		0,		"3");
	_keymap[KEY_4]					=	Keyboard::KEYMAP(	52,		0,		0,		0,		0,		"4");
	_keymap[KEY_5]					=	Keyboard::KEYMAP(	53,		0,		0,		0,		0,		"5");
	_keymap[KEY_6]					=	Keyboard::KEYMAP(	54,		0,		0,		0,		0,		"6");
	_keymap[KEY_7]					=	Keyboard::KEYMAP(	55,		0,		0,		0,		0,		"7");
	_keymap[KEY_8]					=	Keyboard::KEYMAP(	56,		0,		0,		0,		0,		"8");
	_keymap[KEY_9]					=	Keyboard::KEYMAP(	57,		0,		0,		0,		0,		"9");

	_keymap[KEY_EQUAL]				=	Keyboard::KEYMAP(	61,		0,		0,		0,		0,		"=");

	_keymap[KEY_A]					=	Keyboard::KEYMAP(	65,		0,		0,		0,		0,		"A");
	_keymap[KEY_B]					=	Keyboard::KEYMAP(	66,		0,		0,		0,		0,		"B");
	_keymap[KEY_C]					=	Keyboard::KEYMAP(	67,		0,		0,		0,		0,		"C");
	_keymap[KEY_D]					=	Keyboard::KEYMAP(	68,		0,		0,		0,		0,		"D");
	_keymap[KEY_E]					=	Keyboard::KEYMAP(	69,		0,		0,		0,		0,		"E");
	_keymap[KEY_F]					=	Keyboard::KEYMAP(	70,		0,		0,		0,		0,		"F");
	_keymap[KEY_G]					=	Keyboard::KEYMAP(	71,		0,		0,		0,		0,		"G");
	_keymap[KEY_H]					=	Keyboard::KEYMAP(	72,		0,		0,		0,		0,		"H");
	_keymap[KEY_I]					=	Keyboard::KEYMAP(	73,		0,		0,		0,		0,		"I");
	_keymap[KEY_J]					=	Keyboard::KEYMAP(	74,		0,		0,		0,		0,		"J");
	_keymap[KEY_K]					=	Keyboard::KEYMAP(	75,		0,		0,		0,		0,		"K");
	_keymap[KEY_L]					=	Keyboard::KEYMAP(	76,		0,		0,		0,		0,		"L");
	_keymap[KEY_M]					=	Keyboard::KEYMAP(	77,		0,		0,		0,		0,		"M");
	_keymap[KEY_N]					=	Keyboard::KEYMAP(	78,		0,		0,		0,		0,		"N");
	_keymap[KEY_O]					=	Keyboard::KEYMAP(	79,		0,		0,		0,		0,		"O");
	_keymap[KEY_P]					=	Keyboard::KEYMAP(	80,		0,		0,		0,		0,		"P");
	_keymap[KEY_Q]					=	Keyboard::KEYMAP(	81,		0,		0,		0,		0,		"Q");
	_keymap[KEY_R]					=	Keyboard::KEYMAP(	82,		0,		0,		0,		0,		"R");
	_keymap[KEY_S]					=	Keyboard::KEYMAP(	83,		0,		0,		0,		0,		"S");
	_keymap[KEY_T]					=	Keyboard::KEYMAP(	84,		0,		0,		0,		0,		"T");
	_keymap[KEY_U]					=	Keyboard::KEYMAP(	85,		0,		0,		0,		0,		"U");
	_keymap[KEY_V]					=	Keyboard::KEYMAP(	86,		0,		0,		0,		0,		"V");
	_keymap[KEY_W]					=	Keyboard::KEYMAP(	87,		0,		0,		0,		0,		"W");
	_keymap[KEY_X]					=	Keyboard::KEYMAP(	88,		0,		0,		0,		0,		"X");
	_keymap[KEY_Y]					=	Keyboard::KEYMAP(	89,		0,		0,		0,		0,		"Y");
	_keymap[KEY_Z]					=	Keyboard::KEYMAP(	90,		0,		0,		0,		0,		"Z");

	_keymap[KEY_a]					=	Keyboard::KEYMAP(	97,		0,		0,		0,		0,		"a");
	_keymap[KEY_b]					=	Keyboard::KEYMAP(	98,		0,		0,		0,		0,		"b");
	_keymap[KEY_c]					=	Keyboard::KEYMAP(	99,		0,		0,		0,		0,		"c");
	_keymap[KEY_d]					=	Keyboard::KEYMAP(	100,	0,		0,		0,		0,		"d");
	_keymap[KEY_e]					=	Keyboard::KEYMAP(	101,	0,		0,		0,		0,		"e");
	_keymap[KEY_f]					=	Keyboard::KEYMAP(	102,	0,		0,		0,		0,		"f");
	_keymap[KEY_g]					=	Keyboard::KEYMAP(	103,	0,		0,		0,		0,		"g");
	_keymap[KEY_h]					=	Keyboard::KEYMAP(	104,	0,		0,		0,		0,		"h");
	_keymap[KEY_i]					=	Keyboard::KEYMAP(	105,	0,		0,		0,		0,		"i");
	_keymap[KEY_j]					=	Keyboard::KEYMAP(	106,	0,		0,		0,		0,		"j");
	_keymap[KEY_k]					=	Keyboard::KEYMAP(	107,	0,		0,		0,		0,		"k");
	_keymap[KEY_l]					=	Keyboard::KEYMAP(	108,	0,		0,		0,		0,		"l");
	_keymap[KEY_m]					=	Keyboard::KEYMAP(	109,	0,		0,		0,		0,		"m");
	_keymap[KEY_n]					=	Keyboard::KEYMAP(	110,	0,		0,		0,		0,		"n");
	_keymap[KEY_o]					=	Keyboard::KEYMAP(	111,	0,		0,		0,		0,		"o");
	_keymap[KEY_p]					=	Keyboard::KEYMAP(	112,	0,		0,		0,		0,		"p");
	_keymap[KEY_q]					=	Keyboard::KEYMAP(	113,	0,		0,		0,		0,		"q");
	_keymap[KEY_r]					=	Keyboard::KEYMAP(	114,	0,		0,		0,		0,		"r");
	_keymap[KEY_s]					=	Keyboard::KEYMAP(	115,	0,		0,		0,		0,		"s");
	_keymap[KEY_t]					=	Keyboard::KEYMAP(	116,	0,		0,		0,		0,		"t");
	_keymap[KEY_u]					=	Keyboard::KEYMAP(	117,	0,		0,		0,		0,		"u");
	_keymap[KEY_v]					=	Keyboard::KEYMAP(	118,	0,		0,		0,		0,		"v");
	_keymap[KEY_w]					=	Keyboard::KEYMAP(	119,	0,		0,		0,		0,		"w");
	_keymap[KEY_x]					=	Keyboard::KEYMAP(	120,	0,		0,		0,		0,		"x");
	_keymap[KEY_y]					=	Keyboard::KEYMAP(	121,	0,		0,		0,		0,		"y");
	_keymap[KEY_z]					=	Keyboard::KEYMAP(	122,	0,		0,		0,		0,		"z");

	_keymap[KEY_ACTION_ENTER]		=	Keyboard::KEYMAP(	10,		0,		0,		0,		0,		""/*"ENTER"*/);
	_keymap[KEY_ACTION_BACKSPACE]	=	Keyboard::KEYMAP(	127,	0,		0,		0,		0,		""/*"BACKSPACE"*/);
	_keymap[KEY_ACTION_SPACE]		=	Keyboard::KEYMAP(	32,		0,		0,		0,		0,		" ");
	_keymap[KEY_ACTION_UARROW]		=	Keyboard::KEYMAP(	27,		91,		65,		0,		0,		""/*"ARROW_UP"*/);
	_keymap[KEY_ACTION_DARROW]		=	Keyboard::KEYMAP(	27,		91,		66,		0,		0,		""/*"ARROW_DOWN"*/);
	_keymap[KEY_ACTION_RARROW]		=	Keyboard::KEYMAP(	27,		91,		67,		0,		0,		""/*"ARROW_RIGHT"*/);
	_keymap[KEY_ACTION_LARROW]		=	Keyboard::KEYMAP(	27,		91,		68,		0,		0,		""/*"ARROW_LEFT"*/);

	_keymap[KEY_ACTION_PASTE]		=	Keyboard::KEYMAP(	27,		91,		50,		126,	0,		""/*"PASTE"*/);
	_keymap[KEY_ACTION_ERASE]		=	Keyboard::KEYMAP(	27,		91,		51,		126,	0,		""/*"ERASE"*/);
	_keymap[KEY_ACTION_POS1]		=	Keyboard::KEYMAP(	27,		91,		49,		126,	0,		""/*"POS1"*/);
	_keymap[KEY_ACTION_END]			=	Keyboard::KEYMAP(	27,		91,		52,		126,	0,		""/*"END"*/);
	_keymap[KEY_ACTION_PICUP]		=	Keyboard::KEYMAP(	27,		91,		53,		126,	0,		""/*"PICUP"*/);
	_keymap[KEY_ACTION_PICDOWN]		=	Keyboard::KEYMAP(	27,		91,		54,		126,	0,		""/*"PICDOWN"*/);
	_keymap[KEY_ACTION_PAUSE]		=	Keyboard::KEYMAP(	27,		91,		80,		0,		0,		""/*"PAUSE"*/);
	_keymap[KEY_ACTION_TAB]			=	Keyboard::KEYMAP(	9,		0,		0,		0,		0,		""/*"TAB"*/);
	_keymap[KEY_ACTION_FACTOR]		=	Keyboard::KEYMAP(	47,		0,		0,		0,		0,		"/");
	_keymap[KEY_ACTION_MULTIPLIER]	=	Keyboard::KEYMAP(	42,		0,		0,		0,		0,		"*");
	_keymap[KEY_ACTION_MINUS]		=	Keyboard::KEYMAP(	45,		0,		0,		0,		0,		"-");
	_keymap[KEY_ACTION_UNDERSCORE]	=	Keyboard::KEYMAP(	95,		0,		0,		0,		0,		"_");
	_keymap[KEY_ACTION_PLUS]		=	Keyboard::KEYMAP(	43,		0,		0,		0,		0,		"+");
	_keymap[KEY_ACTION_COMMA]		=	Keyboard::KEYMAP(	44,		0,		0,		0,		0,		",");
	_keymap[KEY_ACTION_POINT]		=	Keyboard::KEYMAP(	46,		0,		0,		0,		0,		".");
	_keymap[KEY_ACTION_NUMBERSIGN]	=	Keyboard::KEYMAP(	35,		0,		0,		0,		0,		"#");
	_keymap[KEY_ACTION_COLON]		=	Keyboard::KEYMAP(	58,		0,		0,		0,		0,		":");
	_keymap[KEY_ACTION_SEMICOLON]	=	Keyboard::KEYMAP(	59,		0,		0,		0,		0,		";");
}

bool EventHandlerSingleton::StartKeyThread () {
	if (_key_thread == 0) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		int error = pthread_create(&_key_thread, NULL, &RunInputThread, (void*)this);

		pthread_attr_destroy(&attr);

		if (error == 0) {
			return true;
		}
	}
	return false;
}

bool EventHandlerSingleton::StopKeyThread () {
	if (_key_thread != 0) {
		_key_thread_lifecycle_stop = 1;

		//pthread_mutex_unlock(&_mutex_task_wait);

		int error = pthread_kill(_key_thread, 0);
		//int error = pthread_join(_key_thread, &status);

		// Reconfig
		int er = tcsetattr (0, TCSAFLUSH, &oldtio);
		if (er) {
			return 0;
		}

		if (error == 0) {
			_key_thread = 0;
			_key_thread_lifecycle_stop = 0;
			return true;
		}
	}
	return false;
}

void *EventHandlerSingleton::RunInputThread (void *self) {


	// Instance
	EventHandlerSingleton *EventHandler = (EventHandlerSingleton*)self;


	// Config
	int key = -1;

	struct termios newtio;
	int er;

	er = tcgetattr (0, &EventHandler->oldtio);
	if (er) {
		return 0;
	}

	memcpy((void*)&newtio, (void*)&EventHandler->oldtio, sizeof(EventHandler->oldtio));
	newtio.c_lflag &= ~(ICANON | ECHO );
	er = tcsetattr (0, TCSAFLUSH, &newtio);
	if (er) {
		return 0;
	}


	// Lifecyle
	Keyboard::KEYMAP _tmp_key;
	bool _actionKeyMode = false;
	while (EventHandler->_key_thread_lifecycle_stop == 0) {
		key = getchar();

		if (_actionKeyMode == false) {
			if (EventHandler->IsActionKey(key)) {
				_actionKeyMode = true;
			} else {
				_tmp_key.RESET();
			}
		}

#ifdef RAW_TEST
		_tmp_key.RESET();
		_tmp_key.INSERT(27);
		_tmp_key.INSERT(91);
		_tmp_key.INSERT(67);
		_tmp_key.INSERT(0);
		key = 0;
#endif

#ifdef OUTPUT_RAW
		std::string out = "tmpKey Size: ";
		out.append(to_string(_tmp_key.SIZE()));
		out.append(" key: ");
		out.append(to_string(key));
		out.append("\n");
		printf(out.c_str());
#endif

		if (_tmp_key.INSERT(key)) {
			KEY_MAP::iterator it2 = EventHandler->_keymap.begin();
			while (it2 != EventHandler->_keymap.end()) {
				Keyboard::KEYMAP map2 = it2->second;
				if (map2 == _tmp_key) {
					EventHandler->AddToQueue(new KeyEvent(KeyEventTypePressed, it2->first, _actionKeyMode, map2.SYMBOL()));
					_actionKeyMode = false;
					EventHandler->_received_key.RESET();
					EventHandler->_received_key = _tmp_key;
					_tmp_key.RESET();
					break;
				}
				it2++;
			}
		}

		if (_tmp_key.FULL()) {
			_tmp_key.RESET();
		}

	}

	er = tcsetattr (0, TCSAFLUSH, &EventHandler->oldtio);
	if (er) {
		return 0;
	}

	return 0;
}

bool EventHandlerSingleton::IsActionKey (int Key) {
	ACTION_MAP::iterator it = _actionmap.begin();
	while (it != _actionmap.end()) {
		if ((*it) == Key) {
			return true;
		}
		it++;
	}
	return false;
}

bool EventHandlerSingleton::StartTimeThread () {
	if (_time_thread == 0) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		int error = pthread_create(&_time_thread, NULL, &RunTimeThread, (void*)this);

		pthread_attr_destroy(&attr);

		if (error == 0) {
			return true;
		}
	}
	return false;
}

bool EventHandlerSingleton::StopTimeThread () {
	if (_time_thread != 0) {
		_time_thread_lifecycle_stop = 1;

		int error = pthread_kill(_time_thread, 0);

		if (error == 0) {
			_time_thread = 0;
			_time_thread_lifecycle_stop = 0;
			return true;
		}
	}
	return false;
}

void *EventHandlerSingleton::RunTimeThread (void *self) {

	// Instance
	EventHandlerSingleton *EventHandler = (EventHandlerSingleton*)self;

	while (EventHandler->_time_thread_lifecycle_stop == 0) {
		sem_wait(&EventHandler->_time_sem_wait_lock);
		int TimeRun = false;
		while (TimeRun == false) {
			TimeEvent *TEvent = EventHandler->GetFromTimeQueue();
			if (TEvent != 0) {
				volatile long long CurrentTime = GetMillisecondTime();
				if (CurrentTime >= TEvent->StartTime) {
					EventHandler->AddToQueue(TEvent->_Event);
					TimeRun = true;
					//if (EventHandler->RemoveFromTimeQueue(TEvent)) {
						delete TEvent;
					//}
				}
				if (TimeRun == false) {
					usleep(1000);
				}
			} else {
				TimeRun = true;
			}
		}
	}

	return 0;
}

void EventHandlerSingleton::DeleteAllEvents () {
	pthread_mutex_lock(&_mutex_task_lock);
	pthread_mutex_lock(&_mutex_time_thread_lock);
	_queue.clear();
	_timeQueue.clear();
	pthread_mutex_unlock(&_mutex_task_lock);
	pthread_mutex_unlock(&_mutex_time_thread_lock);
}

long long EventHandlerSingleton::GetMillisecondTime () {
	struct timeval  te;
	gettimeofday(&te, 0);

	return te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
}

bool EventHandlerSingleton::AddToQueue (const Event *_Event) {

	pthread_mutex_lock(&_mutex_task_lock);
	_queue.insert(_queue.begin(), _Event);
#ifdef EVENT_DEBUG_OUTPUT
	//_Event->DebugInfo();
#endif
	pthread_mutex_unlock(&_mutex_task_lock);

	sem_post(&_sem_wait_lock);

	return true;
}

const Event* EventHandlerSingleton::PopFromQueue () {

	const Event *Event = 0;

	pthread_mutex_lock(&_mutex_task_lock);
	if (_queue.size() > 0) {
		Event = _queue.back();
		_queue.pop_back();
	}
	pthread_mutex_unlock(&_mutex_task_lock);

	return Event;
}

bool EventHandlerSingleton::IsActiveTimeEvent (const std::string Identifier) {

	pthread_mutex_lock(&_mutex_time_thread_lock);
	if (_timeQueue.size() > 0) {
		EventTimeQueue::iterator it = _timeQueue.begin();
		while (it != _timeQueue.end()) {
			if (it->second->GetIdentification() == Identifier) {
				pthread_mutex_unlock(&_mutex_time_thread_lock);
				return true;
			}
			it++;
		}
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return false;
}

bool EventHandlerSingleton::RemoveTimeEvent(const std::string Identifier, bool All) {

	pthread_mutex_lock(&_mutex_time_thread_lock);
	if (_timeQueue.size() > 0) {
		EventTimeQueue::iterator it = _timeQueue.begin();
		while (it != _timeQueue.end()) {
			if (it->second->GetIdentification() == Identifier) {
				_timeQueue.erase(it);
				if (All == true) {
					pthread_mutex_unlock(&_mutex_time_thread_lock);
					return true;
				}
			}
			it++;
		}
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return false;
}

const std::string EventHandlerSingleton::AddToTimeQueue (const Event *_Event, long long Timecritical) {

	pthread_mutex_lock(&_mutex_time_thread_lock);
	TimeEvent *_TEvent = new TimeEvent(Timecritical, _Event, &_timeQueue);

	// Sicherstellen das jeder Index nur einmal vorhanden ist. Folgenden Events wird eine Millisekunde addiert
	bool Valid = false;
	long long TimeOffset = 0;
	while (Valid == false) {
		EventTimeQueue::iterator it = _timeQueue.find(_TEvent->StartTime + TimeOffset);
		if (it == _timeQueue.end()) {
			_TEvent->IndexValue = _TEvent->StartTime + TimeOffset;
			_timeQueue[_TEvent->IndexValue] = _TEvent;
			Valid = true;
		}
		TimeOffset++;
	}


#ifdef EVENT_DEBUG_OUTPUT
	//_Event->DebugInfo();
#endif

	pthread_mutex_unlock(&_mutex_time_thread_lock);

	sem_post(&_time_sem_wait_lock);

	return _TEvent->GetIdentification();
}

TimeEvent* EventHandlerSingleton::GetFromTimeQueue () {

	TimeEvent *TEvent = 0;

	pthread_mutex_lock(&_mutex_time_thread_lock);
	if (_timeQueue.size() > 0) {
		TEvent = _timeQueue.begin()->second;
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return TEvent;
}

TimeEvent* EventHandlerSingleton::PopFromTimeQueue () {

	TimeEvent *TEvent = 0;

	pthread_mutex_lock(&_mutex_time_thread_lock);
	if (_timeQueue.size() > 0) {
		TEvent = _timeQueue.begin()->second;
		_timeQueue.erase(_timeQueue.begin());
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return TEvent;
}

bool EventHandlerSingleton::RemoveFromTimeQueue (TimeEvent *_TEvent) {

	pthread_mutex_lock(&_mutex_time_thread_lock);
	EventTimeQueue::iterator it = _timeQueue.find(_TEvent->IndexValue);
	if (_timeQueue.size() > 0 && it != _timeQueue.end()) {
		_timeQueue.erase(it);
		pthread_mutex_unlock(&_mutex_time_thread_lock);
		return true;
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return false;
}

bool EventHandlerSingleton::RemoveFirstFromTimeQueue () {

	pthread_mutex_lock(&_mutex_time_thread_lock);
	if (_timeQueue.size() > 0) {
		_timeQueue.erase(_timeQueue.begin());
		pthread_mutex_unlock(&_mutex_time_thread_lock);
		return true;
	}
	pthread_mutex_unlock(&_mutex_time_thread_lock);

	return false;
}

/*void EventHandlerSingleton::PrintLog (const Event *_Event) {
	std::string out("EventLog: ");
	out.append("EventType: ");

	//std::string debug =
			_Event->DebugInfo();
	out.append(debug);

	if (dynamic_cast<const SIP::SIPCallEvent*> (_Event) != NULL) {
		out.append("SIPCallEvent");
	}
}*/

EventHandler::EventHandler () : _exit(0), _EventHandlerData(0) {
	_EventHandlerData = &EventHandlerSingleton::GetInstance();
}


EventHandler::~EventHandler() {
	// TODO Auto-generated destructor stub

}


// Event senden
void EventHandler::SendEvent (const Event *_Event) {
	if (_exit == false) {
		_EventHandlerData->AddToQueue(_Event);
	}
}

// Zeitgesteuertes Event senden
const std::string EventHandler::SendEvent (const Event *_Event, long long Timecritical) {
	if (_exit == false) {
		return this->SendForceEvent(_Event, Timecritical);
	}
	return "0";
}

// Zeitgesteuertes Event senden (Auch wenn der Exit schon eingeleitet ist. Wichtig um das Programm zu beenden)
const std::string EventHandler::SendForceEvent (const Event *_Event, long long Timecritical) {
	return _EventHandlerData->AddToTimeQueue(_Event, Timecritical);
}

// Überpüfen ob ein Event mit einem bestimmten Identifier in der Zeit-Queue ist.
bool EventHandler::EventActive (const std::string Identifier) {
	return _EventHandlerData->IsActiveTimeEvent(Identifier);
}

// Event mit angegebenen Identifier wird gelöscht. (Optionaler Paramter: true = Alle werden gelöscht, false = Nur das erste wird gelöscht)
bool EventHandler::RemoveActiveEvent (const std::string Identifier, bool All) {
	return _EventHandlerData->RemoveTimeEvent(Identifier, All);
}

void EventHandler::handleEvent(const Event *_Event) {
	EventHandlerSingleton::Handlers::iterator it = _EventHandlerData->_handlers.find(TypeInfo(typeid(*_Event)));
	if(it != _EventHandlerData->_handlers.end()) {
		it->second->exec(_Event);
	}
}

// Haupt Event Routine, kann nur einmal gestartet werden. Wird so lange ausgeführt bis ein System::SystemEventTypeExit Event eintrifft.
int EventHandler::RunEventHandling() {
	int ReturnValue = 0;
	if (_EventHandlerData->_lifecyle_run == false) {
		_EventHandlerData->_lifecyle_run = true;
		const SystemEvent* SEvent = NULL;
		while (!_exit) {
			sem_wait(&_EventHandlerData->_sem_wait_lock);
			const Event *event = _EventHandlerData->PopFromQueue();
			if ((SEvent = dynamic_cast<const SystemEvent*> (event)) != NULL) {
				ReceiveSystemEvent(SEvent);
			}
			handleEvent(event);
		}
		_EventHandlerData->_lifecyle_run = false;
	}

	_EventHandlerData->DeleteAllEvents();
	_EventHandlerData->StopKeyThread();
	_EventHandlerData->StopTimeThread();

	return ReturnValue;
}

void EventHandler::ReceiveSystemEvent (const SystemEvent *Event) {
	if (Event->_EventType == SystemEventTypeExit) {
		_exit = true;
		SendForceEvent(new SystemEvent(SystemEventTypeShutdown), 0);
	}
}
