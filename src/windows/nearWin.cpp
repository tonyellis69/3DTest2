#include "nearWin.h"

#include <glm/glm.hpp>

#include "../gameState.h"
#include "../gameGui.h"

void CNearWin::update(float dT)
{
	return;
	//scraping so scrap

	if (game.player == nullptr)
		return;

	float nearDist = 1.0f;
	bool nearItemsChanged = false;
	glm::vec3 playerPos = game.player->getPos();

	//check if any items are no longer near
	for (auto& item = nearItems.begin(); item != nearItems.end();) {
		if (glm::distance(playerPos, (*item)->getPos()) > nearDist ) {
			item = nearItems.erase(item);
			nearItemsChanged = true;
		}
		else
			item++;
	}

	//add any newly near items
	//TO DO: if we're looping through all entities/items every tick, may as well 
	//clear and rebuild list every time.
	for (auto& entity : game.map->entities) {
		if (entity->isItem && ((CItem*)entity.get())->parent == nullptr
			&& glm::distance(playerPos, entity->getPos()) < nearDist) {
			if (std::find(nearItems.begin(), nearItems.end(), (CItem*)entity.get()) == nearItems.end()) {
				nearItems.push_back((CItem*)entity.get());
				nearItemsChanged = true;
			}
		}
	}

	if (nearItemsChanged) 
		refresh();
}

void CNearWin::removeItem(int itemNo) {
	for (auto& item = nearItems.begin(); item != nearItems.end(); item++) {
		if ((*item)->id == itemNo) {
			nearItems.erase(item);
			refresh();
			return;
		}
	}
}

void CNearWin::refresh() {
		gWin::pNear->clearText();
		for (auto& item : nearItems) {
			gWin::pNear->addText(item->getShortDesc() + "\n");
		}
	}
