#pragma once

#include <unordered_map>

#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

//#include "gamehextObj.h"
#include "actor.h" //replaces the above

#include "IGameHexArray.h"

#include "messaging/messenger.h"


struct hex_hash {
	size_t operator()(const CHex& hex) const {
		return std::hash<glm::i32vec3>()(glm::i32vec3(hex.x, hex.y, hex.z));
	}
};
	

using TMapIt = std::unordered_multimap<CHex, CGameHexObj*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;

/** Extend the hexArray to check for game entities when
	pathfinding. */
class CGameHexArray : public CHexArray, public IGameHexArray, 
	public CMessenger{
public:
	CGameHexArray();
	void setMessageHandlers();
	void setEntityList(TEntities* pEntities);
	bool fromToBlocked(CHex& current, CHex& hex);
	bool isEmpty(glm::i32vec2& hex);
	CHex findLineEnd(CHex& start, CHex& target);
	void moveEntity(CGameHexObj* entity, CHex& hex);
	void add(CGameHexObj* entity, CHex& hex);
	TRange getEntitiesAt(CHex& hex);
	CGameHexObj* getEntityAt(CHex& hex);
	CGameHexObj* getEntityClassAt(int classId, CHex& hex);
	CGameHexObj* getBlockingEntityAt(CHex& hex);
	CGameHexObj* getEntityNotSelf(CGameHexObj* self);

	CHexActor* getRobotAt(CHex& hex);


	void updateBlocking();
	void smartBlockClear( CHex& pos);

	void onGetTravelPath(CGetTravelPath& msg);
	void onMoveEntity(CMoveEntity& msg);
	void onActorBlockCheck(CActorBlock& msg);
	void onGetLineEnd(CGetLineEnd& msg);
	void onGetActorAt(CGetActorAt& msg);

	TEntities* entities; ///<To check for collision against.

private:

	std::unordered_multimap<CHex, CGameHexObj*, hex_hash> entityMap;

};