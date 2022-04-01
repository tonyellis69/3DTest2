#include "avoidGraphic.h"

#include "../robot.h"

#include "hexRender.h"

CAvoidGraphic::CAvoidGraphic() {
	verts.resize(8);
	mesh.indexSize = 9;
	mesh.indexStart = 0;
	mesh.vertStart = 0;
}

void CAvoidGraphic::update(float dT) {
	CRobot* pRobot = (CRobot*)entity.get();

	std::fill(verts.begin(), verts.end() - 4, vc3({ { 0,0,0 }, { 1 } } ));
	//verts[1].v = pRobot->lAvoidVec[0];
	//verts[2].v = pRobot->lAvoidVec[1];
	//verts[0].v = 2.0f * verts[1].v - verts[2].v;
	//verts[3].v = 2.0f * verts[2].v - verts[1].v;

	//verts[5].v = pRobot->rAvoidVec[0];
	//verts[6].v = pRobot->rAvoidVec[1];
	//verts[4].v = 2.0f * verts[5].v - verts[6].v;
	//verts[7].v = 2.0f * verts[6].v - verts[5].v;

	//if (pRobot->lObstacle) {
	//	for (int v = 0; v < 4; v++)
	//		verts[v].c = 1;
	//}
	//if (pRobot->rObstacle) {
	//	for (int v = 4; v < 8; v++)
	//		verts[v].c = 1;
	//}


	//std::vector<unsigned short> index = { 0, 1, 2, 3, 0xFFFF,4,5,6,7 };

	verts[1].v = pRobot->tmpCollisionSegPt;
	verts[2].v = pRobot->tmpCollisionPt;
	verts[0].v = 2.0f * verts[1].v - verts[2].v;
	verts[3].v = 2.0f * verts[2].v - verts[1].v;

	verts[5].v = pRobot->tmpAheadVecBegin;
	verts[6].v = pRobot->tmpAheadVecEnd;
	verts[4].v = 2.0f * verts[5].v - verts[6].v;
	verts[7].v = 2.0f * verts[6].v - verts[5].v;

	//std::vector<unsigned short> index = { 0, 1, 2, 3 };
	std::vector<unsigned short> index = { 0, 1, 2, 3, 0xFFFF,4,5,6,7 };

	buf.storeVerts(verts, index, 3, 1);


}

void CAvoidGraphic::draw(CHexRender& hexRender) {
	if (buf.numVerts > 0)
		hexRender.lineDrawList.push_back({ &buf, &matrix, pPalette, &mesh });
}
