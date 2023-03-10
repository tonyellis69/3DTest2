#include "hexQuad.h"

#include "utils/random.h"

CHexQuad::CHexQuad(glm::i32vec2& pos, glm::i32vec2& size) {
	this->pos = pos;
	this->size = size;
}

/** Recursively split this quad or its children. */
void CHexQuad::split(bool horiz) {
	if (!childA && !childB) {
		horiz = rnd::dice(2) - 1;
		if (horiz) {
			int freeSpace = size.x - 1;
			if (freeSpace < 6)
				return;

			int var = rnd::dice(freeSpace - 4);
			int newX = 2 + var;

			glm::i32vec2 newSize(newX, size.y);
			childA = std::make_shared<CHexQuad>(pos, newSize);
			glm::i32vec2 newPos(pos.x + newX, pos.y);
			newSize = { size.x - newX,size.y };
			childB = std::make_shared<CHexQuad>(newPos, newSize);
		}
		else {
			int freeSpace = size.y - 1;
			if (freeSpace < 6)
				return;
			int var = rnd::dice(freeSpace - 4);
			int newY = 2 + var;

			glm::i32vec2 newSize(size.x, newY);
			childA = std::make_shared<CHexQuad>(pos, newSize);
			glm::i32vec2 newPos(pos.x, pos.y + newY);
			newSize = { size.x,size.y - newY };
			childB = std::make_shared<CHexQuad>(newPos, newSize);

		}

	}
	else {
		childA->split(horiz);
		childB->split(!horiz);

	}

}

