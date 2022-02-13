#include "hexRender.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/renderer.h"

void CHexRender::loadMap(CHexArray* hexArray) {
	pHexArray = hexArray;

	//very temp!!!
	hMVP = pLineShader->getUniformHandle("mvpMatrix");
	hColour = pLineShader->getUniformHandle("colour");
	hWinSize = pLineShader->getUniformHandle("winSize");

	std::vector<vc> verts; std::vector<unsigned int> index;

	for (int y = 0; y < pHexArray->height; y++) {
		for (int x = 0; x < pHexArray->width; x++) {
			int tile = pHexArray->getHexOffset(x, y).content;
			CHex hex = pHexArray->indexToCube(x, y);
			if (tile == emptyHex)
				addToMapBuf(verts, index, hex, 0);
			else
				addToMapBuf(verts, index, hex, 1);

		}
	}

	mapBuf.storeVerts(verts, index, 3,4);

}

void CHexRender::addHexTile(const std::string& name, TDrawable& tile) {
	hexTiles.push_back(tile);
}



void CHexRender::drawMap() {
	renderer.setShader(pLineShader);

	glm::mat4 mvp = pCamera->clipMatrix;

	pLineShader->setShaderValue(hMVP, mvp);
	pLineShader->setShaderValue(hWinSize, pCamera->getView());
	pLineShader->setShaderValue(hColour, glm::vec4(1, 0, 0, 0));

	renderer.drawLineStripAdjBuf(mapBuf, 0, mapBuf.numElements);



}



void CHexRender::addToMapBuf(std::vector<vc>& v, std::vector<unsigned int>& i,CHex& hex, int tileNo) {
	glm::vec3 offset = cubeToWorldSpace(hex);
	int indexOffset = v.size();


	std::vector<vc> v2; std::vector<unsigned short> i2;

	TDrawable tile = hexTiles[tileNo];

	tile.buf->readIndex(i2);
	tile.buf->readVerts(v2);

	i.push_back(0xFFFF);

	for (auto& vert : v2)
		vert.v += offset;
	for (auto& index : i2) {
		if (index != 0xFFFF)
			index += indexOffset;
	}

	v.insert(v.end(),v2.begin(),v2.end());
	i.insert(i.end(), i2.begin(), i2.end());
}
