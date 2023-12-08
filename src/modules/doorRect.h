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

	void writeHexLine(glm::vec3& a, glm::vec3& b, CHexArray& array);

	bool tooSmall = false;

	CProcRoom& roomA;
	CProcRoom& roomB;

	glm::vec3 origin;
	glm::vec3 size;

	//glm::vec3 a;
	//glm::vec3 b;
	//glm::vec3 c;
	//glm::vec3 d;

	glm::vec3 door1A;
	glm::vec3 door1B;

	glm::vec3 door2A;
	glm::vec3 door2B;



};