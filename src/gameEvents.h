#pragma once

#include "messaging/events.h"
#include "gameMsg.h"

class CGameEventObserver : public CEventObserver {
public:

	virtual void onNotify(COnCursorNewHex& msg) {};
	virtual void onNotify(CMouseExitHex& msg) {};
	virtual void onNotify(CPlayerNewHex& msg) {};
	virtual void onNotify(CActorMovedHex& msg) {};
};


/** Maintains a list of observers and sends events to them. */
class CGameEventSubject {
public:
	void subscribe(CGameEventObserver* observer) {
		gameObservers.push_back(observer);
	}

	template <typename T>
	void notify(T& msg) {

		for (unsigned int b = 0; b < gameObservers.size(); b++ ) {
			gameObservers[b]->onNotify(msg);			
		}
	}

	void unsubscribe(CGameEventObserver* observer) {
		for (auto obsvr = gameObservers.begin(); obsvr != gameObservers.end();)
			if (*obsvr == observer)
				obsvr = gameObservers.erase(obsvr);
			else
				obsvr++;
	}

private:
	std::vector<CGameEventObserver*> gameObservers;
};