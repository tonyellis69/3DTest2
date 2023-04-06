#include "avoidGraphic.h"

//#include "../robot.h"
#include "../roboState.h"

#include "hexRender.h"

CAvoidGraphic::CAvoidGraphic() {
	verts.resize(8);
	mesh.indexSize = 9;
	mesh.indexStart = 0;
	mesh.vertStart = 0;
}

void CAvoidGraphic::update(float dT) {
	CEntity* pRobot = entity.get();

	std::fill(verts.begin(), verts.end() - 4, vc3({ { 0,0,0 }, { 1 } } ));

	auto  ai = ((CRoboState*)pRobot->ai.get());

	verts[1].v = ai->tmpCollisionSegPt;
	verts[2].v = ai->tmpCollisionPt;
	verts[0].v = 2.0f * verts[1].v - verts[2].v;
	verts[3].v = 2.0f * verts[2].v - verts[1].v;

	verts[5].v = ai->tmpAheadVecBegin;
	verts[6].v = ai->tmpAheadVecEnd;
	verts[4].v = 2.0f * verts[5].v - verts[6].v;
	verts[7].v = 2.0f * verts[6].v - verts[5].v;


	std::vector<unsigned short> index = { 0, 1, 2, 3, 0xFFFF,4,5,6,7 };

	buf.storeVerts(verts, index, 3, 1);


}

void CAvoidGraphic::draw(CHexRender& hexRender) {
	if (buf.numVerts > 0)
		hexRender.loadLineList(TLineDraw{ &buf, matrix, pPalette, & mesh });
}
