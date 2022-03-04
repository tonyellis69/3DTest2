#pragma once

//#include "hexRender.h"

/** Basic 'info graphic' class, for arrows, boxes etc. */
class CHexRender;
class CGraphic {
public:
	CGraphic() {};
	virtual void update(float dT) {}
	virtual void draw(CHexRender& hexRender) {}



};