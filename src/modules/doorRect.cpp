#include "doorRect.h"

#include <algorithm>

#include "procRoom.h"
#include "hex/hexArray.h"
#include "renderer/imRendr/imRendr.h"

CDoorRect::CDoorRect(CProcRoom& roomA, CProcRoom& roomB) {
	glm::vec3 AhalfSize = roomA.getSize() / 2.0f;
	glm::vec3 BhalfSize = roomB.getSize() / 2.0f;

	glm::vec3 vecAB = roomA.getOrigin() - roomB.getOrigin();

	float lowestTop = std::min(roomA.getOrigin().y + AhalfSize.y, roomB.getOrigin().y + BhalfSize.y);
	float highestBase = std::max(roomA.getOrigin().y - AhalfSize.y, roomB.getOrigin().y - BhalfSize.y);
	float topY = std::max(lowestTop, highestBase);
	float baseY = std::min(lowestTop, highestBase);
	float midY = baseY + ((topY - baseY) / 2.0f);

	float width = (AhalfSize.x + BhalfSize.x) - glm::abs(roomA.getOrigin().x - roomB.getOrigin().x);


	float lowestRight = std::min(roomA.getOrigin().x + AhalfSize.x, roomB.getOrigin().x + BhalfSize.x);
	float highestLeft = std::max(roomA.getOrigin().x - AhalfSize.x, roomB.getOrigin().x - BhalfSize.x);
	float right = std::max(highestLeft, lowestRight);
	float left = std::min(highestLeft, lowestRight);

	a = glm::vec3(left, topY, 0);
	b = glm::vec3(right, topY, 0);
	c = glm::vec3(right, baseY, 0);
	d = glm::vec3(left, baseY, 0);

	origin = a + (c - a) / 2.0f;

	if (abs(vecAB.x) > abs(vecAB.y)) {

	} 
	else {

	}
}

void CDoorRect::drawWireFrame() {
	imRendr::setDrawColour({1,0,1,1});
	imRendr::drawLine(a, b);
	imRendr::drawLine(b, c);
	imRendr::drawLine(c, d);
	imRendr::drawLine(d, a);
	imRendr::setDrawColour({ 1,1,1,1 });
}

/** Write blank hexes for doors, lines of hexes for corridor walls.*/
void CDoorRect::writeHexes(CHexArray& array) {

}
