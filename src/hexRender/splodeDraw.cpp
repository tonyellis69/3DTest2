#include "splodeDraw.h"

 #include "../explosion.h"


CSplodeDraw::CSplodeDraw(CEntity* owner) : CDrawFunc(owner) {


}


void CSplodeDraw::draw(CHexRender& hexRender) {
	auto explode = (CExplosion*)pOwner;

	hexRender.loadExplosionDrawList(TSplodeDraw{ &explode->worldPos,
		&explode->lifeTime, &explode->size, &explode->timeOut,
		&explode->seed, &explode->particleCount, &explode->modelCmp->model.palette });

}