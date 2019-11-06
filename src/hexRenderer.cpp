#include "hexRenderer.h"

#include "utils/log.h"

#include <cmath>


CHexRenderer::CHexRenderer() : hexModel(6) {
	pRenderer = &CRenderer::getInstance();
	tmpCreateHexModel();

	tmpCreateArray();

	fillFloorplanBuffer();

	createLineShader();

	camera.setPos(glm::vec3(0, -20, 12));
	cameraStep = 0.01f;
	cameraPitch = 45;
	camera.pitch(cameraPitch);

}



void CHexRenderer::draw() {
	glm::mat4 mvp = camera.clipMatrix;
	pRenderer->setShader(lineShader);
	lineShader->setShaderValue(hMVP, mvp);

	glEnable(GL_PRIMITIVE_RESTART);
	pRenderer->drawLineStripBuf(floorplanBuf);
	pRenderer->drawLineStripBuf(tmpModelBuf);
	glDisable(GL_PRIMITIVE_RESTART);

}

void CHexRenderer::setCallbackApp(IhexCallback* pApp) {
	pCallbackApp = pApp;
}

/** Fill the floorplan buffer with hexagons translated to worldspace. */
void CHexRenderer::fillFloorplanBuffer() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int index = 0; int vNum = 0;
	for (int x = 0; x < hexArray.width; x++) {
		for (int y = 0; y < hexArray.height; y++) {
			glm::vec3 pos;
			for (auto corner : hexModel) {
				pos = corner + hexArray.hex(x, y).position;
				verts.push_back(pos);
				indices.push_back(vNum++);
				index++;
	
			}
			indices.push_back(indices.back() - 5);
			indices.push_back(65535);
		}

	}

	floorplanBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	floorplanBuf.storeIndex(indices.data(),indices.size());
	floorplanBuf.storeLayout(3, 0, 0, 0);

}

void CHexRenderer::tmpCreateArray() {
	hexArray.init(20, 20);
}

/**	Load the line models we're going to use. */
void CHexRenderer::addModels(std::vector<CMesh>& meshes) {
	//turn models into buffer objects
	meshes[0].writeToBufferv3i(tmpModelBuf);
}




void CHexRenderer::onMouseWheel(float delta) {
	if (pCallbackApp->hexKeyNowCallback(GLFW_KEY_LEFT_SHIFT)) {
		cameraPitch += delta;
		camera.pitch(delta);
		liveLog << "\n" << cameraPitch;
	}
	else
		camera.dolly(delta);
}

void CHexRenderer::keyCheck() {
	if (pCallbackApp->hexKeyNowCallback('W')) {
		//camera.elevate(cameraStep);
		camera.translate(glm::vec3(0, cameraStep, 0));
	}
	if (pCallbackApp->hexKeyNowCallback('S')) {
		//camera.elevate(-cameraStep);
		camera.translate(glm::vec3(0, -cameraStep, 0));
	}
	if (pCallbackApp->hexKeyNowCallback('A')) {
		camera.track(-cameraStep);
	}
	if (pCallbackApp->hexKeyNowCallback('D')) {
		camera.track(cameraStep);
	}
}

void CHexRenderer::tmpCreateHexModel() {
	float radius = 1.0f;
	for (int corner = 0; corner < 6; corner++) {
		float angle = 60 * corner - 30.0f;
		angle = M_PI / 180 * angle;
		hexModel[corner] = glm::vec3(radius * cos(angle),
			radius * sin(angle),0.0f);
	}

}

void CHexRenderer::createLineShader() {
	lineShader = pRenderer->createShader("lineModel");
	hMVP = lineShader->getUniformHandle("mvpMatrix");
}

void CHexRenderer::setAspectRatio(glm::vec2 ratio) {
	camera.setAspectRatio(ratio.x, ratio.y);
}
