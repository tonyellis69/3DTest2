#include "multiDraw.h"

#include <glm/glm.hpp>

#include "../entity.h"


CMultiDraw::CMultiDraw(CEntity* owner) : CDrawFunc(owner) {

}

void CMultiDraw::draw(CHexRender& hexRender) {
	//for (auto& mesh : model->meshes) {
	//	if (model->meshes[0].draw.buf.get()->numVerts > 0)
	//	hexRender.lineDrawList.push_back({model->meshes[0].draw.buf.get(), &mesh.matrix,pOwner->pPalette,
	//		&mesh.draw.meshRec});
	//}

	for (auto& mesh : lowerMeshes) {
		if (model->buf.numVerts > 0)
				hexRender.lineDrawList.push_back({&model->buf, &mesh->matrix,pOwner->pPalette,
					&mesh->meshRec});
	}

	if (upperMask) {
		hexRender.maskList.push_back({ &model->buf, &upperMask->matrix, pOwner->pPalette,
			&upperMask->meshRec });
	}

	for (auto& mesh : upperMeshes) {
		if (model->buf.numVerts > 0)
			hexRender.upperLineList.push_back({ &model->buf, &mesh->matrix,pOwner->pPalette,
				&mesh->meshRec });
	}

	for (auto& mask : lowerMasks) {
		hexRender.lowerMaskList.push_back({ &model->buf, &mask->matrix, pOwner->pPalette,
		&mask->meshRec });
	}

}