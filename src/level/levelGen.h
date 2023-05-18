#pragma once

#include <memory>
#include <set>
#include <tuple>

#include <random> //prob temp

#include "level.h"
#include "hexQuad.h"

struct THexQuadLine {
	glm::i32vec2 A;
	glm::i32vec2 B;
	THexQuadLine(glm::i32vec2 A, glm::i32vec2 B) {
		this->A = A; this->B = B;
	}
	bool operator<(const THexQuadLine& rhs) const {
		if (std::tie(A.x, A.y) < std::tie(rhs.A.x, rhs.A.y))
			return true;
		if (std::tie(A.x, A.y) == std::tie(rhs.A.x, rhs.A.y)) {
			if (std::tie(B.x, B.y) < std::tie(rhs.B.x, rhs.B.y))
				return true;

		}
		return false;
	}
};

/** Creates a CLevel object with a populated, randomly generated map. */
class CRandLevel {
public:
	std::unique_ptr<CLevel>  makeLevel();
	void setSize(int x, int y) {
		size = {x,y};
	}
	void resize(int dSize);
	void updateHexGrid();
	void makeQuadLines(CHexQuad* quad);
	void subdivide();
	void quadRemovals();
	void reset();
	glm::vec3 findPlayerPos();
	void makeDoors();

	std::vector< std::vector<glm::i32vec2> > hexLines;

private:
	void storeUniqueQuadLines( CHexQuad* quad);
	std::vector<glm::i32vec2> makeHexLine(const glm::i32vec2& a,const  glm::i32vec2& b);
	void makeHexLines();
	void split(CHexQuad* quad, bool splitHoriz);
	int findDivisor(int freeSpace);
	std::unique_ptr<CLevel> level;
	glm::i32vec2 size = { 30,30 };
	std::shared_ptr<CHexQuad> rootQuad;

	int divCount = 1;

	int maxRemovals = 5;
	int removalChance = 1;

	std::set<THexQuadLine> uniqueLines;

	std::mt19937 randEngine;

	const int minSplitable = 6;

	
};