#include "indiRect.h"

#include "procRoom.h"

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

	a = {BL.x, TR.y, 0};
	b = {TR.x, TR.y, 0 };
	c = {TR.x, BL.y, 0 };
	d = {BL.x, BL.y, 0 };

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

