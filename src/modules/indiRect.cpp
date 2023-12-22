#include "indiRect.h"

#include "procRoom.h"
#include "hex/hexArray.h"

#include "renderer/imRendr/imRendr.h"

#include "intersect.h"

CIndiRect::CIndiRect(CProcRoom& roomA, CProcRoom& roomB) : A(roomA), B(roomB) {

	glm::vec3 BL;
	glm::vec3 TR;

	//find enclosing rect
	BL.x = std::min(roomA.a.x, roomB.a.x);
	BL.y = std::min(roomA.c.y, roomB.c.y);
	TR.x = std::max(roomA.c.x, roomB.c.x);
	TR.y = std::max(roomA.a.y, roomB.a.y);

	if (roomA.a.x > BL.x) 	//clip horizontally by A
		TR.x = roomA.a.x;
	else
		BL.x = roomA.c.x;

	if (roomA.a.y < TR.y) 	//clip vertically by B
		TR.y = roomB.c.y;
	else
		BL.y = roomB.a.y;

	setVerts(BL, TR);

	for (int i = 0; i < 4; i++) {
		glm::vec3 v = getVert(i);
		if (roomA.isOnBoundary(v) &&  roomB.isOnBoundary(v))
			sharedCorner = i;
	}

	if (sharedCorner == -1) { //rooms not in contact, use method B
		float dist = FLT_MAX;
		for (int i = 0; i < 4; i++) {
			float newDist = sqrDistPointSegment(roomA.getOrigin(), roomB.getOrigin(), getVert(i));
			if (newDist < dist) {
				sharedCorner = i;
				dist = newDist;
			}
		}
	}
}

void CIndiRect::drawWireFrame() {

	if (scrapped)
		imRendr::setDrawColour({ 1,1,0,1 });
	else
		imRendr::setDrawColour({ 1,0,0,1 });

	imRendr::drawLine(a, b);
	imRendr::drawLine(b, c);
	imRendr::drawLine(c, d);
	imRendr::drawLine(d, a);

	imRendr::setDrawColour({ 1,1,1,1 });
}


void CIndiRect::clipAgainstRoom(CProcRoom& room) {
	if (room == A || room == B)
		return;

	//if there's no intersection, bail
	float x1 = std::max(d.x, room.d.x);
	float y1 = std::max(d.y, room.d.y);
	float x2 = std::min(b.x, room.b.x);
	float y2 = std::min(b.y, room.b.y);
	if (x1 >= x2 || y1 >= y2)
		return;



	//find the intersection rect that has sharedVert.

	//let opCorner be this indiRect's corner diagonally opposite sharedCorner
	int opCorner = (sharedCorner + 2) % 4;
	glm::vec3 sharedVert = getVert(sharedCorner);
	glm::vec3 opVert = getVert(opCorner);

	//if intersecting room has shared vert, it spoils this rect
	for (int c = 0; c < 4; c++) {
		if (glm::distance(room.getVert(c), sharedVert) < 0.0001) {
			scrapped = true;
			return;
		}
	}

	glm::vec3 oldOpVert = opVert;

	if ((y1 != sharedVert.y) && abs(y1 - sharedVert.y) < abs(opVert.y - sharedVert.y))
		opVert.y = y1;
	if ((y2 != sharedVert.y) && abs(y2 - sharedVert.y) < abs(opVert.y - sharedVert.y))
		opVert.y = y2;

	if ((x1 != sharedVert.x) && abs(x1 - sharedVert.x) < abs(opVert.x - sharedVert.x))
		opVert.x = x1;
	if ((x2 != sharedVert.x) && abs(x2 - sharedVert.x) < abs(opVert.x - sharedVert.x))
		opVert.x = x2;

	if (opVert.x != oldOpVert.x && opVert.y != oldOpVert.y) {
		//revert the biggest change
		glm::vec3 diff = opVert - sharedVert;
		if (abs(diff.x) < abs(diff.y))
			opVert.x = oldOpVert.x;
		else
			opVert.y = oldOpVert.y;
	}

	if (sharedVert.x == opVert.x || sharedVert.y == opVert.y) {
		scrapped = true;
		return;
	}


	setVerts(sharedVert, opVert);
}

void CIndiRect::writeHexes(CHexArray& array) {
	//
	CHex sharedCornerHex = worldSpaceToHex(getVert(sharedCorner));

	int adjCorner; THexDir inDir; THexDir slopeDir;
	switch (sharedCorner) {
	case 0: adjCorner = 1; inDir = hexNW; slopeDir = hexNE; break;
	case 1: adjCorner = 0; inDir = hexNE;  slopeDir = hexNW;  break;
	case 2: adjCorner = 3; inDir = hexSE; slopeDir = hexSW; break;
	case 3: adjCorner = 2; inDir = hexSW;  slopeDir = hexSE;  break;
	}

	CHex furthestDiagHex = worldSpaceToHex(getVert(adjCorner));
	furthestDiagHex = (adjCorner == 0 || adjCorner == 3) ? getNeighbour(furthestDiagHex, hexEast)
		: getNeighbour(furthestDiagHex, hexWest);

	int topR = worldSpaceToHex(a).r +1;
	int bottomR = worldSpaceToHex(c).r-1;

	CHex mitre[4];

	//find hex where the two diagonals meet
	CHex tmpHex;
	if (sharedCorner == 0 || sharedCorner == 2) {
		mitre[3] = CHex(sharedCornerHex.q, -sharedCornerHex.q - furthestDiagHex.s, furthestDiagHex.s);
	}
	else {
		mitre[3] = CHex(furthestDiagHex.q, -furthestDiagHex.q - sharedCornerHex.s, sharedCornerHex.s);
	}

	//ensure mitre inside rect 
	if ((sharedCorner == 3 || sharedCorner == 2) && mitre[3].r < topR) {
		mitre[3].r = topR;
		if (sharedCorner == 2)
			mitre[3].s = -mitre[3].q - mitre[3].r;
		else
			mitre[3].q = -mitre[3].s - mitre[3].r;
	}
	if ((sharedCorner == 0 || sharedCorner == 1) && mitre[3].r > bottomR) {
		mitre[3].r = bottomR;
		if (sharedCorner == 0)
			mitre[3].s = -mitre[3].q - mitre[3].r;
		else
			mitre[3].q = -mitre[3].s - mitre[3].r;
	}

	//create mitre
	for (int s = 2; s >= 0; s--) {
		mitre[s] = getNeighbour(mitre[s + 1], inDir);
	}


	//for (int t = 0; t < 4; t++)
	//	array.getHexCube(mitre[t]).content = testHex3;


	//carve doors
	bool eastDoor = (sharedCorner == 1 || sharedCorner == 2) ? true : false;
	bool northDoor = (sharedCorner == 0 || sharedCorner == 1) ? true : false;
	for (int door = 1; door < 3; door++) {
		//horizontal
		CHex currentHex = mitre[door];
		bool carving = false;
		while (!carving || array.getHexCube(currentHex).content == solidHex) {
			if (array.getHexCube(currentHex).content == solidHex) {
				carving = true;
				array.getHexCube(currentHex).content = emptyHex;
			}
			currentHex = (eastDoor) ? getNeighbour(currentHex, hexEast) : getNeighbour(currentHex, hexWest);
		}

		//angled
		currentHex = mitre[door];
		carving = false;
		while (!carving || array.getHexCube(currentHex).content == solidHex) {
			if (array.getHexCube(currentHex).content == solidHex) {
				carving = true;
				array.getHexCube(currentHex).content = emptyHex;
			}
			currentHex = getNeighbour(currentHex, slopeDir);
		}
	}


	//corridor walls
	for (int wall = 0; wall < 4; wall+=3) {
		//horizontal
		CHex currentHex = mitre[wall];
		while (array.getHexCube(currentHex).content != solidHex) {
			array.getHexCube(currentHex).content = solidHex;
			currentHex = (eastDoor) ? getNeighbour(currentHex, hexEast) : getNeighbour(currentHex, hexWest);
		}
		 
		//angled
		currentHex = mitre[wall];
		while ((northDoor) ? currentHex.r >= topR : currentHex.r <= bottomR) {
			array.getHexCube(currentHex).content = solidHex;
			currentHex = getNeighbour(currentHex, slopeDir);
		} 
	}

}

