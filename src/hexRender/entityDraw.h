#pragma once

#include "drawFunc.h"


class CEntity;
class CEntityDraw : public CDrawFunc {
public:
	CEntityDraw(CEntity* owner);
	void draw(CHexRender& hexRender);

};