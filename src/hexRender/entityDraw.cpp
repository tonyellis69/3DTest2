#include "entityDraw.h"

#include <glm/glm.hpp>

#include "../entity/entity.h"


CEntityDraw::CEntityDraw(CEntity* owner) : CDrawFunc(owner) {

}

void CEntityDraw::draw(CHexRender& hexRender) {
	for (auto& mesh : model->meshes) {
		if (model->buf.numVerts > 0)
		//hexRender.lineDrawList.push_back({&model->buf, mesh.matrix, &model->palette,
		//	&mesh.meshRec});
		hexRender.loadLineList(TLineDraw{ &model->buf, mesh.matrix, &model->palette,
			&mesh.meshRec });

	}


}