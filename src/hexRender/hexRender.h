#pragma once

#include <vector>
#include <string>

#include "hex/hexArray.h"
#include "renderer/mesh.h"
#include "model.h"
#include "renderer/camera.h"
#include "renderer/shader.h"

class CHexRender {
public:
	void loadMap(CHexArray* hexArray);
	void addHexTile(const std::string& name, TDrawable& tile);
	void drawMap();

	CCamera* pCamera;///<Old renderer's camera - replace with own!

	CShader* pLineShader; //also temp
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;

	


private:
	void addToMapBuf(std::vector<vc>& v, std::vector<unsigned int>& i, CHex& hex, int tile);

	std::vector<TDrawable> hexTiles;
	CHexArray* pHexArray = nullptr;


	CBuf2 mapBuf;

};