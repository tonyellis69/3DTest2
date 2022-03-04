#include "entityDraw.h"

#include <glm/glm.hpp>

#include "../entity.h"


CEntityDraw::CEntityDraw(CEntity* owner) : CDrawFunc(owner) {

}

void CEntityDraw::draw(CHexRender& hexRender) {
	for (auto& mesh : model->meshes) {
		hexRender.lineDrawList.push_back({model->meshes[0].draw.buf.get(), &mesh.matrix,pOwner->pPalette,
			&mesh.draw.meshRec});
	}


}