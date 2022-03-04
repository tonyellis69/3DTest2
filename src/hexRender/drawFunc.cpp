#include "drawFunc.h"

#include "../entity.h"

CDrawFunc::CDrawFunc(CEntity* owner) {
	pOwner = owner;
	model = &owner->model;
}
