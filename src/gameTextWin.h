#pragma once

#include <string>

#include "gameWin.h"
#include "UI/GUIrichText.h"

/** A window consisting of a rich text control. */
class CGameTextWin : public CGameWin {
public:
	CGameTextWin();
	void addText(const std::string& text);
	//void setTheme(const std::string& themeName);
	//void setStyle(const std::string& styleName);
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

	void setHotTextMouseoverHandler(std::function<void(const std::string&)> handler) {
		richText->setHotTextMouseoverHandler(handler);
	}

	void setHotTextClickHandler(std::function<void(const std::string&)> handler) {
		richText->setHotTextClickHandler(handler);
	}

	void update(float dT);


	float timeOut = 0;
	

private:
	CGUIrichText* richText = NULL;
	glm::i32vec2 totalBorderSize; ///<Total space taken up by border furniture.



};