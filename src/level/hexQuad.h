#pragma once

#include <memory>
#include <glm/glm.hpp>

/** Defines a quad portion of a hexgrid. */
class CHexQuad {
public:
	CHexQuad(glm::i32vec2& pos, glm::i32vec2& size);
	void split();
	void draw();

	glm::i32vec2 pos;
	glm::i32vec2 size;
	std::shared_ptr<CHexQuad> childA;
	std::shared_ptr<CHexQuad> childB;


};