#include "itemDraw.h"

#include "../entity/entity.h"

CItemDraw::CItemDraw(CEntity* owner) : CDrawFunc(owner)  {

}

void CItemDraw::draw(CHexRender& hexRender) {
	auto& pModel = pOwner->modelCmp->model;
	//auto& pMesh = pModel.meshes[0];

	for (unsigned int m = 0; m < 2; m++) {
		auto& mesh = pModel.meshes[m];
		hexRender.loadSolidList(TSolidDraw{ &pModel.buf, mesh.matrix,&pModel.palette,
	&mesh.meshRec });

	}

	auto& mesh = pModel.meshes[2];

	if (pModel.buf.numVerts > 0)
		hexRender.loadLineList(TLineDraw{ &pModel.buf, mesh.matrix, &pModel.palette,
			&mesh.meshRec });

}
