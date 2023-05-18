#pragma once

#include "hexRender.h"

class CEntity;
/** The basic draw-me component for entities. */
class CDrawFunc {
public:
	CDrawFunc() {};
	CDrawFunc(CEntity* owner);
	virtual void draw(CHexRender& hexRender) {}


//protected:
	CModel* model;
	CEntity* pOwner;

	bool visible = true;
};