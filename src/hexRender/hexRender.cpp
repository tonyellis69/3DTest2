#include "hexRender.h"

//#include <cmath>
#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/renderer.h"

#include "graphics/shader2.h"

/** Load shaders, etc. */
void CHexRender::init() {
	lineShader = shader::create("lineModel");
	//very temp!!!
	hMVP = lineShader->getUniform("mvpMatrix");
	hColour = lineShader->getUniform("colour");
	hWinSize = lineShader->getUniform("winSize");
	hPalette = lineShader->getUniform("colourPalette");

	filledShader = shader::create("filled");
	hMVPF = filledShader->getUniform("mvpMatrix");
	hPaletteF = filledShader->getUniform("colourPalette");

	splodeShader = shader::create("explosion");
	hPos = splodeShader->getUniform("pos");
	hExpMVP = splodeShader->getUniform("mvpMatrix");
	hLifeTime = splodeShader->getUniform("lifeTime");
	hSize = splodeShader->getUniform("size");
	hTimeOut = splodeShader->getUniform("timeOut");
	hSeed = splodeShader->getUniform("seed");
	hExpPalette = splodeShader->getUniform("colourPalette");

	std::vector<glm::vec3> dummyVerts(numExplosionParticles, glm::vec3(0));
	std::vector<unsigned int> dummyIndex(numExplosionParticles);
	std::iota(std::begin(dummyIndex), std::end(dummyIndex), 0); // Fill with 0, 1, ..., 99.
	explosionBuf.storeVerts(dummyVerts, dummyIndex, 1);
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
//	renderer.setShader(pLineShader);
	lineShader->activate();

	glm::mat4 mvp = pCamera->clipMatrix;

	lineShader->setUniform(hMVP, mvp);
	lineShader->setUniform(hWinSize, pCamera->getView());
	lineShader->setUniform(hColour, glm::vec4(1, 0, 0, 0));
	glUniform4fv(hPalette, 4, (float*)(uniqueTileColours.data()));

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
	solidDrawList.clear();
	explosionDrawList.clear();
}

void CHexRender::drawLineList() {
	lineShader->activate();
	lineShader->setUniform(hWinSize, pCamera->getView());
	//lineShader->setUniform(hColour, glm::vec4(0, 0, 0, 0));

	for (auto& draw : lineDrawList) {
		draw.buf->setVAO();
		glm::mat4 mvp = pCamera->clipMatrix * *draw.matrix;
		lineShader->setUniform(hMVP, mvp);
		lineShader->setUniform(hPalette, *draw.palette);
		drawMeshLine(*draw.meshRec);
	}
}

void CHexRender::drawSolidList() {
	filledShader->activate();

	for (auto& draw : solidDrawList) {
		draw.buf->setVAO();
		glm::mat4 mvp = pCamera->clipMatrix * *draw.matrix;
		filledShader->setUniform(hMVPF, mvp);
		filledShader->setUniform(hPaletteF, *draw.palette);
		drawMeshSolid(*draw.meshRec);
	}

}

void CHexRender::drawExplosionList() {
	splodeShader->activate();
	splodeShader->setUniform(hExpMVP, pCamera->clipMatrix);

	for (auto& draw : explosionDrawList) {
		splodeShader->setUniform(hPos, *draw.pos);
		splodeShader->setUniform(hLifeTime, *draw.lifeTime);
		splodeShader->setUniform(hSize, *draw.size);
		splodeShader->setUniform(hTimeOut, *draw.timeOut);
		splodeShader->setUniform(hSeed, *draw.seed);
		splodeShader->setUniform(hExpPalette, *draw.palette);

		drawExplosion(*draw.particleCount);
	}

}



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
