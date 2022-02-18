#pragma once

#include "drawFunc.h"
#include "graphics/shader2.h"

class CEntityDraw : public CDrawFunc {
public:
	CEntityDraw(CModel* model);
	void draw(CHexRender& hexRender);

private:
	CShader* pLineShader;

	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;
	unsigned int hPalette;
};