#pragma once

#include "graphics.h"

#include "../entity/entity.h"

struct vc2 {
	glm::vec3 v;
	int c = 0;
};

class CDestinationGraphic : public CGraphic {
public:
	CDestinationGraphic();
	void update(float dT);
	void draw(CHexRender& hexRender);



	glm::vec3* dest = nullptr;

	std::vector<vc2> verts;
	CBuf2 buf;
	TMeshRec mesh;
	glm::mat4 matrix;
	std::vector<glm::vec4>* pPalette;
};