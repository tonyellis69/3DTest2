#pragma once

#include <glm/glm.hpp>

class CHexArray;
class CProcRoom;
class CDoorRect {
public:
	CDoorRect(CProcRoom& roomA, CProcRoom& roomB);
	void drawWireFrame();
	void writeHexes(CHexArray& array);


	glm::vec3 origin;
	glm::vec3 size;

	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;



};