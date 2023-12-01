#pragma once

#include <glm/glm.hpp>

#include "baseRect.h"
#include "procRoom.h"
//class CProcRoom;

class CIndiRect : public CBaseRect {
public:
	CIndiRect(CProcRoom& roomA, CProcRoom& roomB);
	void drawWireFrame();
	bool clip(CProcRoom& room);


	CProcRoom A;
	CProcRoom B;

	int sharedCorner;

	bool scrapped = false;

};