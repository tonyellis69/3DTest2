#include "dropAI.h"

#include <glm/glm.hpp>

#include "../entity/entity.h"
#include "../gameState.h"

void CDropAI::update(float dT) {
	if (!still) {
		if (glm::distance(oldPos, thisEntity->getPos()) > 0.001f) {
			oldPos = thisEntity->getPos();
			return;
		}
		thisEntity->removeComponent(thisEntity->phys);
		still = true;
	}

	if (game.player->live) {
		glm::vec3 toPlayer = game.player->getPos() - thisEntity->getPos();
		if (glm::length(toPlayer) < 0.1f) {
			thisEntity->destroyMe();
			//message about points
			return;
		}


		if (glm::length(toPlayer) < 2.0f) {
			toPlayer = glm::normalize(toPlayer);
			thisEntity->setPosition( thisEntity->getPos() + toPlayer * 5.0f * dT);
		}

	}
}
