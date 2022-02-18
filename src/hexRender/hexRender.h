#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "hex/hexArray.h"
#include "renderer/mesh.h"
#include "model.h"
#include "renderer/camera.h"
#include "renderer/shader.h"

#include "graphics/shader2.h"


struct THexTile;
class CHexRender {
public:
	void init();
	void loadMap(CHexArray* hexArray);
	void addHexTile(const std::string& name, TVertData& vertData, std::vector<glm::vec4>& colours);
	void drawMap();

	CCamera* pCamera;///<Old renderer's camera - replace with own!

	CShaderOld* pLineShader; //also temp
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;
	unsigned int hPalette;
	
private:
	void addToMapBuf(std::vector<vc>& v, std::vector<unsigned int>& i, CHex& hex, int tile);
	unsigned int getColourNo(glm::vec4& colour);

	std::vector<THexTile> hexTiles;
	CHexArray* pHexArray = nullptr;


	CBuf2 mapBuf;
	std::vector<glm::vec4> uniqueTileColours;

	std::unordered_map<std::string, std::shared_ptr<CShader>> shaders;


};

struct THexTile {
	std::string name;
	TVertData vertData;
	std::vector<glm::vec4> colours;
};