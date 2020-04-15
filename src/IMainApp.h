#pragma once

#include "UI/GUIbase.h"

class IMainApp {
public:
	virtual bool hexKeyNowCallback(int key) { return false; }; //TO DO: why not make pure?
	virtual bool hexMouseButtonNowCallback(int button) { return false; }
	virtual void addGameWindow(CGUIbase* gameWin) = 0;
	virtual glm::i32vec2 getMousePos() = 0;
};