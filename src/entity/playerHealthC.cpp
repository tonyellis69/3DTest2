
#include "playerHealthC.h"

#include "entity.h"

#include "listen/listen.h"
#include  "../gameEvent.h"

#include "../items/shield.h"

#include "../gameState.h"

#include "utils/log.h"

void CPlayerHealthC::receiveDamage(CEntity& attacker, int damage) {
	if (thisEntity->playerC->shield) {
		CShieldComponent* playerShield = (CShieldComponent*)thisEntity->playerC->shield->item;
		damage = playerShield->absorbDamage(damage);
		if (damage <= 0)
			return;
	}


	liveLog << "\nPlayer hit!";

	int finalDamage = damage;// armour->reduceDamage(damage);

	hp -= finalDamage;

	if (hp < 1) {
		//game.level->removeEntity(this);
		thisEntity->playerC->dead = true;
		thisEntity->visible = false;
		//game.onPlayerDeath();
		gameWorld.spawn("explosion", thisEntity->getPos(), 1.5f);

		CGameEvent e;
		e.type = gamePlayerDeath;
		lis::event(e);

	}
}