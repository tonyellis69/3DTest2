#include "itemDraw.h"

#include "../entity.h"

CItemDraw::CItemDraw(CEntity* owner) : CDrawFunc(owner)  {

}

void CItemDraw::draw(CHexRender& hexRender) {
	for (unsigned int m = 0; m < 2; m++) {
		auto& mesh = model->meshes[m];
		hexRender.solidDrawList.push_back({ model->meshes[0].draw.buf.get(), &mesh.matrix,pOwner->pPalette,
	&mesh.draw.meshRec });

	}

	auto& mesh = model->meshes[2];

	if (model->meshes[0].draw.buf.get()->numVerts > 0)
		hexRender.lineDrawList.push_back({ model->meshes[0].draw.buf.get(), &mesh.matrix,pOwner->pPalette,
			&mesh.draw.meshRec });
}
