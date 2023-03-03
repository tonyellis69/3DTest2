#include "level.h"

#include <unordered_set>
#include <algorithm>

#include "utils/log.h"
#include "utils/random.h"


#include "utils/files.h"

#include "..\robot.h"

#include "..\spawner.h"


THexList CLevel::findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive) {
	std::unordered_set<CHex, hex_hash> uniqueHexes;

	int attempts;
	if (obsessive)
		attempts = 3;
	else
		attempts = 1;

	for (auto perimeterHex : perimeterHexes) {
		for (int offset = 0; offset < attempts; offset++) {
			THexList line = *hexLine4(apex, perimeterHex, offset);

			for (auto hex = line.begin() + 1; hex != line.end(); hex++) {
				uniqueHexes.insert(*hex);
				if (hexArray.getHexCube(*hex).content == 2) {
					break;
				}
			}
		}
	}

	uniqueHexes.insert(apex);


	THexList visibleHexes;
	visibleHexes.assign(uniqueHexes.begin(), uniqueHexes.end());

	return visibleHexes;
}


/** Clear the fog-of-war wherever the given viewfield indicates a now-visible hex. 
	Set visibility to zero where a hex has now become non-visible. */
void CLevel::updateVisibility(THexList& visibleHexes, THexList& unvisibledHexes) {
	for (auto& visibleHex : visibleHexes) {
		//hexArray.setFog(visibleHex, 0);
		hexArray.setVisibility(visibleHex, 1.0f);
	}

	for (auto& unvisibledHex : unvisibledHexes) {
		hexArray.setVisibility(unvisibledHex, 0.5f);
	}
}

glm::i32vec2 CLevel::getGridSize() {
	return glm::i32vec2(hexArray.width,hexArray.height);
}



