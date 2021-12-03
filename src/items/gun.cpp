#include "gun.h"

#include "..\spawner.h"
#include "sound/sound.h"


#include <glm/gtc/matrix_transform.hpp>	

CGun::CGun() {
	entityType = entGun;
}

void CGun::fire(float firingAngle) {
	gunType->fire(firingAngle);
}

void CGun::drop() {
	CPlayerObject* player = (CPlayerObject*) parent;
	if (player->gun == this)
		player->gun = nullptr;
	parent = nullptr;
}

std::string CGun::getShortDesc() {
	if (parent) {
		CPlayerObject* player = (CPlayerObject*)parent;
		if (player->gun == this)
			return CItem::getShortDesc() + " (equipped)";
	}
	return CItem::getShortDesc();
}








//!!!Different gun types:

void CSmallGun::fire(float firingAngle) {
	auto missile = (CMissile*) spawn::missile("missile", gun->parent->worldPos, firingAngle).get();
	missile->setOwner(gun->parent);

	snd::play("shoot");
}
