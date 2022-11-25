#include "collider.h"

#include "entity.h"


std::tuple<bool, glm::vec3> ColliderCmp::segCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
    return parentEntity->collisionCheck(segA, segB);
}

std::tuple<bool, glm::vec3> ColliderCmp::entCollisionCheck(CEntity* ent2) {
    return parentEntity->collisionCheck(ent2);
}
