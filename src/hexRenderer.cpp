#include "hexRenderer.h"

#include "utils/log.h"

#include <cmath>


CHexRenderer::CHexRenderer() : hexModel(6) {
	pRenderer = &CRenderer::getInstance();
	tmpCreateHexagonModel();

	//TO DO: should be handed this by hexWorld, made by mapper
	tmpCreateArray();

	fillFloorplanLineBuffer();
	fillFloorplanSolidBuffer();

	createLineShader();

	camera.setPos(glm::vec3(0, -20, 12));
	cameraStep = 0.01f;
	cameraPitch = 45;
	camera.pitch(cameraPitch);

}


void CHexRenderer::start() {


}



void CHexRenderer::draw() {
	drawFloorPlan();
	drawEntities();
}

void CHexRenderer::drawFloorPlan() {
	glm::mat4 mvp = camera.clipMatrix;
	pRenderer->setShader(lineShader);
	lineShader->setShaderValue(hMVP, mvp);
	pRenderer->drawLineStripBuf(floorplanLineBuf);
	pRenderer->drawTriStripBuf(floorplanSolidBuf);
}

void CHexRenderer::drawEntities() {
	CHexObject* playerObj = pCallbackObj->getEntity();
	glm::mat4 mvp = camera.clipMatrix * playerObj->worldMatrix;
	lineShader->setShaderValue(hMVP, mvp);
	pRenderer->drawLineStripBuf(*playerObj->buf);
}



void CHexRenderer::setCallbackApp(IhexRendererCallback* pObj) {
	pCallbackObj = pObj;
}

/*
void CHexRenderer::fillFloorplanLineBuffer() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int index = 0; int vNum = 0;
	for (int x = 0; x < hexArray.width; x++) {
		for (int y = 0; y < hexArray.height; y++) {
			if (hexArray.hex(x, y).content == 1) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					int v1 = vNum; int v2 = vNum + 1;
					pos = corner + hexArray.hex(x, y).position;
					verts.push_back(pos);
					vNum++;
					indices.push_back(v1);
					indices.push_back(v2);


				}
				indices.pop_back();
				indices.push_back(vNum -6);
			}
		}
	}

	

	floorplanLineBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	floorplanLineBuf.storeIndex(indices.data(),indices.size());
	floorplanLineBuf.storeLayout(3, 0, 0, 0);
} */
/** Fill the floorplan line buffer with outline hexagons translated to worldspace. */
void CHexRenderer::fillFloorplanLineBuffer() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int index = 0; int vNum = 0;
	for (int x = 0; x < hexArray.width; x++) {
		for (int y = 0; y < hexArray.height; y++) {
			if (hexArray.hex(x, y).content == 1) {
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
	}

	floorplanLineBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	floorplanLineBuf.storeIndex(indices.data(), indices.size());
	floorplanLineBuf.storeLayout(3, 0, 0, 0);
}


/** Fill the floorplan solid buffer with solid hexagon polys translated to worldspace. */
void CHexRenderer::fillFloorplanSolidBuffer() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int index = 0; int vNum = 0;
	for (int x = 0; x < hexArray.width; x++) {
		for (int y = 0; y < hexArray.height; y++) {
			if (hexArray.hex(x, y).content == 2) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					pos = corner + hexArray.hex(x, y).position;
					verts.push_back(pos);
					vNum++;
				}
				indices.push_back(vNum - 3);
				indices.push_back(vNum - 2);
				indices.push_back(vNum - 4);
				indices.push_back(vNum - 1);
				indices.push_back(vNum - 5);
				indices.push_back(vNum - 6);
				indices.push_back(65535);
				index += 6;
			}
		}

	}

	floorplanSolidBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	floorplanSolidBuf.storeIndex(indices.data(), indices.size());
	floorplanSolidBuf.storeLayout(3, 0, 0, 0);
}

void CHexRenderer::tmpCreateArray() {
	hexArray.init(40, 40);
	hexArray.hex(10, 10).content = 2;
}



void CHexRenderer::dollyCamera(float delta) {
	camera.dolly(delta);
}

void CHexRenderer::pitchCamera(float delta) {
	cameraPitch += delta;
	camera.pitch(delta);
}

void CHexRenderer::moveCamera(const glm::vec3& move) {
	glm::vec3 newMove = move * cameraStep;
	camera.translate(newMove);
}









void CHexRenderer::tmpCreateHexagonModel() {
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

void CHexRenderer::setCameraAspectRatio(glm::vec2 ratio) {
	camera.setAspectRatio(ratio.x, ratio.y);
}
