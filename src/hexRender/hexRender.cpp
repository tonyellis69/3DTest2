#include "hexRender.h"

//#include <cmath>
#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/renderer.h"

#include "graphics/shader2.h"

#include "utils/log.h"

#include "vertBufs.h" 

/** Load shaders, etc. */
void CHexRender::init() {
	recompileShader();

	std::vector<glm::vec3> dummyVerts(numExplosionParticles, glm::vec3(0));
	std::vector<unsigned int> dummyIndex(numExplosionParticles);
	std::iota(std::begin(dummyIndex), std::end(dummyIndex), 0); // Fill with 0, 1, ..., 99.
	explosionBuf.storeVerts(dummyVerts, dummyIndex, 1);

	


	std::vector<vBuf::T2DtexVert> quadVerts{	{ {-1.0f, 1.0f}, { 0.0f,1.0f} },
												{ {-1.0f, -1.0f}, {0,0.0f} },
												{ {1.0f, 1.0f}, {1.0f, 1.0f} },
												{ {1.0f, -1.0f}, {1.0f, 0.0f } } };
	std::vector<unsigned short> idx{ 0,1,2,3 };
	screenQuad.storeVerts(quadVerts, idx, 2, 2);

	glGenFramebuffers(1, &hScreenFrameBuffer);
	glGenFramebuffers(2, hBlurFrameBuffer);

	glGenRenderbuffers(1, &hStencilBuf);

	glGenTextures(1, &hDepthTex);

	lineDrawList.reserve(maxDrawListSize);
	upperLineList.reserve(maxDrawListSize);
	solidDrawList.reserve(maxDrawListSize);
	explosionDrawList.reserve(maxDrawListSize);
	maskList.reserve(maxDrawListSize);
	lowerMaskList.reserve(maxDrawListSize);
	//NB: most of these need far less
}
void CHexRender::recompileShader() {
	lineShader = shader::create("lineModel");
	//very temp!!!
	hMVP = lineShader->getUniform("mvpMatrix");
	hColour = lineShader->getUniform("colour");
	hWinSize = lineShader->getUniform("winSize");
	hPalette = lineShader->getUniform("colourPalette");
	hChannel = lineShader->getUniform("channel");
	hThickness = lineShader->getUniform("thickness");
	hSmoothing = lineShader->getUniform("smoothing");
	hScale = lineShader->getUniform("scale");
	hSolid = lineShader->getUniform("solid");

	filledShader = shader::create("filled");
	hMVPF = filledShader->getUniform("mvpMatrix");
	hPaletteF = filledShader->getUniform("colourPalette");

	maskShader = shader::create("mask");
	hMaskMVP = maskShader->getUniform("mvpMatrix");

	splodeShader = shader::create("explosion");
	hPos = splodeShader->getUniform("pos");
	hExpMVP = splodeShader->getUniform("mvpMatrix");
	hLifeTime = splodeShader->getUniform("lifeTime");
	hSize = splodeShader->getUniform("size");
	hTimeOut = splodeShader->getUniform("timeOut");
	hSeed = splodeShader->getUniform("seed");
	hExpPalette = splodeShader->getUniform("colourPalette");

	blurShader = shader::create("blur");
	hSrcTexture = blurShader->getUniform("srcTexture");
	hHorizontal = blurShader->getUniform("horizontal");
	hKernelSize = blurShader->getUniform("blurSize");
	hSigma = blurShader->getUniform("sigma");


	screenBufShader = shader::create("screen");
	hScreenBuf = screenBufShader->getUniform("screenBuf");
	hScreenMask = screenBufShader->getUniform("screenMask");
	hBlurTex = screenBufShader->getUniform("blurTex");
	hX = screenBufShader->getUniform("x");

	sceneLayerShader = shader::create("sceneLayer");
	hBlur = sceneLayerShader->getUniform("blurTexture");
	hFade = sceneLayerShader->getUniform("fade");
}



void CHexRender::loadMap(CHexArray* hexArray) {
	pHexArray = hexArray;




	std::vector<vc> verts; std::vector<unsigned int> index;

	for (int y = 0; y < pHexArray->height; y++) {
		for (int x = 0; x <  pHexArray->width; x++) {
			int tile = pHexArray->getHexOffset(x, y).content;
			CHex hex = pHexArray->indexToCube(x, y);
			if (tile == emptyHex)
				addToMapBuf(verts, index, hex, 0);
			else
				addToMapBuf(verts, index, hex, 1);

		}
	}

	mapBuf.storeVerts(verts, index, 3,1);

}

void CHexRender::addHexTile(const std::string& name, TVertData& vertData, std::vector<glm::vec4>& colours) {
	THexTile tile = { name, vertData, colours};
	hexTiles.push_back(tile);
}



void CHexRender::drawMap() {


	lineShader->activate();

	glm::mat4 mvp = camera.clipMatrix;

	lineShader->setUniform(hMVP, mvp);
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hColour, glm::vec4(1, 0, 0, 0));
	glUniform4fv(hPalette, 4, (float*)(uniqueTileColours.data()));
	lineShader->setUniform(hChannel, 0.0f);
	lineShader->setUniform(hScale, 1.0f);
	lineShader->setUniform(hThickness, sceneryLine);

	lineShader->setUniform(hSmoothing, 0.0f);
	lineShader->setUniform(hSolid, 0.1f);

	//renderer.drawLineStripAdjBuf(mapBuf, 0, mapBuf.numElements);
	mapBuf.setVAO();
	glDrawElements(GL_LINE_STRIP_ADJACENCY, mapBuf.numElements, GL_UNSIGNED_SHORT,0);
	mapBuf.clearVAO();
	

}

void CHexRender::drawMeshLine(TMeshRec& mesh) {
	glDrawElements(GL_LINE_STRIP_ADJACENCY, mesh.indexSize, GL_UNSIGNED_SHORT, (void*)(mesh.indexStart * sizeof(unsigned short)) );
}

void CHexRender::drawMeshSolid(TMeshRec& mesh) {
	glDrawElements(GL_TRIANGLES, mesh.indexSize, GL_UNSIGNED_SHORT, (void*)(mesh.indexStart * sizeof(unsigned short)));
}

void CHexRender::drawExplosion(int particleCount) {
	renderer.drawPointsBuf(explosionBuf, 0, particleCount);
}

void CHexRender::storePalette(const std::string& name, std::vector<glm::vec4> palette) {
	palettes[name] = palette;
}

std::vector<glm::vec4>* CHexRender::getPalette(const std::string& name) {
	return &palettes[name];
}

void CHexRender::resetDrawLists() {
	lineDrawList.clear();
	upperLineList.clear();
	solidDrawList.clear();
	explosionDrawList.clear();
	maskList.clear();
	lowerMaskList.clear();
	lineDrawListDBG.clear();

	lineDrawSize = 0;
	upperLineSize = 0;
	solidDrawSize = 0;
	explosionDrawSize = 0;
	maskSize = 0;
	lowerMaskSize = 0;
}

void CHexRender::drawLineList() {
	lineShader->activate();
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hChannel, 1.0f);
	lineShader->setUniform(hThickness, tmpLineThickness);
	lineShader->setUniform(hSmoothing, tmpLineSmooth);
	lineShader->setUniform(hSolid, tmpLineSolid);
	lineShader->setUniform(hScale, 1.0f);

	//glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //the test to pass
	//glStencilMask(0x00);


	for (auto& draw : lineDrawList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix *draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
	}

//	sysLog << "\n" << camera.clipMatrix * glm::vec4(-77.2343, 10.2631, 0,1);

	//lineShader->setUniform(hChannel, 2.0f);
	//lineShader->setUniform(hThickness, 20);
	//for (auto& draw : lineDrawList) {
	//	draw.buf->setVAO();
	//	glm::mat4 mvp = camera.clipMatrix * *draw.matrix;
	//	lineShader->setUniform(hMVP, mvp);
	//	lineShader->setUniform(hPalette, *draw.palette);
	//	drawMeshLine(*draw.meshRec);
	//}



	//glDisable(GL_STENCIL_TEST);
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}

void CHexRender::drawLineListDBG() {
	//return;
	lineShader->activate();
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hChannel, 1.0f);
	lineShader->setUniform(hThickness, tmpLineThickness);
	lineShader->setUniform(hSmoothing, tmpLineSmooth);
	lineShader->setUniform(hSolid, tmpLineSolid);
	lineShader->setUniform(hScale, 1.0f);

	struct vc2 {
		glm::vec3 v;
		int c = 0;
	};

	std::vector<vc2> verts;

	for (auto& draw : lineDrawListDBG) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
		draw.buf->readVerts(verts);
		sysLog << "\nverts: ";
		for (auto& i : verts)
			sysLog << i.v << " ";
	}



}

void CHexRender::drawUpperLineList() {
	lineShader->activate();
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hChannel, 1.0f);
	lineShader->setUniform(hThickness, tmpLineThickness);
	lineShader->setUniform(hSmoothing, tmpLineSmooth);

	for (auto& draw : upperLineList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
	}
	glBindVertexArray(0);
}

void CHexRender::makeGlowShapes() {
	glBindFramebuffer(GL_FRAMEBUFFER, hScreenFrameBuffer); 
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenMask.handle, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fatalLog << alertMsg << "\nError creating framebuffer.";
		return;
	}
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	lineShader->activate();
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hChannel, 1.0f);
	lineShader->setUniform(hThickness, 20.0f);
	lineShader->setUniform(hSmoothing, 0);
	lineShader->setUniform(hSolid, 0.1f);
	lineShader->setUniform(hScale, 1.5f);

	for (auto& draw : lineDrawList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
	}

	for (auto& draw : upperLineList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
	}

	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//screenMask.savePNG("d://shapes.png");
}


void CHexRender::drawSolidList() {
	filledShader->activate();

	for (auto& draw : solidDrawList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		filledShader->setUniform(hMVPF, mvp);
		filledShader->setUniform(hPaletteF, *draw.palette);
		drawMeshSolid(*draw.meshRec);
	}

}

void CHexRender::drawMaskList() {
	maskShader->activate();

	//glEnable(GL_STENCIL_TEST);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //if either test fails, keep buffer
	//glStencilFunc(GL_ALWAYS, 1, 0xFF); //so test always passes 
	//glStencilMask(0xFF);
	glDepthRange(0, 0);
	glClear( GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	for (auto& draw : maskList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		maskShader->setUniform(hMaskMVP, mvp);
		drawMeshSolid(*draw.meshRec);
	}

	//modelTexture.savePNG("d://model.png");
	//screenMask.savePNG("d://mask.png");

	glDepthRange(0, 1); //restore default

	for (auto& draw : lowerMaskList) {
		draw.buf->setVAO();
		glm::mat4 mvp = camera.clipMatrix * draw.matrix;
		maskShader->setUniform(hMaskMVP, mvp);
		drawMeshSolid(*draw.meshRec);
	}

}

void CHexRender::drawExplosionList() {
	splodeShader->activate();
	splodeShader->setUniform(hExpMVP, camera.clipMatrix);

	for (auto& draw : explosionDrawList) {
		splodeShader->setUniform(hPos, draw.pos);
		splodeShader->setUniform(hLifeTime, *draw.lifeTime);
		splodeShader->setUniform(hSize, *draw.size);
		splodeShader->setUniform(hTimeOut, *draw.timeOut);
		splodeShader->setUniform(hSeed, *draw.seed);
		splodeShader->setUniform(hExpPalette, *draw.palette);

		drawExplosion(*draw.particleCount);
	}

}

void CHexRender::startScreenBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, hScreenFrameBuffer); //NB: bulk of overhead is here
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenBuffer.handle, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, screenMask.handle, 0);
	GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fatalLog << alertMsg << "\nError creating framebuffer.";
		return;
	}
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT );
}

void CHexRender::startSceneBuffer() {
	//glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, hScreenFrameBuffer); //NB: bulk of overhead is here
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, levelTexture.handle, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, modelTexture.handle, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, screenMask.handle, 0);

	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hStencilBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, hDepthTex, 0);
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fatalLog << alertMsg << "\nError creating framebuffer.";
		return;
	}

	glClearColor(0, 0, 0, 0);
	//glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CHexRender::blur() {
	glClearColor(0.0, 0.0, 0.0, 0); 
	for (int b = 0; b < 2; b++) {
		glBindFramebuffer(GL_FRAMEBUFFER, hBlurFrameBuffer[b]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, blurTexture[b].handle,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clears away last frame's residue
	}
	

	glDisable(GL_BLEND);

	blurShader->activate();
	screenQuad.setVAO();
	bool horizontal = true, first_iteration = true;

	int blurs = tmpBlurs;
	glViewport(0, 0, blurTexture[0].width, blurTexture[0].height);
	for (int b = 0; b < blurs; b++) {
		glBindFramebuffer(GL_FRAMEBUFFER, hBlurFrameBuffer[horizontal]);
		unsigned int hTexture = first_iteration ? screenMask.handle : blurTexture[!horizontal].handle;
		blurShader->setTexture0(hSrcTexture, hTexture);
		blurShader->setUniform(hHorizontal, horizontal);
		blurShader->setUniform(hKernelSize, tmpKernel);
		blurShader->setUniform(hSigma, tmpSigma);

		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		horizontal = !horizontal;
		if (first_iteration) {
			first_iteration = false;
			
		}
	}
	screenQuad.clearVAO();
	glViewport(0, 0, screenBuffer.width, screenBuffer.height);

	glEnable(GL_BLEND);
	//blurTexture[1].savePNG("d://blur.png");
	//screenMask.savePNG("d://mask.png");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CHexRender::drawScreenBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//blurTexture[1].savePNG("d://blur1.png");
	screenBufShader->activate();
	screenBufShader->setTexture0(hScreenBuf, screenBuffer.handle);
	screenBufShader->setTexture1(hScreenMask, screenMask.handle); 
	screenBufShader->setTexture2(hBlurTex, blurTexture[1].handle); 
	screenBufShader->setUniform(hX, tmpX);
	screenQuad.setVAO();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	screenQuad.clearVAO();
}

void CHexRender::drawSceneLayers() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//levelTexture.savePNG("d://level.png");
	//blurTexture[1].savePNG("d://blur.png");
	sceneLayerShader->activate();
	//sceneLayerShader->setTexture0(hMap, levelTexture.handle);
	//sceneLayerShader->setTexture1(hModels, modelTexture.handle);
	sceneLayerShader->setTexture2(hBlur, blurTexture[1].handle);

	screenQuad.setVAO();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	screenQuad.clearVAO();

	//modelTexture.savePNG("d://model.png");
	//levelTexture.savePNG("d://level.png");
	//screenMask.savePNG("d://mask.png");
	
}

void CHexRender::drawGlow() {
	glDisable(GL_DEPTH_TEST);
	sceneLayerShader->activate();
	sceneLayerShader->setTexture0(hBlur, blurTexture[1].handle);
	sceneLayerShader->setUniform(hFade, tmpFade);
	screenQuad.setVAO();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	screenQuad.clearVAO();
	glEnable(GL_DEPTH_TEST);

}


void CHexRender::setScreenSize(glm::vec2& ratio) {
	screenSize = glm::i32vec2(ratio);
	screenBuffer.resize(int(ratio.x), int(ratio.y) );
	screenMask.resize(int(ratio.x), int(ratio.y));
	resizeBlurTextures();

	levelTexture.resize(int(ratio.x), int(ratio.y));
	modelTexture.resize(int(ratio.x), int(ratio.y));

	glBindRenderbuffer(GL_RENDERBUFFER, hStencilBuf);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,int(ratio.x),int(ratio.y));
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, int(ratio.x), int(ratio.y));


	glBindTexture(GL_TEXTURE_2D, hDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, int(ratio.x), int(ratio.y), 
		0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CHexRender::resizeBlurTextures() {
	glm::i32vec2 newSize = screenSize / tmpBlurTextDivisor;
	blurTexture[0].resize(newSize.x, newSize.y);
	blurTexture[1].resize(newSize.x, newSize.y);

}

void CHexRender::setCameraPos(float x, float y) {
	float camZ = camera.getPos().z; //FIX: just store cam height
	camera.setPos(glm::vec3(x, y, camZ));
}

void CHexRender::drawModelAt(CModel& model, glm::vec3& pos) {
	lineShader->activate();
	lineShader->setUniform(hWinSize, camera.getView());
	lineShader->setUniform(hChannel, 1.0f);
	lineShader->setUniform(hThickness, 0.5f);////FIXME! User should control these, or (better) model should
	lineShader->setUniform(hSmoothing, tmpLineSmooth);
	lineShader->setUniform(hSolid, tmpLineSolid);
	lineShader->setUniform(hScale, 1.0f);



	model.buf.setVAO();

	glm::mat4 mvp = camera.clipMatrix * glm::translate(glm::mat4(1), pos);
	lineShader->setUniform(hMVP, mvp);
	lineShader->setUniform(hPalette, model.palette);
	drawMeshLine(model.getMainMesh()->meshRec);
	
	model.buf.clearVAO();
}

void CHexRender::loadLineList(TLineDraw& listEntry) {
	lineDrawList.emplace_back(listEntry);
	lineDrawSize++;
}

void CHexRender::loadLineListDbg(TLineDraw& listEntry)
{
	lineDrawListDBG.emplace_back(listEntry);
}

void CHexRender::loadUpperLineList(TLineDraw& listEntry) {
	upperLineList.emplace_back(listEntry);
	upperLineSize++;
}

void CHexRender::loadSolidList(TSolidDraw& listEntry) {
	solidDrawList.emplace_back(listEntry);
	solidDrawSize++;
}

void CHexRender::loadUpperMaskList(TSolidDraw& listEntry) {
	maskList.emplace_back(listEntry);
	maskSize++;
}

void CHexRender::loadLowerMaskList(TSolidDraw& listEntry) {
	lowerMaskList.emplace_back(listEntry);
	lowerMaskSize++;
}

void CHexRender::loadExplosionDrawList(TSplodeDraw& listEntry) {
	explosionDrawList.emplace_back(listEntry);
	explosionDrawSize++;
}

bool CHexRender::dollyCamera(float delta) {
	camera.dolly(delta);
	glm::vec3 pos = camera.getPos();
	if (pos.z <= 0) {
		pos.z = 1;
		camera.setPos(pos);
		return false;
	}
	return true;
}

void CHexRender::pitchCamera(float delta) {
	cameraPitch += delta;
	camera.pitch(delta);
}

void CHexRender::moveCamera(glm::vec3& move) {
	camera.translate(move);
}

/** Point the camera in the given direction. Eg, top-down. */
void CHexRender::pointCamera(glm::vec3& dir) {
	camera.lookAt(dir);
}

void CHexRender::setCameraHeight(float z) {
	glm::vec3 newPos = camera.getPos();
	newPos.z = z;
	camera.setPos(newPos);
}

void CHexRender::setCameraPos(glm::vec3& pos) {
	camera.setPos(pos);
}

void CHexRender::setCameraPitch(float pitch) {
	cameraPitch = pitch;
	camera.pitch(pitch);
}

void CHexRender::setCameraAspectRatio(glm::vec2& ratio, float fov) {
	camera.setAspectRatio(ratio, fov);
}

/** Return the worldspace pos on the hex plane for the given screen positon. */
glm::vec3 CHexRender::screenToWS(int screenX, int screenY) {
	//convert to clip coordinates
	glm::vec2 screenSize = camera.getView();
	glm::vec4 clip((2.0f * screenX) / screenSize.x - 1.0f, 1.0f - (2.0f * screenY) / screenSize.y, -1.0f, 1.0f);

	//undo perspective 
	glm::vec4 view = glm::inverse(camera.perspectiveMatrix) * clip;
	view.z = -1.0f;
	view.w = 0.0f;

	//undo view (camera position)
	glm::vec4 worldPos = camera.worldMatrix * view;
	glm::vec3 ray = glm::normalize(glm::vec3(worldPos));

	//this should be a ray, projecting from 0,0,0 in the given direction.

	return castFromCamToHexPlane(ray);

}

glm::vec3 CHexRender::castFromCamToHexPlane(glm::vec3& ray) {
	glm::vec3 planeN(0, 0, 1); //normal of plane on which hexes lie.
	float d = 0; //distance of plane from origin
	float t = -(glm::dot(camera.getPos(), planeN) + d)
		/ glm::dot(ray, planeN);
	//t = distance from camera to plane for this ray

	return camera.getPos() + ray * t; //extend ray to find where it hits plane.

}

/** Add the verts for tile tileNo to the given arrays, offset to position hex.
	This enables the creation of one big buffer storing all scenery for the map,
	which can be drawn with one drawcall.*/
void CHexRender::addToMapBuf(std::vector<vc>& v, std::vector<unsigned int>& i,CHex& hex, int tileNo) {
	glm::vec3 offset = cubeToWorldSpace(hex);
	int currentVerts = v.size();

	THexTile tile = hexTiles[tileNo];

	auto meshIt = tile.vertData.meshes.begin() + 1;
	unsigned int vert = 0;
	unsigned int coloursIdx = 0;
	unsigned int colourNo = getColourNo(tile.colours[0]);
	v.resize(v.size() + tile.vertData.vertices.size());
	for (auto& newVert : tile.vertData.vertices) {
		v[currentVerts + vert].v = newVert + offset;
		v[currentVerts + vert].c = colourNo;
		vert++;
		if (meshIt != tile.vertData.meshes.end() && vert == meshIt->vertStart) {
			coloursIdx++;
			colourNo = getColourNo(tile.colours[coloursIdx]);
			meshIt++;
		}
	}

	int it = i.size();
	i.resize(i.size() + tile.vertData.indices.size());

	for (auto& index : tile.vertData.indices) {
		if (index == 0xFFFF )
			i[it] = 0xFFFFFFFF;
		else
			i[it] = index + currentVerts;
		it++;
	}
}

/** Return the unique id assigned to this colour. */
unsigned int CHexRender::getColourNo(glm::vec4& colour) {
	auto search = std::find(uniqueTileColours.begin(), uniqueTileColours.end(), colour);
	if (search != uniqueTileColours.end()) {
		return search - uniqueTileColours.begin();
	}
	uniqueTileColours.push_back(colour);
	return uniqueTileColours.size() -1;
}
