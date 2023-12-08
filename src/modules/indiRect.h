#pragma once

#include <glm/glm.hpp>

#include "baseRect.h"
#include "procRoom.h"
//class CProcRoom;

class CIndiRect : public CBaseRect {
public:
	CIndiRect(CProcRoom& roomA, CProcRoom& roomB);
	void drawWireFrame();
	void clipAgainstRoom(CProcRoom& room);


	CProcRoom A;
	CProcRoom B;

	int sharedCorner = -1;

	bool scrapped = false;

};