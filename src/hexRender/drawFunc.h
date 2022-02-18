#pragma once

#include "hexRender.h"

/** The basic draw-me component for entities. */
class CDrawFunc {
public:
	CDrawFunc() {};
	CDrawFunc(CModel* model);
	virtual void draw(CHexRender& hexRender) {}


protected:
	CModel* model;

};