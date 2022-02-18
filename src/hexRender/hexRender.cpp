#include "hexRender.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/renderer.h"

#include "graphics/shader2.h"

/** Load shaders, etc. */
void CHexRender::init() {
	/*auto test = std::make_shared<CShader>("lineModel");
	shaders["lineModel"] = test;*/
	shader::create("lineModel");
}

void CHexRender::loadMap(CHexArray* hexArray) {
	pHexArray = hexArray;

	//very temp!!!
	hMVP = pLineShader->getUniformHandle("mvpMatrix");
	hColour = pLineShader->getUniformHandle("colour");
	hWinSize = pLineShader->getUniformHandle("winSize");
	hPalette = pLineShader->getUniformHandle("colourPalette");


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
	renderer.setShader(pLineShader);

	glm::mat4 mvp = pCamera->clipMatrix;

	pLineShader->setShaderValue(hMVP, mvp);
	pLineShader->setShaderValue(hWinSize, pCamera->getView());
	pLineShader->setShaderValue(hColour, glm::vec4(1, 0, 0, 0));
	glUniform4fv(hPalette, 4, (float*)(uniqueTileColours.data()));

	renderer.drawLineStripAdjBuf(mapBuf, 0, mapBuf.numElements);

	

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
