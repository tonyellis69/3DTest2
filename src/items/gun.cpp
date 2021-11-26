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

void CGun::draw() {
	//hexRendr2.drawLineModel(model.meshes[0]);

	hexRendr2.drawSolidModel(model.meshes[0]);
	hexRendr2.drawSolidModel(model.meshes[1]);
	hexRendr2.drawLineModel(model.meshes[2]);
}






//!!!Different gun types:

void CSmallGun::fire(float firingAngle) {
	auto missile = (CMissile*) spawn::missile("missile", gun->parent->worldPos, firingAngle).get();
	missile->setOwner(gun->parent);

	snd::play("shoot");
}
