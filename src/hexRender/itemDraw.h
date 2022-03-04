#pragma once

#include "drawFunc.h"


class CItemDraw : public CDrawFunc {
public:
	CItemDraw(CEntity* owner);
	void draw(CHexRender& hexRender);

private:

};