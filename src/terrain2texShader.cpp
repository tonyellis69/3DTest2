
#include "renderer/renderer.h"
#include "terrain2texShader.h"

void CTerrain2texShader::getShaderHandles() {
	pRenderer->setShader(hShader);
	hNwSampleCorner = pRenderer->getShaderDataHandle(hShader,"nwSampleCorner");
	hPixelScale = pRenderer->getShaderDataHandle(hShader, "pixelScale");
	
	
}

void CTerrain2texShader::setNwSampleCorner(glm::vec2 & corner) {
	pRenderer->setShaderValue(hNwSampleCorner, 1, corner);
}

void CTerrain2texShader::setPixelScale(float pixelScale) {
	pRenderer->setShaderValue(hPixelScale, 1, pixelScale);
}
