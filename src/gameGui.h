#pragma once

#include <string>
#include <functional>
#include <memory>

#include <glm\glm.hpp>

#include "windows/smartWin.h"

/** Library for creating and using game-specialised GUI controls. */

//class TWinPlugin;
namespace gWin {
	void initWindows();
	void createWin(const std::string& winName, int x, int y, int w, int h);
	void update(float dT);
	void setPlugin(const std::string& winName, TWinPlugin plugin);


	extern TWinPlugin pNear;
	extern TWinPlugin pInv;
	extern TWinPlugin pItemMenu;
	extern TWinPlugin pExam;
}
