#pragma once

#include <glm/glm.hpp>
#include <string>
#include <functional>

#include "entity.h"
#include "missile.h"
#include "robot.h"
#include "explosion.h"

#include "messaging/msg2.h"

const int spwnMsg = 1000;

/** A class to tidy away the creation of entities. */
class CSpawn {
public:
	static CRobot* robot(const std::string& name, glm::vec3& pos);
	static CMissile* missile(const std::string& name, glm::vec3& pos, float angle);
	static CExplosion* explosion(const std::string& name, glm::vec3& pos, float scale);

	template<typename Obj, typename memberFunc >
	static void setCallback(Obj* obj, void (memberFunc::* fn)(const std::string&, TEntity)) {
		msg::attach(spwnMsg, obj, fn);
	}


};

using spawn = CSpawn;