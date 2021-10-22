#include "spawner.h"

#include "robot.h"
#include "missile.h"

std::unordered_map< std::string, CBuf2> CSpawn::meshBufs;
std::unordered_map< std::string, TModelData> CSpawn::modelBufs;



CPlayerObject* CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	setEntityLineModel(player.get(), name);
	player->setPosition(pos);

	player->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	msg::emit(spwnMsg, name, player);
	return player.get();
}

/** Create the given entity, and notify the registered callback-handler. */
CRobot* CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	setEntityLineModel(robot.get(), "robot");
	robot->setPosition(pos);

	if (name == "melee bot") {
		robot->setState(robotWander3);
		robot->entityType = entMeleeBot;
	}
	else if (name == "shooter bot") {
		robot->setState(robotWander3);
		robot->entityType = entShootBot;
	}


	robot->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	msg::emit(spwnMsg, name, robot);
	return robot.get();
}

CMissile* CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	setEntityLineModel(missile.get(), "bolt");
	missile->setPosition(pos, angle);

	missile->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	msg::emit(spwnMsg, name, missile);
	return missile.get();
}

CExplosion* CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);
	explode->worldPos = pos;

	msg::emit(spwnMsg, name, explode);
	return explode.get();
}


void CSpawn::setEntityLineModel(CEntity* entity, const std::string& modelName) {
	entity->lineModel.buffer2 = &meshBufs[modelName];
	entity->lineModel.model = modelBufs[modelName];
}



