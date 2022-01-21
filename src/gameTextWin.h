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

	bool OnMouseMove(const int mouseX, const int mouseY, int key);
	bool onMouseOff(const  int mouseX, const  int mouseY, int key);

	void setHotTextMouseoverHandler(std::function<void(const std::string&)> handler) {
		richText->setHotTextMouseoverHandler(handler);
	}

	void setHotTextClickHandler(std::function<void(const std::string&)> handler) {
		richText->setHotTextClickHandler(handler);
	}

	void update(float dT);

	void setPlugin(TWinPlugin plugin) {
		smartPlugin = plugin;
		smartPlugin->setWindow(this);
	}

	void msg(const std::string& msg);

	void onRichTextMouseover(const std::string& msg);
	void onRichTextClick(const std::string& msg);

	float timeOut = 0;
	TWinPlugin smartPlugin = nullptr;

	CGUIrichText* richText = NULL;
	glm::i32vec2 totalBorderSize; ///<Total space taken up by border furniture.



};