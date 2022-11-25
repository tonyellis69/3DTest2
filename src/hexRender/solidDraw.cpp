#include "solidDraw.h"

CSolidDraw::CSolidDraw(CEntity* owner) : CDrawFunc(owner) {

}

void CSolidDraw::draw(CHexRender& hexRender) {
	auto& mesh = model->meshes[0];
	
	hexRender.loadSolidList(TSolidDraw{ &model->buf, mesh.matrix,&model->palette,
		&mesh.meshRec });

	//hexRender.loadLineList(TLineDraw{ &model->buf, mesh.matrix, &model->palette,
	//&mesh.meshRec });




}

