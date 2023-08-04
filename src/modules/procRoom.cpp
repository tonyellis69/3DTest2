#include "procRoom.h"

#include "renderer/imRendr/imRendr.h"

#include "hex/hex.h"
#include "hex/hexArray.h"

CProcRoom::CProcRoom(glm::vec3& pos, glm::i32vec2& roomSize) {
	origin = pos;
	size = { roomSize.x * hexWidth, roomSize.y * hexRowHeight, 0 };

	origin = cubeToWorldSpace(worldSpaceToHex(origin));

	calcWorldCorners();
}

void CProcRoom::drawWireFrame() {
	imRendr::drawLine(a,b);
	imRendr::drawLine(b,c);
	imRendr::drawLine(c,d);
	imRendr::drawLine(d,a);
}

void CProcRoom::writeHexes(CHexArray& array) {
	writeHexLine(a, b, array);
	writeHexLine(b, c, array);
	writeHexLine(c, d, array);
	writeHexLine(d, a, array);
}

/**	Return the penetration of A into B. */
glm::vec3 CProcRoom::findOverlap(CProcRoom& roomB) {
	glm::vec3 AhalfSize = size / 2.0f;
	glm::vec3 BhalfSize = roomB.size / 2.0f;

	const float margin = 0.00001f;

	if (abs(origin.x - roomB.origin.x) >= (AhalfSize.x + BhalfSize.x - margin) ||
		abs(origin.y - roomB.origin.y) >= (AhalfSize.y + BhalfSize.y - margin))
		return glm::vec3(0);

	glm::vec3 separation(0);
	separation.x = (AhalfSize.x + BhalfSize.x) - std::abs(origin.x - roomB.origin.x);
	separation.y = (AhalfSize.y + BhalfSize.y) - std::abs(origin.y - roomB.origin.y);

	glm::vec3 dir = roomB.origin - this->origin;
	separation.x *= dir.x > 0 ? 1 : -1;
	separation.y *= dir.y > 0 ? 1 : -1;

	return separation;
}

bool CProcRoom::isApartFrom(CProcRoom& roomB) {
	glm::vec3 AhalfSize = size / 2.0f;
	glm::vec3 BhalfSize = roomB.size / 2.0f;
	float margin = 0.5f;

	if (abs(origin.x - roomB.origin.x) >= (AhalfSize.x + BhalfSize.x + margin) ||
		abs(origin.y - roomB.origin.y) >= (AhalfSize.y + BhalfSize.y + margin))
		return true;
	return false;
}


bool CProcRoom::notInside(glm::vec3& halfGrid) {
	float halfW = size.x / 2.0f;
	float halfH = size.y / 2.0f;

	if (a.x < -halfGrid.x || a.y > halfGrid.y ||
		c.x > halfGrid.x || c.y < -halfGrid.y)
		return true;

	return false;
}


void CProcRoom::move(glm::i32vec2& m) {
	glm::i32vec2 offset = cubeToOffset(worldSpaceToHex(origin));
	offset += m;
	origin = cubeToWorldSpace(offsetToCube(offset));
	calcWorldCorners();
}

void CProcRoom::calcWorldCorners() {
	float halfW = size.x / 2.0f;
	float halfH = size.y / 2.0f;

	a = glm::vec3(-halfW, halfH, 0) + origin;
	b = glm::vec3(halfW, halfH, 0) + origin;
	c = glm::vec3(halfW, -halfH, 0) + origin;
	d = glm::vec3(-halfW, -halfH, 0) + origin;

	//form a rectangle with corners on hex centres
	glm::vec3 halfSize = size / 2.0f;
	a = glm::vec3(0, size.y, 0) - halfSize + origin;
	b = glm::vec3(size.x, size.y, 0) - halfSize + origin;
	c = glm::vec3(size.x, 0, 0) - halfSize + origin;
	d = glm::vec3(0, 0, 0) - halfSize + origin;

}

void CProcRoom::writeHexLine(glm::vec3& a, glm::vec3& b, CHexArray& array) {
	CHex A = worldSpaceToHex(a);
	CHex B = worldSpaceToHex(b);

	THexList hexes;

	hexes = *hexLine(A, B);

	for (auto& hex : hexes) {
		array.getHexCube(hex).content = solidHex;
	}

	return;

}

