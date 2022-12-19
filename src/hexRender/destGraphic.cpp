#include "destGraphic.h"

#include "../robot.h"

#include "hexRender.h"

#include "utils/log.h"

CDestinationGraphic::CDestinationGraphic() {
	verts.resize(4);
	mesh.indexSize = 4;
	mesh.indexStart = 0;
	mesh.vertStart = 0;


}



/** Create the verts. */
void CDestinationGraphic::update(float dT) {
	CRobot* pRobot = (CRobot*)entity.get();

	dest = &pRobot->getDestination();



	verts[1].v = entity->getPos();
	verts[2].v =  *dest;
	verts[0].v = 2.0f * verts[1].v - verts[2].v;
	verts[3].v = 2.0f * verts[2].v - verts[1].v;

	std::vector<unsigned short> index = {0, 1, 2, 3};
	buf.storeVerts(verts, index, 3, 1);


}

void CDestinationGraphic::draw(CHexRender& hexRender) {

	if (dest == nullptr) {
		 return;
	}
	if (buf.numVerts > 0) 
			hexRender.loadLineList(TLineDraw{ &buf, matrix, pPalette, &mesh});


}

