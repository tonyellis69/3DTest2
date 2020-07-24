#pragma once

#include "messaging/events.h"

class CGameEventObserver : public CEventObserver {
public:

	virtual void onNotify(COnNewHex& msg) {};
	virtual void onNotify(CMouseExitHex& msg) {};
	virtual void onNotify(CPlayerNewHex& msg) {};
};


/** Maintains a list of observers and sends events to them. */
class CGameEventSubject {
public:
	void subscribe(CGameEventObserver* observer) {
		gameObservers.push_back(observer);
	}

	template <typename T>
	void notify(T& msg) {

		for (int b = 0; b < gameObservers.size(); b++ ) {
			gameObservers[b]->onNotify(msg);			
		}
	}

private:
	std::vector<CGameEventObserver*> gameObservers;
};