#pragma once

#include <glm/glm.hpp>

#include "baseRect.h"

/** Scrapping in favour of automata. */
/** No I'm not */


class CHexArray;
class CProcRoom;
class CDoorRect : public CBaseRect {
public:
	CDoorRect(CProcRoom& roomA, CProcRoom& roomB);
	void drawWireFrame();
	void writeHexes(CHexArray& array);


	bool tooSmall = false;

	CProcRoom& roomA;
	CProcRoom& roomB;

	glm::vec3 size;

	bool leftRight;

	int doorHexWidth = 2;

	glm::vec3 door1A;
	glm::vec3 door1B;

	glm::vec3 door2A;
	glm::vec3 door2B;



};