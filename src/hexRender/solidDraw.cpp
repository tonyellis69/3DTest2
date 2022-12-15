#include "solidDraw.h"

CSolidDraw::CSolidDraw(CEntity* owner) : CDrawFunc(owner) {

}

void CSolidDraw::draw(CHexRender& hexRender) {
	
	auto& pModel  = pOwner->modelCmp->model;
	auto& pMesh =  pModel.meshes[0];
	
	//hexRender.loadSolidList(TSolidDraw{ &model->buf, mesh.matrix,&model->palette,
	//	&mesh.meshRec });

	hexRender.loadSolidList(TSolidDraw{ &pModel.buf, pMesh.matrix,&pModel.palette,
	&pMesh.meshRec });

}

