#pragma once

#include <memory>

#include "level.h"

/** Creates a CLevel object with a populated, randomly generated map. */
class CRandLevel {
public:
	std::unique_ptr<CLevel>  makeLevel();
	void setSize(int x, int y) {
		size = {x,y};
	}
	void resize(int dSize);

private:
	glm::i32vec2 size = { 10,10 };

};