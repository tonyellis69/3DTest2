#include "destGraphic.h"

#include "../robot.h"

CDestinationGraphic::CDestinationGraphic() {
	verts.resize(4);
	mesh.indexSize = 4;
	mesh.indexStart = 0;
	mesh.vertStart = 0;
}

#include "hexRender.h"

/** Create the verts. */
void CDestinationGraphic::update(float dT) {
	CRobot* pRobot = (CRobot*)entity.get();

	dest = pRobot->getDestination();
	if (dest == nullptr) {
		return;
	}

	verts[1].v = entity->worldPos;
	verts[2].v = *dest;
	verts[0].v = 2.0f * verts[1].v - verts[2].v;
	verts[3].v = 2.0f * verts[2].v - verts[1].v;

	std::vector<unsigned short> index = {0, 1, 2, 3};
	buf.storeVerts(verts, index, 3, 1);
}

void CDestinationGraphic::draw(CHexRender& hexRender) {
	if (dest == nullptr)
		return;
	hexRender.lineDrawList.push_back({ &buf, &matrix, pPalette, &mesh });
}

