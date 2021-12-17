#pragma once

#include <string>
#include <functional>
#include <memory>

#include <glm\glm.hpp>

#include "windows/smartWin.h"

/** Library for creating and using game-specialised GUI controls. */

//class TWinPlugin;
namespace gWin {

	void createWin(const std::string& winName, int x, int y, int w, int h);
	void update(float dT);
	void addText(const std::string& winName, const std::string& text);
	void clearText(const std::string& winName);
	void setDefaultFont(const std::string& winName, const std::string& fontName);
	void setColour(const std::string& winName, glm::vec4& colour);
	void hideWin(const std::string& winName);
	void showWin(const std::string& winName);
	//void setHotTextMouseoverHandler(const std::string& winName, std::function<void(const std::string&) > handler);
	//void setHotTextClickHandler(const std::string& winName, std::function<void(const std::string&) > handler);
	void alignWithMouse(const std::string& winName);
	void centreOnMouse(const std::string& winName);
	void putLeftOf(const std::string& winName, const std::string& parentWin);
	void timeOut(const std::string& winName, float time);
	void hideOnMouseOff(const std::string& winName);
	void setPlugin(const std::string& winName, TWinPlugin plugin);
	void msg(const std::string& winName, const std::string& msg);
}
