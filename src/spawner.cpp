#include "spawner.h"

#include "robot.h"
#include "missile.h"

CPlayerObject* CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	player->setPosition(pos);

	msg::emit(spwnMsg, name, player);
	return player.get();
}

/** Create the given entity, and notify the registered callback-handler. */
CRobot* CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	robot->setPosition(pos);

	if (name == "melee bot") {
		robot->setState(robotWander3);
		robot->entityType = entMeleeBot;
	}
	else if (name == "shooter bot") {
		robot->setState(robotWander3);
		robot->entityType = entShootBot;
	}

	msg::emit(spwnMsg, name, robot);
	return robot.get();
}

CMissile* CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->setPosition(pos, angle);

	msg::emit(spwnMsg, name, missile);
	return missile.get();
}

CExplosion* CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);
	explode->worldPos = pos;

	msg::emit(spwnMsg, name, explode);
	return explode.get();
}



