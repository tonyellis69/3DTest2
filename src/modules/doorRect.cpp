#include "doorRect.h"

#include <algorithm>

#include "procRoom.h"
#include "hex/hexArray.h"
#include "renderer/imRendr/imRendr.h"

#include "intersect.h"

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

	glm::vec3 midPt1;
	glm::vec3 midPt2;


	if ( (left + 0.0001f >= (roomA.getOrigin().x + AhalfSize.x) && (right - 0.0001f <= roomB.getOrigin().x - BhalfSize.x)) ||
		
		(left + 0.0001f  >= (roomB.getOrigin().x + BhalfSize.x) && (right - 0.0001f <= roomA.getOrigin().x - AhalfSize.x)) ) {



//	if (abs(vecAB.x) > abs(vecAB.y)) { //left-right door
//	if ( (AhalfSize.x + BhalfSize.x) - vecAB.x <= 0) {
		midPt1 = a + (d - a) / 2.0f;
		midPt2 = b + (c - b) / 2.0f;

		const float halfDoorHeight = hexHeight;// *1.5f;
		door1A = midPt1 + glm::vec3(0,halfDoorHeight, 0);
		door1B = midPt1 + glm::vec3(0,-halfDoorHeight, 0);
		door2A = midPt2 + glm::vec3(0,halfDoorHeight, 0);
		door2B = midPt2 + glm::vec3(0,-halfDoorHeight, 0);
	} 
	else {
		midPt1 = a + (b - a) / 2.0f;
		midPt2 = d + (c - d) / 2.0f;
		midPt1 = cubeToWorldSpace(worldSpaceToHex(midPt1));
		midPt2 = cubeToWorldSpace(worldSpaceToHex(midPt2));

		const float halfDoorWidth = hexWidth;// hexWidth * 1.5f;
		door1A = midPt1 + glm::vec3(halfDoorWidth, 0, 0) ;
		door1B = midPt1 + glm::vec3(-halfDoorWidth, 0, 0);
		door2A = midPt2 + glm::vec3(halfDoorWidth, 0, 0) ;
		door2B = midPt2 + glm::vec3(-halfDoorWidth, 0, 0);

	}


	//door1A = cubeToWorldSpace(worldSpaceToHex(door1A));
	//door1B = cubeToWorldSpace(worldSpaceToHex(door1B));
	//door2A = cubeToWorldSpace(worldSpaceToHex(door2A));
	//door2B = cubeToWorldSpace(worldSpaceToHex(door2B));
}

void CDoorRect::drawWireFrame() {
	imRendr::setDrawColour({0,1,0,1});
	imRendr::drawLine(a, b);
	imRendr::drawLine(b, c);
	imRendr::drawLine(c, d);
	imRendr::drawLine(d, a);

	imRendr::setDrawColour({ 1,0,1,1 });
	imRendr::drawLine(door1A, door1B);
	imRendr::drawLine(door2A, door2B);


	imRendr::setDrawColour({ 1,1,1,1 });
}

/** Write blank hexes for doors, lines of hexes for corridor walls.*/
void CDoorRect::writeHexes(CHexArray& array) {

}
