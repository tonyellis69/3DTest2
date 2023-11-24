#include "indiRect.h"

#include "procRoom.h"

#include "renderer/imRendr/imRendr.h"

CIndiRect::CIndiRect(CProcRoom& roomA, CProcRoom& roomB) {

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
		if (roomA.isOnBoundary(v) && roomB.isOnBoundary(v))
			sharedVert = i;
	}
}

void CIndiRect::drawWireFrame() {

	imRendr::setDrawColour({ 1,0,0,1 });

	imRendr::drawLine(a, b);
	imRendr::drawLine(b, c);
	imRendr::drawLine(c, d);
	imRendr::drawLine(d, a);

	imRendr::setDrawColour({ 1,1,1,1 });
}
