#include "itemDraw.h"

#include "../entity.h"

CItemDraw::CItemDraw(CEntity* owner) : CDrawFunc(owner)  {

}

void CItemDraw::draw(CHexRender& hexRender) {
	for (unsigned int m = 0; m < 2; m++) {
		auto& mesh = model->meshes[m];
		hexRender.solidDrawList.push_back({ &model->buf, &mesh.matrix,pOwner->pPalette,
	&mesh.meshRec });

	}

	auto& mesh = model->meshes[2];

	if (model->buf.numVerts > 0)
		hexRender.lineDrawList.push_back({ &model->buf, &mesh.matrix,pOwner->pPalette,
			&mesh.meshRec });
}
