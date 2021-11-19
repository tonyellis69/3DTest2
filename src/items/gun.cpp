#include "gun.h"

#include "..\spawner.h"
#include "sound/sound.h"

#include "..\3Dtest\src\hexRenderer.h"
#include <glm/gtc/matrix_transform.hpp>	

CGun::CGun() {
	entityType = entGun;
}

void CGun::fire(float firingAngle) {
	gunType->fire(firingAngle);
}






//!!!Different gun types:

void CSmallGun::fire(float firingAngle) {
	auto missile = (CMissile*) spawn::missile("missile", gun->parent->worldPos, firingAngle).get();
	missile->setOwner(gun->parent);

	snd::play("shoot");
}
