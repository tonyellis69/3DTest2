#pragma once

#include <glm/glm.hpp>

#include "baseRect.h"

class CProcRoom;

class CIndiRect : public CBaseRect {
public:
	CIndiRect(CProcRoom& roomA, CProcRoom& roomB);

	void drawWireFrame();

	//glm::vec3 a;
	//glm::vec3 b;
	//glm::vec3 c;
	//glm::vec3 d;

	int sharedVert;
};