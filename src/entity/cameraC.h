#pragma once

#include "component.h"

class CHexRender;
class CameraC : public CDerivedC<CameraC> {
public:
	CameraC(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void update(float dT);
	void setHeight(float h);

	CHexRender* pHexRender;
	float height = 1;
};