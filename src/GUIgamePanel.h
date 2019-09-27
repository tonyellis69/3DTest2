#pragma once

#include "..\\3DEngine\src\UI\GUIpanel.h"

class CGUIgamePanel : public CGUIpanel {
public:
	CGUIgamePanel(int x, int y, int w, int h);



	void setObjId(int objId) {
		this->objId = objId;
	}
	int getObjId() { return objId; }

	int objId; ///<The associated objId if this is a popUp window;
};