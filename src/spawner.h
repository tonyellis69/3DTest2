#pragma once

#include <glm/glm.hpp>
#include <string>

#include "entity\entity.h"



/** A class to tidy away the creation of entities. */
class CSpawn {
public:
	static CEntity* player(const std::string& name, glm::vec3& pos);
	static CEntity* robot(const std::string& name, glm::vec3& pos);
	static CEntity* missile(const std::string& name, glm::vec3& pos, float angle);
	static CEntity* explosion(const std::string& name, glm::vec3& pos, float scale);
	static CEntity* gun(const std::string& name, glm::vec3& pos = glm::vec3( 0,0,0 ));
	static CEntity* armour(const std::string& name, glm::vec3& pos = glm::vec3(0, 0, 0));

	static CEntity* shield(const std::string& name);

	static CEntity* drop(const std::string& name, glm::vec3& pos);

	static CEntity* mainCamera(const std::string& name, glm::vec3& pos);



};
	
using spawn = CSpawn;