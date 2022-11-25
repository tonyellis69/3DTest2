#include "multiDraw.h"

#include <glm/glm.hpp>

#include "../entity/entity.h"


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
/*				hexRender.lineDrawList.push_back({&model->buf, mesh->matrix,&model->palette,
					&mesh->meshRec})*/;
				hexRender.loadLineList(TLineDraw{ &model->buf, mesh->matrix,&model->palette,
					&mesh->meshRec });
	}

	if (upperMask) {
		hexRender.loadUpperMaskList(TSolidDraw{ &model->buf, upperMask->matrix, &model->palette,
			&upperMask->meshRec });
	}

	for (auto& mesh : upperMeshes) {
		if (model->buf.numVerts > 0)
			hexRender.loadUpperLineList(TLineDraw{ &model->buf, mesh->matrix,&model->palette,
				&mesh->meshRec });
	}

	for (auto& mask : lowerMasks) {
		hexRender.loadLowerMaskList(TSolidDraw{ &model->buf, mask->matrix, &model->palette,
		&mask->meshRec });
	}

}