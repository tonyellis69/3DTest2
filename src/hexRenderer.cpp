#include "hexRenderer.h"

#include "utils/log.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

CHexRenderer::CHexRenderer() : hexModel(6) {
	pRenderer = &CRenderer::getInstance();
	tmpCreateHexagonModel();


	createSolidHexModel();

	createLineShader();

	camera.setPos(glm::vec3(0, -0, 12));
	cameraStep = 0.01f;
	cameraPitch = 45;
	camera.pitch(cameraPitch);

	floorplanColour = glm::vec4(0, 1, 0, 1);

}


void CHexRenderer::start() {
	
	fillFloorplanLineBuffer();
	fillFloorplanSolidBuffer();

}

void CHexRenderer::setMap(CHexArray* hexArray){
	this->hexArray = hexArray;
}



void CHexRenderer::draw() {
	drawFloorPlan();
	drawHighlights();
	drawEntities();
}

void CHexRenderer::drawFloorPlan() {
	glm::mat4 mvp = camera.clipMatrix;
	pRenderer->setShader(lineShader);
	lineShader->setShaderValue(hMVP, mvp);
	lineShader->setShaderValue(hColour, floorplanColour);
	pRenderer->drawLineStripBuf(floorplanLineBuf);
	pRenderer->drawTriStripBuf(floorplanSolidBuf);
}

/** Draw any hey highlighting, such as the cursor. */
void CHexRenderer::drawHighlights() {
	CHexObject* cursorObj = pCallbackObj->getCursorObj();
	glm::mat4 mvp = camera.clipMatrix * cursorObj->worldMatrix;
	lineShader->setShaderValue(hColour, floorplanColour);
	lineShader->setShaderValue(hMVP, mvp);
//	pRenderer->drawTriStripBuf(solidHexBuf);


	glm::vec4 pathStartColour(0, 1, 0, 0.1f);
	glm::vec4 pathEndColour(0, 1, 0, 0.75f);
	float inc = 1.0 / cursorPath.size();  float t = 0;
	for (auto hex : cursorPath) {
		glm::mat4 worldPos = glm::translate(glm::mat4(1), hexArray->getWorldPos(hex));
		mvp = camera.clipMatrix * worldPos;
		lineShader->setShaderValue(hMVP, mvp);
		glm::vec4 pathColour = glm::mix(pathStartColour, pathEndColour, t);
		lineShader->setShaderValue(hColour, pathColour);
		pRenderer->drawTriStripBuf(solidHexBuf);
		t += inc;
	}

	THexList* path = pCallbackObj->getPath();
	pathStartColour = { 1,0, 0, 0.1f };
	pathEndColour = { 1,0, 0, 0.75f };
	 inc = 1.0 / path->size();   t = 0;
	for (auto hex : *path) {
		glm::mat4 worldPos = glm::translate(glm::mat4(1), hexArray->getWorldPos(hex));
		mvp = camera.clipMatrix * worldPos;
		lineShader->setShaderValue(hMVP, mvp);
		glm::vec4 pathColour = glm::mix(pathStartColour, pathEndColour, t);
		lineShader->setShaderValue(hColour, pathColour);
		pRenderer->drawTriStripBuf(solidHexBuf);
		t += inc;
	}




}

void CHexRenderer::drawEntities() {
	CHexObject* playerObj = pCallbackObj->getEntity();
	glm::mat4 mvp = camera.clipMatrix * playerObj->worldMatrix;
	lineShader->setShaderValue(hMVP, mvp);
	lineShader->setShaderValue(hColour, floorplanColour);
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
	for (int x = 0; x < hexArray->width; x++) {
		for (int y = 0; y < hexArray->height; y++) {
			if (hexArray->hex(x, y).content == 1) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					int v1 = vNum; int v2 = vNum + 1;
					pos = corner + hexArray->hex(x, y).position;
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
	for (int x = 0; x < hexArray->width; x++) {
		for (int y = 0; y < hexArray->height; y++) {
			if (hexArray->getHex(x, y).content == 1) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					pos = corner + hexArray->getHex(x, y).position;
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
	int vNum = 0;
	for (int x = 0; x < hexArray->width; x++) {
		for (int y = 0; y < hexArray->height; y++) {
			if (hexArray->getHex(x, y).content == 2) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					pos = corner + hexArray->getHex(x, y).position;
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
			}
		}

	}

	floorplanSolidBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	floorplanSolidBuf.storeIndex(indices.data(), indices.size());
	floorplanSolidBuf.storeLayout(3, 0, 0, 0);
}

void CHexRenderer::createSolidHexModel() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int vNum = 0;
	for (auto corner : hexModel) {
		glm::vec3  pos = corner;
		verts.push_back(pos);
	}
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(2);
	indices.push_back(5);
	indices.push_back(1);
	indices.push_back(0);
	indices.push_back(65535);


	solidHexBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3)* verts.size(), verts.size());
	solidHexBuf.storeIndex(indices.data(), indices.size());
	solidHexBuf.storeLayout(3, 0, 0, 0);
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
	hColour = lineShader->getUniformHandle("colour");
}

void CHexRenderer::setCameraAspectRatio(glm::vec2 ratio) {
	camera.setAspectRatio(ratio.x, ratio.y);
}

/** Return the hex coordinates for the given screen positon. */
CHex CHexRenderer::pickHex(int screenX, int screenY) {
	//convert to clip coordinates
	glm::vec2 screenSize = camera.getView();
	glm::vec4 clip( (2.0f * screenX) / screenSize.x - 1.0f, 1.0f - (2.0f * screenY) / screenSize.y, -1.0f, 1.0f);

	//undo perspective 
	glm::vec4 view = glm::inverse(camera.perspectiveMatrix) * clip;
	view.z = -1.0f; view.w = 0.0f;

	//undo view (camera position)
	glm::vec4 worldPos = camera.worldMatrix * view;
	glm::vec3 ray = glm::normalize(glm::vec3(worldPos));

	//this should be a ray, projecting from 0,0,0 in the given direction.

	glm::vec3 planeN(0, 0, 1); //normal of plane on which hexes lie.
	float d = 0; //distance of plane from origin
	float t = -(glm::dot(camera.getPos(), planeN) + d)
			/ glm::dot(ray, planeN); 
	//t = distance from camera to plane for this ray

	glm::vec3 p = camera.getPos() + ray * t; //extend ray to find where it hits plane.

	CHex hexPos = worldSpaceToHex(p);

	//liveLog << "\n" << hexPos.x << " " << hexPos.y << " " << hexPos.z;


	return hexPos;
}


/** Create a buffer identified by the given name, and return a pointer to it. */
CBuf* CHexRenderer::addBuffer(const std::string& name) {
	return &modelBuffers[name];
}

CBuf* CHexRenderer::getBuffer(const std::string& name) {
	return &modelBuffers[name];
}

/** Fill the structure used for drawing the path between player and cursor. */
void CHexRenderer::setCursorPath(CHex& playerPos, CHex& cursorPos) {
	cursorPath = *hexLine(playerPos, cursorPos);;
}

/** Set path directly. */
void CHexRenderer::setCursorPath(THexList& path) {
	cursorPath = path;
}
