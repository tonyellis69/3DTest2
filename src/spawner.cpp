#include "spawner.h"

#include "robot.h"
#include "missile.h"

/** Create the given entity, and notify the registered callback-handler. */
CRobot* CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	//robot->setLineModel("robot");
	robot->setPosition(pos);

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



