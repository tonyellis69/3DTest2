#include "itemDraw.h"

#include "../entity/entity.h"

CItemDraw::CItemDraw(CEntity* owner) : CDrawFunc(owner)  {

}

void CItemDraw::draw(CHexRender& hexRender) {
	for (unsigned int m = 0; m < 2; m++) {
		auto& mesh = model->meshes[m];
		hexRender.loadSolidList(TSolidDraw{ &model->buf, mesh.matrix,&model->palette,
	&mesh.meshRec });

	}

	auto& mesh = model->meshes[2];

	if (model->buf.numVerts > 0)
		hexRender.loadLineList(TLineDraw{ &model->buf, mesh.matrix, &model->palette,
			&mesh.meshRec });

}
