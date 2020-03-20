#pragma once

#include "UI/GUIbase.h"

class IMainApp {
public:
	virtual bool hexKeyNowCallback(int key) { return false; };
	virtual bool hexMouseButtonNowCallback(int button) { return false; }
	virtual void addGameWindow(CGUIbase* gameWin) = 0;
};