#include "drawFunc.h"

#include "../entity/entity.h"

CDrawFunc::CDrawFunc(CEntity* owner) {
	pOwner = owner;
	//model = &owner->model;
}
