#include "cameraC.h"

#include "listen/listen.h"

#include "../hexEngineEvent.h"

#include "../hexRender/hexRender.h"
#include "entity.h"

void CameraC::onAdd() {
	CEngineEvent e(engGetRenderer);

	lis::event(e);

	pHexRender = e.pHexRender;

}

void CameraC::update(float dT) {
	glm::vec3 pos = thisEntity->getPos();
	pos.z = height;
	pHexRender->setCameraPos(pos);

}

void CameraC::setHeight(float h) {
	height = h;
}
