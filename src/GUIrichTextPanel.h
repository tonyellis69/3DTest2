#pragma once

#include "GUIgamePanel.h"
#include "..\3DEngine\src\UI\GUIrichText.h"



/** A composite control that adds a rich text control to a panel, which
	can then be modified, added-to, etc. */
class CGUIrichTextPanel : public CGUIgamePanel {
public:	
	CGUIrichTextPanel(int x, int y, int w, int h);
	void setRichtextInset(int newInset);

	//CFont* getFont();
	//void setTextColour(float r, float g, float b, float a);
	//void setTextColour(UIcolour colour);
	//void setText(std::string newText);
	void clear();
	void setResizeMode(TResizeMode mode);
	void setShortestSpaceBreak(int lineWidth);
	//void setTextStyles(std::vector<TtextStyle>* styles);
	//void setTextTheme(const std::string& themeName) {
	//	richText->setTextTheme(themeName);
	//}
//	bool setTextStyle(std::string styleName);
	void setDefaultTextStyle(std::string styleName);
//	std::vector<unsigned int> purgeHotText(unsigned int id);
	void update(float dT);

	void appendMarkedUpText(std::string text);
	unsigned int getRichTextID();


	bool MouseWheelMsg(const int mouseX, const int mouseY, int wheelDelta, int key);

	bool OnLMouseDown(const int mouseX, const int mouseY, int key);
	bool onMouseOff(const  int mouseX, const  int mouseY, int key);
	

	void resizeToFit();

	void message(CGUIbase* sender, CMessage & msg);

	//bool setTempText(bool onOff);

	//void suspend(bool isOn);
	//bool collapseTempText();
	//bool solidifyTempText();
	//void unhotDuplicates();
	//void removeMarked();

	//bool busy();

	void displayText(std::string text);

	void deliverByClause(float dT);

	void deliverByCharacter(float dT);

	//bool setLineFadeIn(bool onOff) {
	//	if (richText->isBusy())
	//		return false;
	//	//richText->enableLineFadeIn = onOff;
	//	return true;
	//}

	//bool clearToBookMark() {
	//	return richText->clearToBookMark();
	//}

	enum TStatus { initial, readyToPosition, displaying, 
		readyToDelete};
	void setStatus(TStatus newStatus) {
		status = newStatus;
	}
	bool noMouse();



	//bool isDisplayFinished() {
	//	return richText->isDisplayFinished();
	//}

	CGUIrichText* richText; ///<Rich text control.
	int inset; ///<Inset for rich text control within panel.


	glm::i32vec2 lastMousePos; ///<For dragging purposes
	bool dragging; 
	bool draggable;

	//TTextDelivery deliveryMode;
	std::string deliveryBuffer;
	float clauseInterval;
	float clauseDelay;
	float charInterval;
	float charDelay;

	TStatus status;

	int mouseOffMargin; ///<Extra pixels to allow before mouse declared outside panel.

};