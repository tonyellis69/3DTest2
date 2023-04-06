#include "botHealthC.h"

#include "entity.h"

#include "../gameState.h"
#include "../spawner.h"
#include "utils/random.h"
#include "utils/mathsLib.h"

#include "../roboState.h"

void CBotHealthC::receiveDamage(CEntity& attacker, int damage) {
	hp--;
	if (hp == 0) {
		game.killEntity(*thisEntity);
		spawn::explosion("explosion", thisEntity->getPos(), 1.5f);

		//make a random angle
		float angle = rnd::rand(rad360);
		auto drop = spawn::drop("drop", thisEntity->getPos());
		glm::vec3 v = angle2vec(angle);
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);
		drop->tmpId = 57;

		drop = spawn::drop("drop", thisEntity->getPos());
		v = angle2vec(angle + rad120 + rnd::rand(-rad20, rad20));
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);

		drop = spawn::drop("drop", thisEntity->getPos());
		v = angle2vec(angle - rad120 + rnd::rand(-rad20, rad20));
		drop->phys->velocity = v * 5.0f + rnd::rand(2.0f);



	}
	else {
		if (((CRoboState*)thisEntity->ai.get())->canSeeEnemy() == false) {
			thisEntity->ai = std::make_shared<CTurnToSee>(thisEntity, glm::normalize(attacker.getPos() - thisEntity->getPos()));
		}
	}

}
