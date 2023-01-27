#include "robot.h"

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>


#include "utils/log.h"

#include "hexRenderer.h"

#include "gameState.h"
#include "missile.h"

#include <glm/gtx/vector_angle.hpp>

#include "sound/sound.h"

#include "utils/random.h"

#include "spawner.h"

#include "renderer/imRendr/imRendr.h"

#include "intersect.h"
#include "utils/mathsLib.h"


CRobot::CRobot() {
	isRobot = true;
	//physics.invMass = 1.0f / 80.0f; //temp!
}


void CRobot::update(float dT) {
	CEntity::update(dT);

}


void CRobot::receiveDamage(CEntity& attacker, int damage) {
	hp--;
	if (hp == 0) {
		game.killEntity(*this);
		spawn::explosion("explosion", getPos(), 1.5f);

		//make a random angle
		float angle = rnd::rand(rad360);
		glm::vec3 v = angle2vec(angle) * 5.0f;

		auto drop = spawn::drop("drop", getPos());
		drop->phys->velocity = v;
		drop->tmpId = 57;

	}
	else {
		if ( ((CRoboState*)ai.get())->canSeeEnemy() == false) {
			ai = std::make_shared<CTurnToSee>(this, glm::normalize(attacker.getPos() - getPos()));
		}
	}
}


void CRobot::fireMissile(CEntity* target) {
	glm::vec3 targetVec = target->getPos() - getPos();
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	auto missile = (CMissile*) spawn::missile("missile",getPos(), targetAngle).get();
	missile->setOwner(this);
	missile->setSpeed(15);// 7.0f);

	snd::play("shoot");
}


