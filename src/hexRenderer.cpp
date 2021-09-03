#include "hexRenderer.h"


#include "renderer/renderer.h"

CHexRenderer hexRendr2;

#include "utils/log.h"

#include "UI/GUIstyleSheet.h"

#include "../3DTest/src/gameState.h"

#include <cmath>
#include <numeric>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp> 


CHexRenderer::CHexRenderer() : hexModel(6) {

}

/** Essential setup. */
void CHexRenderer::init() 
{
	tmpCreateHexagonModel();
	createSolidHexModel();

	createLineShader();
	createHexShader();
	createVisibilityShader();
	createExplosionShader();

	camera.setNearFar(0.1f, 1000.0f);
	camera.setPos(glm::vec3(0, -0, 12));
	//cameraPitch = 45;
	//camera.pitch(cameraPitch);
	followCam = false;

	floorplanLineColour = glm::vec4(0.3, 1, 0.3, 1);
	floorplanSpaceColour = glm::vec4(0.6431, 0.7412, 0.9882, 0.03);
	floorplanSpaceColour = glm::vec4(0, 0, 0, 1);
	floorplanSolidColour = glm::vec4(0, 0.65f, 0, 1);
	floorplanSolidColour = glm::vec4(0, 0.47f, 0.16f, 1);
	solidHex = &lineModels["solidHex"];


	std::vector<glm::vec3> lineVerts = { {-0.1,0,0}, {0,0,0}, {1,0,0}, {1.1,0,0} };
	std::vector<unsigned int> index = { 0,1,2,3 };
	unitLineBuf.storeVerts(lineVerts, index, 3);

	std::vector<glm::vec3> dummyVerts(90, glm::vec3(0));
	std::vector<unsigned int> dummyIndex(90);
	std::iota(std::begin(dummyIndex), std::end(dummyIndex), 0); // Fill with 0, 1, ..., 99.
	explosionBuf.storeVerts(dummyVerts, dummyIndex, 1);
}


void CHexRenderer::setMap(CHexArray* hexArray){
	this->hexArray = hexArray;
	
	fillFloorplanLineBuffer();
	fillFloorplanSolidBuffer(floorplanSolidBuf, 2, 1);
	fillFloorplanSolidBuffer(floorplanSpaceBuf, 1, 0.9f);

	createFogBuffer(hexArray->width,hexArray->height);

	//updateHexShaderBuffer();
	updateMapVerts();
}




void CHexRenderer::drawFloorPlan() {
	renderer.setShader(lineShader);
	glm::mat4 mvp = camera.clipMatrix;
	lineShader->setShaderValue(hMVP, mvp);
	
	glDisable(GL_DEPTH_TEST);
	//draw inner hex for empty hexes
	//lineShader->setShaderValue(hColour, floorplanSpaceColour);

	//draw filled hexes for solid hexes
	hexSolidShader->setShaderValue(hColour, floorplanSolidColour);

	//draw hex wireframe grid
	lineShader->setShaderValue(hColour, floorplanLineColour);



	//!!!!!!!!!!!!!!!!!!temp hardcoding
	if (hexArray->effectsNeedUpdate) {
		updateFogBuffer();
		hexArray->effectsNeedUpdate = false;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, hFogTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, hFogBuffer);
		unsigned int err = glGetError();

	}

	renderer.setShader(hexLineShader);
	glUniform1i(hFogTexUniform, 1); //!!!!!0 to turn back on
 
	renderer.setShader(visibilityShader);
	glUniform1i(hEffectsTexUniformv, 0);

	renderer.setShader(hexSolidShader);
	hexSolidShader->setShaderValue(hHexMVPs, mvp);
	hexSolidShader->setShaderValue(hGridSizes, glm::i32vec2(hexArray->width, hexArray->height));
	renderer.drawPointsBuf(hexShaderBuf, 0, hexShaderBuf.numElements);

	renderer.setShader(hexLineShader);
	hexLineShader->setShaderValue(hHexMVP, mvp);
	hexLineShader->setShaderValue(hGridSize, glm::i32vec2(hexArray->width, hexArray->height));
	renderer.drawPointsBuf(hexShaderBuf, 0, hexShaderBuf.numElements);


	renderer.setShader(visibilityShader);
	visibilityShader->setShaderValue(hHexMVPv, mvp);
	visibilityShader->setShaderValue(hGridSizev, glm::i32vec2(hexArray->width, hexArray->height));
	//!!!temporarily disabled to see whole map
	//renderer.drawPointsBuf(hexShaderBuf, 0, hexShaderBuf.numElements);




	glEnable(GL_DEPTH_TEST);
}


void CHexRenderer::drawPath(THexList* path, glm::vec4& pathStartColour, glm::vec4& pathEndColour) {
	renderer.setShader(lineShaderBasic);
	glm::mat4 mvp(1);
	float inc = 1.0 / path->size();  float t = 0;
	for (auto& hex : *path) {
		glm::mat4 worldPos = glm::translate(glm::mat4(1), hexArray->getWorldPos(hex));
		mvp = camera.clipMatrix * worldPos;
		lineShaderBasic->setShaderValue(hMVPb, mvp);
		glm::vec4 pathColour = glm::mix(pathStartColour, pathEndColour, 1 - t);
		lineShaderBasic->setShaderValue(hColourb, pathColour);
		renderer.drawTriStripBuf(solidHexBuf);
		t += inc;
	}
}


void CHexRenderer::drawLineModel(CLineModel& lineModel) {
	TModelNode& node = lineModel.model;
	drawNode2(node, glm::mat4(1), lineModel.buffer2);
}

/** Point the camera in the given direction. Eg, top-down. */
void CHexRenderer::pointCamera(glm::vec3& dir) {
	camera.lookAt(dir);
}

void CHexRenderer::setCameraHeight(float z) {
	glm::vec3 newPos = camera.getPos();
	newPos.z = z;
	camera.setPos(newPos);
}

void CHexRenderer::setCameraPos(glm::vec3& pos) {
	camera.setPos(pos);
}

void CHexRenderer::setCameraPitch(float pitch) {
	cameraPitch = pitch;
	camera.pitch(pitch);
}

//void CHexRenderer::drawNode(TModelNode& node, glm::mat4& parentMatrix, CBuf* buf) {
//	glm::mat4 mvp = camera.clipMatrix * node.matrix *parentMatrix;// **drawData.worldMatrix;
//	lineShader->setShaderValue(hMVP, mvp);
//
//	for (auto mesh : node.meshes) {
//		lineShader->setShaderValue(hColour, mesh.colour);
//		renderer.drawLinesRange(mesh.indexStart, mesh.indexSize, *buf);
//	}
//
//	for (auto subNode : node.subNodes)
//		drawNode(subNode, node.matrix * parentMatrix, buf);
//
//}

void CHexRenderer::drawNode2(TModelNode& node, glm::mat4& parentMatrix, CBuf2* buf) {
	renderer.setShader(lineShader);
	glm::mat4 mvp = camera.clipMatrix * node.matrix * parentMatrix;
	lineShader->setShaderValue(hMVP, mvp);
	lineShader->setShaderValue(hWinSize, camera.getView());

	for (auto& mesh : node.meshes) {
		lineShader->setShaderValue(hColour, mesh.colour);
	
		if (mesh.isLine) {//TO DO: ugh, try to avoid
			renderer.drawLineStripAdjBuf(*buf, (void*)(mesh.indexStart * sizeof(unsigned short)), mesh.indexSize);
			//renderer.drawLinesBuf(*buf, (void*)(mesh.indexStart * sizeof(unsigned short)), mesh.indexSize);
		}
		else
			renderer.drawTrisBuf(*buf, (void*)(mesh.indexStart * sizeof(unsigned short)), mesh.indexSize);

	}

	for (auto& subNode : node.subNodes)
		drawNode2(subNode, node.matrix * parentMatrix, buf);

}

/** Create a buffer for holding every hex's fog-of-war status. (And 
	maybe other things. */
void CHexRenderer::createFogBuffer(int w, int h) {
	//delete any existing buffer
	glDeleteBuffers(1,&hFogBuffer);
	glDeleteBuffers(1,&hFogTex);

	glGenBuffers(1, &hFogBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, hFogBuffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(TMapEffects) * w * h, NULL, GL_DYNAMIC_DRAW);

	glGenTextures(1, &hFogTex);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

}

/** Update the data in the fog data buffer supplied to shaders. */
void CHexRenderer::updateFogBuffer() {
	glBindBuffer(GL_TEXTURE_BUFFER, hFogBuffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(TMapEffects) * hexArray->effectsData.size(), hexArray-> effectsData.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

}

/** Create a vertex buffer with data on each map hex. Eg, index and 
	scenery type. */
void CHexRenderer::updateMapVerts() {
	std::vector<unsigned int> hexIndices;
	struct THexElem {
		glm::i32vec2 v;
		unsigned int content = 0;
	};
	std::vector<THexElem> shaderVerts;
	int index = 0; int vNum = 0; int hexIndex = 0;
	for (int y = 0; y < hexArray->height; y++) {
		for (int x = 0; x < hexArray->width; x++) {
			unsigned int content = hexArray->getHexOffset(x, y).content;
			shaderVerts.push_back({ {x,y},content });
			hexIndices.push_back(hexIndex++);
		}
	}

	hexShaderBuf.storeVerts(shaderVerts, hexIndices, 2, 1);
}

/** Draw a simple line between the two points. */
void CHexRenderer::drawSightLine(glm::vec3& posA, glm::vec3& posB) {
	glm::vec3 lineVec = posB - posA;

	float angle = glm::orientedAngle(glm::normalize(lineVec), glm::vec3(1, 0, 0), glm::vec3(0,0,-1));

	glm::mat4 rot = glm::rotate((angle), glm::vec3(0, 0, 1));

	glm::mat4 trans = glm::translate(glm::mat4(1), posA) * rot;

	glm::mat4 scale =  glm::scale(trans, glm::vec3(glm::length(lineVec),0,0)) ;

	renderer.setShader(lineShader);
	glm::mat4 mvp = camera.clipMatrix * scale;
	lineShader->setShaderValue(hMVP, mvp);
	lineShader->setShaderValue(hWinSize, camera.getView());

	lineShader->setShaderValue(hColour,glm::vec4(1,0,0,1));

	renderer.drawLineStripAdjBuf(unitLineBuf, 0, 4);
}

void CHexRenderer::drawExplosion( glm::vec3& pos, float& lifeTime, float& size, float& timeOut) {
	renderer.setShader(explosionShader);
	glm::mat4 mvp = camera.clipMatrix;
	explosionShader->setShaderValue(hExpMVP, mvp);
	explosionShader->setShaderValue(hPos, pos);
	explosionShader->setShaderValue(hLifeTime, lifeTime);
	explosionShader->setShaderValue(hSize, size);
	explosionShader->setShaderValue(hTimeOut, timeOut);
	renderer.drawPointsBuf(explosionBuf, 0, explosionBuf.numElements);
}



/** Fill the floorplan line buffer with outline hexagons translated to worldspace. */
void CHexRenderer::fillFloorplanLineBuffer() {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices; 	
	int index = 0; int vNum = 0; int hexIndex = 0;
	for (int y = 0; y < hexArray->height; y++) {
		for (int x= 0; x < hexArray->width; x++) {
			if (hexArray->getHexOffset(x, y).content != 0) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					pos = corner + hexArray->getHexOffset(x, y).position;
					verts.push_back(pos);
					indices.push_back(vNum++);
					index++;
			

				}
				indices.push_back(indices.back() - 5);
				indices.push_back(65535);
			}
		}
	}

	//floorplanLineBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	//floorplanLineBuf.storeIndex(indices.data(), indices.size());
	//floorplanLineBuf.storeLayout(3, 0, 0, 0);

	floorplanLineBuf.storeVerts(verts, indices, 3);

}


/** Fill the floorplan solid buffer with solid hexagon polys translated to worldspace. */
void CHexRenderer::fillFloorplanSolidBuffer(CBuf2& buf, int drawValue, float scale) {
	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
	int vNum = 0;
	for (int x = 0; x < hexArray->width; x++) {
		for (int y = 0; y < hexArray->height; y++) {
			if (hexArray->getHexOffset(x, y).content == drawValue) {
				glm::vec3 pos;
				for (auto corner : hexModel) {
					pos = (corner * scale) + hexArray->getHexOffset(x, y).position;
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

	//buf.storeVertexes((void*)verts.data(), sizeof(glm::vec3) * verts.size(), verts.size());
	//buf.storeIndex(indices.data(), indices.size());
	//buf.storeLayout(3, 0, 0, 0);

	buf.storeVerts(verts, indices, 3);
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


	//solidHexBuf.storeVertexes((void*)verts.data(), sizeof(glm::vec3)* verts.size(), verts.size());
	//solidHexBuf.storeIndex(indices.data(), indices.size());
	//solidHexBuf.storeLayout(3, 0, 0, 0);

	solidHexBuf.storeVerts(verts, indices, 3);
}





void CHexRenderer::dollyCamera(float delta) {
	camera.dolly(delta);
}

void CHexRenderer::pitchCamera(float delta) {
	cameraPitch += delta;
	camera.pitch(delta);
}

void CHexRenderer::moveCamera(glm::vec3& move) {
	camera.translate(move);
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
	lineShader = renderer.createShader("lineModel");
	hMVP = lineShader->getUniformHandle("mvpMatrix");
	hColour = lineShader->getUniformHandle("colour");
	hWinSize = lineShader->getUniformHandle("winSize");

	lineShaderBasic = renderer.createShader("lineModelBasic");
	hMVPb = lineShaderBasic->getUniformHandle("mvpMatrix");
	hColourb = lineShaderBasic->getUniformHandle("colour");

}

void CHexRenderer::createHexShader() {
	hexLineShader = renderer.createShader("hexLine");
	hHexMVP = hexLineShader->getUniformHandle("mvpMatrix");
	hGridSize = hexLineShader->getUniformHandle("gridSize");
	hFogTexUniform = hexLineShader->getUniformHandle("fogTex");

	hexSolidShader = renderer.createShader("hexSolid");
	hHexMVPs = hexSolidShader->getUniformHandle("mvpMatrix");
	hGridSizes = hexSolidShader->getUniformHandle("gridSize");
	hFogTexUniforms = hexSolidShader->getUniformHandle("fogTex");
}

void CHexRenderer::createVisibilityShader() {
	visibilityShader = renderer.createShader("visibilityShader");
	hHexMVPv = visibilityShader->getUniformHandle("mvpMatrix");
	hGridSizev = visibilityShader->getUniformHandle("gridSize");
	hEffectsTexUniformv = visibilityShader->getUniformHandle("effectsTex");
}

void CHexRenderer::createExplosionShader() {
	explosionShader = renderer.createShader("shaders\\explosion");
	hPos = explosionShader->getUniformHandle("pos");
	hExpMVP = explosionShader->getUniformHandle("mvpMatrix");
	hLifeTime = explosionShader->getUniformHandle("lifeTime");
	hSize = explosionShader->getUniformHandle("size");
	hTimeOut = explosionShader->getUniformHandle("timeOut");
}

void CHexRenderer::setCameraAspectRatio(glm::vec2 ratio) {
	camera.setAspectRatio(ratio.x, ratio.y);
}

/** Return the hex coordinates for the given screen positon. */
std::tuple <CHex, glm::vec3> CHexRenderer::pickHex(int screenX, int screenY) {
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

	glm::vec3 p = castFromCamToHexPlane(ray);

	CHex hexPos = worldSpaceToHex(p);

	//liveLog << "\n" << hexPos.x << " " << hexPos.y << " " << hexPos.z;


	return { hexPos, p };
}

void CHexRenderer::loadMesh(const std::string& name, const std::string& fileName) {
	importer.loadFile(fileName);
	
	//store vert buffer

	//CBuf meshBuf;
	//modelBuffers2.push_back(meshBuf);

	CBuf2 meshBuf2;
	modelBuffers.push_back(meshBuf2);

	//importer.getSingleMesh().exportToBuffer(modelBuffers2.back());
	importer.getSingleMesh().exportToBuffer(modelBuffers.back());

	//store model
	TModelNode& model = importer.getMeshNodes();
	model.name = name;
	//lineModels[name] = { model,&modelBuffers2.back(),&modelBuffers.back() };

	CLineModel lineModel;
	lineModel.model = model;
	//lineModel.buffer = &modelBuffers2.back();
	lineModel.buffer2 = &modelBuffers.back();
	lineModel.setColourR(floorplanLineColour); //////temp!!!!!!!!!!!

	lineModels[name] = lineModel;

	
}


CLineModel CHexRenderer::getLineModel(const std::string& name) {
	return lineModels[name];
}

/** Fill the structure used for drawing the path between player and cursor. */
void CHexRenderer::setCursorPath(CHex& playerPos, CHex& cursorPos) {
	cursorPath = *hexLine(playerPos, cursorPos);;
}

/** Set path directly. */
void CHexRenderer::setCursorPath(THexList& path) {
	cursorPath = path;
}


void CHexRenderer::toggleFollowCam() {
	followCam = !followCam;

	if (followCam) {
		//find camera vector to xy plane
		glm::vec3 camVector = camera.getTargetDir();


		//glm::vec3 planeN(0, 0, 1); //normal of plane on which hexes lie.
		//float d = 0; //distance of plane from origin
		//float t = -(glm::dot(camera.getPos(), planeN) + d)
		//	/ glm::dot(camVector, planeN);
		////t = distance from camera to plane for target vector

		//glm::vec3 p = camera.getPos() + camVector * t; //extend vector to find where it hits plane.

		followCamVec = camera.getPos() - castFromCamToHexPlane(camVector);
	}

}

/** Position the camera over the given target. */
void CHexRenderer::followTarget(glm::vec3& target) {
	glm::vec3 newPos = camera.getPos();
	newPos.x = target.x;
	newPos.y = target.y;
	camera.setPos(newPos);
}

/** Scroll the screen if the mouse is at the edge. */
void CHexRenderer::attemptScreenScroll(glm::i32vec2& mousePos, float dT) {
	//if (followCam)
	//	return;
	float xMove = dT * screenScrollSpeed;
	float yMove = xMove;
	glm::vec2 screenSize = camera.getView();

	glm::vec3 cameraMove(0);
	if (mousePos.x >= screenSize.x - 5 )
		cameraMove ={ xMove,0,0 };
	else if (mousePos.x < 5 )
		cameraMove = { -xMove,0,0 };

	if (mousePos.y >= screenSize.y - 5 )
		cameraMove = { 0,-yMove,0 };
	else if (mousePos.y < 5)
		cameraMove = { 0,yMove,0 };
	
	if (cameraMove.length() == 0)
		return;
	

	glm::vec3 camTarget = castFromCamToHexPlane(camera.getTargetDir());
	camTarget += cameraMove;

	glm::vec3 mapSize = hexArray->worldPosCornerDist;
	

	if (camTarget.x < -mapSize.x || camTarget.x > mapSize.x
		|| camTarget.y < -mapSize.y || camTarget.y > mapSize.y)
		return;

	moveCamera(cameraMove);
}

glm::vec3 CHexRenderer::castFromCamToHexPlane(glm::vec3& ray) {
	glm::vec3 planeN(0, 0, 1); //normal of plane on which hexes lie.
	float d = 0; //distance of plane from origin
	float t = -(glm::dot(camera.getPos(), planeN) + d)
		/ glm::dot(ray, planeN);
	//t = distance from camera to plane for this ray

	return camera.getPos() + ray * t; //extend ray to find where it hits plane.

}

/** Return the on screen position of a point in world space. */
glm::i32vec2 CHexRenderer::worldPosToScreen(glm::vec3& worldPos) {
	glm::vec4 translatedPos = camera.clipMatrix * glm::vec4(worldPos,1.0f);

	translatedPos /= translatedPos.w;

	translatedPos = (translatedPos + glm::vec4(1.0f)) * 0.5f;
	translatedPos.x *= camera.getView().x;
	translatedPos.y = (1.0f - translatedPos.y) * camera.getView().y;


	return translatedPos;
}

/** Highlight this hex. */
void CHexRenderer::highlightHex(CHex& hex) {
	glDisable(GL_DEPTH_TEST);
	//draw coloured filled hex
	TModelNode& node = solidHex->model;
	glm::mat4 worldM = glm::translate(glm::mat4(1), hexArray->getWorldPos(hex));
	node.meshes[0].colour = glm::vec4(0, 0, 0.8f, 1); 
	drawNode2(node, worldM, solidHex->buffer2);


	//draw smaller background colour hex
	glm::mat4 scaleM = glm::scale(glm::mat4(1), glm::vec3(0.9f));
	worldM = worldM * scaleM;
	node.meshes[0].colour =  floorplanSpaceColour;
	drawNode2(node, worldM, solidHex->buffer2);

	glEnable(GL_DEPTH_TEST);
}
