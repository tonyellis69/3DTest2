#pragma once


#include "graphics.h"

#include "../entity/entity.h"

struct vc3 {
	glm::vec3 v;
	int c = 0;
};

class CAvoidGraphic : public CGraphic {
public:
	CAvoidGraphic();
	void update(float dT);
	void draw(CHexRender& hexRender);



	glm::vec3* dest = nullptr;

	std::vector<vc3> verts;
	CBuf2 buf;
	TMeshRec mesh;
	glm::mat4 matrix;
	std::vector<glm::vec4>* pPalette;
};