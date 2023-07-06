#pragma once

#include "component.h"

class CHexRender;
enum TCameraCMode  { camDefault, camZoom2fit };
class CameraC : public CDerivedC<CameraC> {
public:
	CameraC(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void update(float dT);
	void setHeight(float h);
	void setZoom2Fit(bool isOn);


	CHexRender* pHexRender;
	float height = 15;

private:
	void zoom2fit();

	TCameraCMode mode = camDefault;
	bool zoomed2Fit = false;
};