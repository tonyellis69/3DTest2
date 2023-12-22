#include "doorRect.h"

#include <algorithm>

#include "procRoom.h"
#include "hex/hexArray.h"
#include "renderer/imRendr/imRendr.h"

#include "intersect.h"

CDoorRect::CDoorRect(CProcRoom& A, CProcRoom& B) : roomA(A), roomB(B) {
	//make a rectangle that connects both rooms
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

	setVerts(glm::vec3(left, baseY, 0), glm::vec3(right, topY, 0));

	glm::vec3 midPt1;
	glm::vec3 midPt2;

	//are the rooms roughly side-by side?
	if ( (left + 0.0001f >= (roomA.getOrigin().x + AhalfSize.x) && (right - 0.0001f <= roomB.getOrigin().x - BhalfSize.x)) ||	
		(left + 0.0001f  >= (roomB.getOrigin().x + BhalfSize.x) && (right - 0.0001f <= roomA.getOrigin().x - AhalfSize.x)) ) {
		leftRight = true;

		const float halfDoorHeight = hexHeight;

		if (glm::length(a - d) < hexRowHeight * 4) {
			tooSmall = true;
		}

		//!!!!TODO: this is for the diagnostic graphics and can go
		midPt1 = a + (d - a) / 2.0f;     
		midPt2 = b + (c - b) / 2.0f;
	
		door1A = midPt1 + glm::vec3(0,halfDoorHeight, 0);
		door1B = midPt1 + glm::vec3(0,-halfDoorHeight, 0);
		door2A = midPt2 + glm::vec3(0,halfDoorHeight, 0);
		door2B = midPt2 + glm::vec3(0,-halfDoorHeight, 0);
	} 
	else {	//top-bottom door
		leftRight = false;
		const float halfDoorWidth = hexWidth;// hexWidth * 1.5f;

		if (glm::length(b - a) + 0.0001f < hexWidth * 4) {
			tooSmall = true;
		}

		//!!!!TODO: this is for the diagnostic graphics and can go
		midPt1 = a + (b - a) / 2.0f;
		midPt2 = d + (c - d) / 2.0f;
		midPt1 = cubeToWorldSpace(worldSpaceToHex(midPt1));
		midPt2 = cubeToWorldSpace(worldSpaceToHex(midPt2));

		
		door1A = midPt1 + glm::vec3(halfDoorWidth, 0, 0) ;
		door1B = midPt1 + glm::vec3(-halfDoorWidth, 0, 0);
		door2A = midPt2 + glm::vec3(halfDoorWidth, 0, 0) ;
		door2B = midPt2 + glm::vec3(-halfDoorWidth, 0, 0);

	}
}

void CDoorRect::drawWireFrame() {
	if (tooSmall)
		imRendr::setDrawColour({1,0,0,1 });
	else
		imRendr::setDrawColour({0,1,0,1});
	imRendr::drawLine(a, b);
	imRendr::drawLine(b, c);
	imRendr::drawLine(c, d);
	imRendr::drawLine(d, a);

	if (tooSmall)
		imRendr::setDrawColour({ 1,0,0,1 });
	else
		imRendr::setDrawColour({ 1,0,1,1 });
	imRendr::drawLine(door1A, door1B);
	imRendr::drawLine(door2A, door2B);


	imRendr::setDrawColour({ 1,1,1,1 });
}

/** Write blank hexes for doors, lines of hexes for corridor walls.*/
void CDoorRect::writeHexes(CHexArray& array) {
	CHex nearestCentreHex = worldSpaceToHex(origin);

	if (leftRight) {
		float leftBorder = a.x - 0.001f;
		float rightBorder = c.x + 0.001f;
		CHex offsetCentreHex = nearestCentreHex;

		for (int row = 0; row < doorHexWidth; row++) {
			glm::vec3 hexPos = cubeToWorldSpace(offsetCentreHex);

			CHex currentHex = worldSpaceToHex({ leftBorder,hexPos.y,0 });
			CHex endHex = worldSpaceToHex({ rightBorder,hexPos.y,0 });

			while (getNeighbour(currentHex, hexWest) != endHex) {
				if (array.getHexCube(currentHex).content == solidHex) {
					array.getHexCube(currentHex).content = emptyHex;
				}
				currentHex = getNeighbour(currentHex, hexEast);
			}
			offsetCentreHex = getNeighbour(offsetCentreHex, hexNW);
		}

		//now draw corridor walls:
		float originHexY = cubeToWorldSpace(nearestCentreHex).y;
		CHex start[2];
		start[0] = worldSpaceToHex({ leftBorder,originHexY - hexRowHeight,0 });
		start[1] = worldSpaceToHex({ leftBorder,originHexY + (hexRowHeight * 2)  ,0 });

		CHex end[2];
		end[0] = worldSpaceToHex({ rightBorder,originHexY - hexRowHeight,0 });
		end[1] = worldSpaceToHex({ rightBorder,originHexY + (hexRowHeight * 2) ,0 });

		for (int wall = 0; wall < 2; wall++) {
			bool drawing = false;
			CHex currentHex = start[wall];
			while (getNeighbour(currentHex, hexWest) != end[wall]) {
				if (array.getHexCube(currentHex).content == solidHex) {
					drawing = !drawing;
				}
				else if (drawing)
					array.getHexCube(currentHex).content = solidHex;

				currentHex = getNeighbour(currentHex, hexEast);
			}
		}
	}
	else { //north-south
		float topBorder = a.y + 0.001f;
		float bottomBorder = c.y - 0.001f;

		float originHexX = cubeToWorldSpace(nearestCentreHex).x;
		CHex forcedHex = worldSpaceToHex({ originHexX - 0.001f, bottomBorder,0 });
		CHex start[2];
		start[0] = forcedHex;
		start[1] = getNeighbour(forcedHex, hexEast);

		int endRow = worldSpaceToHex({ 0, topBorder, 0 }).r;

		for (int door = 0; door < 2; door++) {
			CHex currentHex = start[door];
			bool leftVeer = true;
			while (currentHex.r >= endRow) {
				array.getHexCube(currentHex).content = emptyHex;
				currentHex = (leftVeer) ? getNeighbour(currentHex, hexNW) : getNeighbour(currentHex, hexNE);
				leftVeer = !leftVeer;
			}
		}


		//now do the corridor walls:
		start[0] = getNeighbour(start[0], hexWest);
		start[1] = getNeighbour(start[1], hexEast);

		for (int wall = 0; wall < 2; wall++) {
			CHex currentHex = start[wall];
			bool leftVeer = true;
			bool drawing = false;
			while (currentHex.r >= endRow) {
				if (array.getHexCube(currentHex).content == solidHex) {
					drawing = !drawing;
				}
				else if (drawing)
					array.getHexCube(currentHex).content = solidHex;
				currentHex = (leftVeer) ? getNeighbour(currentHex, hexNW) : getNeighbour(currentHex, hexNE);
				leftVeer = !leftVeer;
			}
		}

	}
}

