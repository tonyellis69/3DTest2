#pragma once

#include <glm/glm.hpp>

#include "hex/hex.h"
#include "baseRect.h"

class CHexArray;
class CProcRoom : public CBaseRect {
public:
	CProcRoom(glm::vec3& pos, glm::i32vec2& size);
	void drawWireFrame();
	void writeHexes(CHexArray& array);
	glm::vec3 findOverlap(CProcRoom& roomB);
	bool isApartFrom(CProcRoom& roomB);
	bool notInside(glm::vec3& grid);
	void move(glm::i32vec2& m);
	glm::vec3 getOrigin() {
		return origin;
	}
	glm::vec3 getSize() {
		return size;
	}
	CHex getOriginHex() {
		return worldSpaceToHex(origin);
	}
	THexList getCornerHexes() {
		THexList corners;
		corners.push_back(worldSpaceToHex(a));
		corners.push_back(worldSpaceToHex(b));
		corners.push_back(worldSpaceToHex(c));
		corners.push_back(worldSpaceToHex(d));
		return corners;
	}

	//glm::vec3 a;
	//glm::vec3 b;
	//glm::vec3 c;
	//glm::vec3 d;

private:
	void calcWorldCorners();
	void writeHexLine(glm::vec3& a, glm::vec3& b, CHexArray& array);

	glm::vec3 origin;
	glm::vec3 size;





};