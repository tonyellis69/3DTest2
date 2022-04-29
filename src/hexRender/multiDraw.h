#pragma once

#include "drawFunc.h"


class CEntity;
class CMultiDraw : public CDrawFunc {
public:
	CMultiDraw(CEntity* owner);
	void draw(CHexRender& hexRender);

	std::vector<TModelMesh*> lowerMeshes;
	TModelMesh* upperMask = NULL;
	std::vector<TModelMesh*> upperMeshes;
};