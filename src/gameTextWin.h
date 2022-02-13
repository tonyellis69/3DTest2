#pragma once

#include <string>

#include "gameWin.h"
#include "UI/GUIrichText.h"

#include "windows/smartWin.h"



/** A window consisting of a rich text control. */
class CGameTextWin : public CGameWin {
public:
	CGameTextWin();
	void addText(const std::string& text);
	void setDefaultFont(const std::string& fontName);
	void setColour(glm::vec4& colour);
	void clearText();
	bool isEmpty();
	void resizeToFit();
	void message(CGUIbase* sender, CMessage& msg);
	void resize(int w, int h);

	void positionAtMousePointer();
	void positionOffset();

	void onNotify(CMouseExitHex& msg);


	void update(float dT);

	void setPlugin(TWinPlugin plugin) {
		smartPlugin = plugin;
		smartPlugin->setWindow(this);
	}

	//void msg(const std::string& msg);


	float timeOut = 0;
	TWinPlugin smartPlugin = nullptr;

	CGUIrichText* richText = NULL;
	glm::i32vec2 totalBorderSize; ///<Total space taken up by border furniture.



};