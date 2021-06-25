#pragma once

#include <string>
#include <glm\glm.hpp>

/** Library for creating and using game-specialised GUI controls. */

namespace gWin {

	void createWin(const std::string& winName, int x, int y, int w, int h);
	void addText(const std::string& winName, const std::string& text);
	void clearText(const std::string& winName);
	void setFont(const std::string& winName, const std::string& fontName);
	void setColour(const std::string& winName, glm::vec4& colour);



}
