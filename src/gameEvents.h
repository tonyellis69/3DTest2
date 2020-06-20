#pragma once

#include "messaging/events.h"

class CGameEventObserver : public CEventObserver {
public:

	virtual void onNotify(COnNewHex& msg) {};
	virtual void onNotify(CMouseExitHex& msg) {};
};


/** Maintains a list of observers and sends events to them. */
class CGameEventSubject {
public:
	void subscribe(CGameEventObserver* observer) {
		gameObservers.push_back(observer);
	}

	template <typename T>
	void notify(T& msg) {
		for (auto observer : gameObservers) {
			observer->onNotify(msg);
		}
	}

private:
	std::vector<CGameEventObserver*> gameObservers;
};