#pragma once

#include <glew.h>
#include <glm/glm.hpp>
#include "renderer\renderShader.h"



/** A wrapper for the terrain to texture shader. */
class CTerrain2texShader : public CRenderShader {
public:
	CTerrain2texShader() {};
	void getShaderHandles();
	void setNwSampleCorner(glm::vec2& corner);
	void setPixelScale(float pixelScale);
	
	GLuint hNwSampleCorner;
	GLuint hPixelScale;


};