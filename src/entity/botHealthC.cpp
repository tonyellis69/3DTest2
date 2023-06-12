#include "botHealthC.h"

#include "entity.h"

#include "../gameState.h"

#include "utils/random.h"
#include "utils/mathsLib.h"

#include "../roboState.h"

void CBotHealthC::receiveDamage(CEntity& attacker, int damage) {
	hp--;
	if (hp == 0) {
		gameWorld.killEntity(*thisEntity);
		gameWorld.spawn("explosion", thisEntity->getPos(), 1.5f);

		//make a random angle
		float angle = rnd::rand(rad360);
		auto drop = gameWorld.spawn("drop", thisEntity->getPos());
		glm::vec3 v = angle2vec(angle);
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);
		drop->tmpId = 57;

		drop = gameWorld.spawn("drop", thisEntity->getPos());
		v = angle2vec(angle + rad120 + rnd::rand(-rad20, rad20));
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);

		drop = gameWorld.spawn("drop", thisEntity->getPos());
		v = angle2vec(angle - rad120 + rnd::rand(-rad20, rad20));
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);



	}
	else {
		if (((CRoboState*)thisEntity->ai)->canSeeEnemy() == false) {
			thisEntity->addComponent<CTurnToSee>(glm::normalize(attacker.getPos() - thisEntity->getPos()));
		}
	}

}
