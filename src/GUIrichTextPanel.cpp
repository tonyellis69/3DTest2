#include "GUIrichTextPanel.h"
#include "..\3DEngine\src\UI\GUIroot.h"

CGUIrichTextPanel::CGUIrichTextPanel(int x, int y, int w, int h) : CGUIgamePanel(x,y,w,h) {
	setLocalPos(x, y);
	drawBorder = false;
	setWidth(w);
	setHeight(h);
	type = uiRichTextPanel;

//	setBackColour1(oldbackColour1);
//	setBackColour2(oldbackColour2);

	inset = 10;
	richText = new CGUIrichText(inset, inset, w - inset*2, h - inset);
	richText->setMultiLine(true);
	richText->anchorRight = inset;
	richText->anchorLeft = inset;
	richText->anchorBottom = inset;
	Add(richText);
	dragging = false;
	draggable = false;

	deliveryMode = noDelivery;
	clauseInterval = 0;
	clauseDelay = 0.2f;
	charInterval = 0;
	charDelay = 0.01f;
	status = initial;
	mouseOffMargin = 20; //TO DO should default to 0
	objId = -1;
}

void CGUIrichTextPanel::setRichtextInset(int newInset) {
	inset = newInset;
	richText->anchorRight = inset;
	richText->anchorLeft = inset;
	richText->anchorBottom = inset;
	needsUpdate = true;
}

void CGUIrichTextPanel::setFont(CFont * newFont) {
	richText->setFont(newFont);
}

CFont * CGUIrichTextPanel::getFont() {
	return richText->getFont();
}

void CGUIrichTextPanel::setTextColour(float r, float g, float b, float a) {
	richText->setTextColour(r, g, b, a);
}

void CGUIrichTextPanel::setTextColour(UIcolour colour) {
	richText->setTextColour(colour);
}


void CGUIrichTextPanel::setText(std::string newText) {
	richText->setText(newText);
}

void CGUIrichTextPanel::appendText(std::string newText) {
	richText->appendText(newText);
}

void CGUIrichTextPanel::clear() {
	richText->clear();
}



void CGUIrichTextPanel::setResizeMode(TResizeMode mode){
	richText->setResizeMode(mode);
}

void CGUIrichTextPanel::setShortestSpaceBreak(int lineWidth) {
	richText->shortestSpaceBreak = lineWidth;
}


/*
void CGUIrichTextPanel::refreshCurrentTextStyles(std::vector<TtextStyle>* styles) {
	richText->refreshCurrentTextStyles(styles);
}*/

bool CGUIrichTextPanel::setTextStyle(std::string styleName) {
	return richText->setTextStyle(styleName);
}

void CGUIrichTextPanel::setDefaultTextStyle(std::string styleName) {
	richText->setDefaultTextStyle(styleName);
}

std::vector<unsigned int> CGUIrichTextPanel::purgeHotText(unsigned int id){
	return richText->purgeHotText(id);
}

void CGUIrichTextPanel::update(float dT) {
	/*if (!deliveryBuffer.empty() && !richText->busy) {
		switch (deliveryMode) {
		case byClause: deliverByClause(dT); break;
		case byCharacter: deliverByCharacter(dT); break;
		}
	}*/
	richText->update(dT);
}

void CGUIrichTextPanel::appendMarkedUpText(std::string text) {
	richText->appendMarkedUpText(text);
}

unsigned int CGUIrichTextPanel::getRichTextID() {
	return richText->getUniqueID();
}



bool CGUIrichTextPanel::MouseWheelMsg(const  int mouseX, const  int mouseY, int wheelDelta, int key) {
	return richText->MouseWheelMsg(mouseX, mouseY, wheelDelta, key);
}

void CGUIrichTextPanel::OnLMouseDown(const int mouseX, const int mouseY, int key) {
	if (!IsOnControl((CGUIbase&)*this,mouseX, mouseY)) {
		CMessage msg;
		msg.Msg = uiClickOutside;
		msg.x = mouseX; msg.y = mouseY;
		pDrawFuncs->handleUImsg(*this->richText, msg);

	}
}

void CGUIrichTextPanel::onRMouseUp(const int mouseX, const int mouseY) {

}

bool CGUIrichTextPanel::onMouseOff(const int mouseX, const int mouseY, int key) {
	return true;
}

void CGUIrichTextPanel::OnMouseMove(const int mouseX, const int mouseY, int key) {

}






void CGUIrichTextPanel::resizeToFit() {
	richText->resizeToFit();
}

void CGUIrichTextPanel::message(CGUIbase* sender, CMessage& msg) {
	if (msg.Msg == uiMsgChildResize) {
		setWidth(richText->getWidth() + (2 * inset));
		setHeight(richText->getHeight() + (2 * inset));
	}

	//handle rich text child events:
	if (sender == richText) {
		switch (msg.Msg) {
		case uiMsgDragging:
			if (draggable) {
				if (!dragging) {
					lastMousePos = glm::i32vec2(msg.x, msg.y);
					dragging = true;
				}
				else {
					glm::i32vec2 dMouse = glm::i32vec2(msg.x, msg.y) - lastMousePos;
					lastMousePos = glm::i32vec2(msg.x, msg.y);
					dMouse += getLocalPos();
					setLocalPos(dMouse.x, dMouse.y);

				}
			}
			break;

		case uiMsgLMouseUp:
			if (dragging) {
				dragging = false;
			}
			break;

		case uiMsgRMouseUp:
			callbackObj->GUIcallback(this, msg);
			break;

		case uiMsgHotTextChange:
			callbackObj->GUIcallback(this, msg);
			break;

		case uiMsgHotTextClick:
			callbackObj->GUIcallback(this, msg);
			break;

		}
	}

}

bool CGUIrichTextPanel::setTempText(bool onOff) {
	return richText->setTempText(onOff);
}

void CGUIrichTextPanel::suspend(bool isOn) {
	richText->suspend(isOn);
}

bool CGUIrichTextPanel::collapseTempText() {
	return richText->collapseTempText();
}

bool CGUIrichTextPanel::solidifyTempText() {
	return richText->solidifyTempText();
}

void CGUIrichTextPanel::unhotDuplicates() {
	richText->unhotDuplicates();
}

void CGUIrichTextPanel::removeMarked() {
	richText->removeMarked();
}

bool CGUIrichTextPanel::busy() {
	return richText->isBusy();
}

/** Pass the given text on to the rich text control, either immediately or via a buffer,
	depending on delivery style.*/
void CGUIrichTextPanel::displayText(std::string text) {
	richText->appendMarkedUpText(text);
	return;

	if (deliveryMode == noDelivery) {
		richText->appendMarkedUpText(text); 
		return;
	}
	deliveryBuffer += text;
}

/** Break the text in deliveryBuffer into clauses and send them individually to the rich text control to
	display. */
void CGUIrichTextPanel::deliverByClause(float dT) {
	std::string text;
	clauseInterval += dT;
	if (clauseInterval > clauseDelay) {
		clauseInterval = 0;
		//find next clause
		unsigned int found = deliveryBuffer.find_first_of(",.");
		if (found != std::string::npos) {
			found++;
		}
		else
			found = deliveryBuffer.size();
		text = deliveryBuffer.substr(0, found);
		deliveryBuffer = deliveryBuffer.substr(found, std::string::npos);
		richText->appendMarkedUpText(text);
	}
}

/** Break the text in deliveryBuffer into characters and send them individually to the rich text control. */
void CGUIrichTextPanel::deliverByCharacter(float dT) {
	std::string text;
	charInterval += dT;
	if (charInterval < charDelay)
		return;

	if (deliveryBuffer[0] == '\\') {
		if (deliveryBuffer.substr(1, 6) == "style{" || deliveryBuffer.substr(1, 2) == "h{") {
			unsigned int found = deliveryBuffer.find_first_of("}");
			text = deliveryBuffer.substr(0,found+1);
			//deliveryBuffer = deliveryBuffer.substr(found + 1, string::npos);
		}
		else if (deliveryBuffer[1] == 'h')
			text = deliveryBuffer.substr(0, 2);
	}
	else {
		text = deliveryBuffer.substr(0, 1);
		charInterval = 0;
	}
	deliveryBuffer = deliveryBuffer.substr(text.size(), std::string::npos);
	richText->appendMarkedUpText(text);

}

/** Return true if the mouse pointer is outside the panel and the mouseOffmargin. */
bool CGUIrichTextPanel::noMouse() {
	glm::i32vec2 margin = { mouseOffMargin, mouseOffMargin };
	if (glm::any(glm::lessThan(rootUI->mousePos, getScreenPos() - margin)) ||
		glm::any(glm::greaterThan(rootUI->mousePos, getScreenPos() + getSize() + margin))) {

		return true;
	}

	return false;
}





