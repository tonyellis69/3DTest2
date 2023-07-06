#include "cameraC.h"

#include "listen/listen.h"

#include "../hexEngineEvent.h"

#include "../hexRender/hexRender.h"
#include "entity.h"

#include "../gameState.h"

void CameraC::onAdd() {
	CEngineEvent e(engGetRenderer);

	lis::event(e);

	pHexRender = e.pHexRender;

}

void CameraC::update(float dT) {
	glm::vec3 pos = thisEntity->getPos();
	pos.z = height;
	pHexRender->setCameraPos(pos);
	if (mode == camZoom2fit && !zoomed2Fit)
		zoom2fit();
}

void CameraC::setHeight(float h) {
	height = h;
}

void CameraC::setZoom2Fit(bool isOn) {
	mode = camZoom2fit;
	zoomed2Fit = false;
}

void CameraC::zoom2fit() {
	glm::vec3 gridTL = abs(cubeToWorldSpace(gameWorld.level.indexToCube(glm::i32vec2{ 0, 0 })));
	glm::vec3 gridBR = abs(cubeToWorldSpace(gameWorld.level.indexToCube(gameWorld.level.getGridSize())));

	glm::i32vec2 scnSize = pHexRender->getScreenSize();
	glm::vec3 scnTL = abs(pHexRender->screenToWS(0, 0));
	glm::vec3 scnBR = abs(pHexRender->screenToWS(scnSize.x, scnSize.y));

	glm::vec3 margin(2, 2, 0);

	if (glm::any(glm::greaterThan(gridTL + margin, scnTL))) {
		//pHexRender->dollyCamera(-4.0f);
		height += 4;
	}
	else	if (glm::all(glm::lessThanEqual(gridTL, scnTL - margin * 1.5f))) {
		//pHexRender->dollyCamera(1.0f);
		height--;
	}
	else
		zoomed2Fit = false;
}