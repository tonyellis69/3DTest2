#include "hexQuad.h"

#include "utils/random.h"

CHexQuad::CHexQuad(glm::i32vec2& pos, glm::i32vec2& size) {
	this->pos = pos;
	this->size = size;
}

/** Recursively split this quad or its children. */
void CHexQuad::split() {
	if (!childA && !childB) {
		bool horiz = rnd::dice(2) - 1;
		if (horiz) {
			int newX = size.x / 2;
			glm::i32vec2 newSize(newX, size.y);
			childA = std::make_shared<CHexQuad>(pos, newSize);
			glm::i32vec2 newPos(pos.x + newX, pos.y);
			newSize = { size.x - newX,size.y };
			childB = std::make_shared<CHexQuad>(newPos, newSize);
		}
		else {
			int newY = size.y / 2;
			glm::i32vec2 newSize(size.x, newY);
			childA = std::make_shared<CHexQuad>(pos, newSize);
			glm::i32vec2 newPos(pos.x, pos.y + newY);
			newSize = { size.x,size.y - newY };
			childB = std::make_shared<CHexQuad>(newPos, newSize);

		}

	}
	else {
		childA->split();
		childB->split();

	}

}

void CHexQuad::draw() {
	//draw lines of hexes between the four corners.

}
