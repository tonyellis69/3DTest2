#include "entityDraw.h"

#include "graphics/shader2.h"

CEntityDraw::CEntityDraw(CModel* model) : CDrawFunc(model) {
	pLineShader = shader::get("lineModel");

	hMVP = pLineShader->getUniform("mvpMatrix");
	hColour = pLineShader->getUniform("colour");
	hWinSize = pLineShader->getUniform("winSize");
	hPalette = pLineShader->getUniform("colourPalette");
}

void CEntityDraw::draw(CHexRender& hexRender) {
	//call a hexRender draw mesh function here for each mesh
	//something like hexRender->




}