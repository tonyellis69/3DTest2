#pragma once

#include <memory>

#include "level.h"
#include "hexQuad.h"

/** Creates a CLevel object with a populated, randomly generated map. */
class CRandLevel {
public:
	std::unique_ptr<CLevel>  makeLevel();
	void setSize(int x, int y) {
		size = {x,y};
	}
	void resize(int dSize);
	void draw(CHexQuad* quad);
	void subdivide();

private:
	void fillLine(glm::i32vec2& a, glm::i32vec2& b);

	std::unique_ptr<CLevel> level;
	glm::i32vec2 size = { 30,30 };
	std::shared_ptr<CHexQuad> rootQuad;

};