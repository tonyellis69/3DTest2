#pragma once

#define _USE_MATH_DEFINES //for cmath

#include "drawFunc.h"


class CSolidDraw : public CDrawFunc {
public:
	CSolidDraw(CEntity* owner);
	void draw(CHexRender& hexRender);

private:

};