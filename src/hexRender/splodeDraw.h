#pragma once

#include "drawFunc.h"

class CSplodeDraw : public CDrawFunc {
public:
	CSplodeDraw(CEntity* owner);
	void draw(CHexRender& hexRender);



};