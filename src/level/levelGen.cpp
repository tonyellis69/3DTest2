#include "levelGen.h"



#include "utils/random.h"

CHexArray CRandLevel::makeLevel() {
    CHexArray tmpArray;
    tmpArray.onSpawn(size.x,size.y);
    if (!rootQuad)
        rootQuad = std::make_shared<CHexQuad>(glm::i32vec2{ 0,0 }, size);

    uniqueLines.clear();
    makeQuadLines(rootQuad.get());

    removeRandomLines();

    storeUniqueQuadLines(rootQuad.get()); //for outline
    makeHexLines();



    tmpArray = writeHexLinesToLevel(tmpArray);

    return tmpArray;
}

/** Grow or shrink the map, keeping its general proportions. */
void CRandLevel::resize(int dSize) {
    size.x += dSize;
    size.y += dSize;

    if (rootQuad)
        rootQuad = std::make_shared<CHexQuad>(glm::i32vec2{ 0,0 }, size);
    removalChance = 1;
}

void CRandLevel::removeRandomLines() {
    for (auto& line = uniqueLines.begin(); line != uniqueLines.end();) {
        if (rnd::dice(maxRemovals) < removalChance) {
            line = uniqueLines.erase(line);
        }
        else
            line++;
    }
}



/** Make lines out of the deepest subquads and store the unique ones. */
void CRandLevel::makeQuadLines(CHexQuad* quad) {
    if (quad->childA && quad->childB) {
        divCount++;
        makeQuadLines(quad->childA.get());
        makeQuadLines(quad->childB.get());
    }
    else 
    {
        storeUniqueQuadLines(quad);
    }

}

/** Subdivide all leaf quads. */
void CRandLevel::subdivide() {
    split(rootQuad.get(), true);

}

void CRandLevel::quadRemovals() {
    removalChance++;
    if (removalChance > maxRemovals) 
        removalChance = 1;
    //else remove *more* lines 

    //now clear and refill the array somehow

}

void CRandLevel::reset() {
    size = { 50,50 };
    removalChance = 1;
    rootQuad = nullptr;
    divCount = 1;
}

glm::vec3 CRandLevel::findPlayerPos() {
    CHex defaultPos = offsetToCube(size.x * -0.25f, size.y * 0.25f);
    return cubeToWorldSpace(defaultPos);
}

void CRandLevel::makeDoors() {

}



void CRandLevel::storeUniqueQuadLines( CHexQuad* quad) {
    uniqueLines.emplace(quad->pos, glm::i32vec2{ quad->pos.x + quad->size.x,quad->pos.y });
    uniqueLines.emplace(glm::i32vec2{ quad->pos.x + quad->size.x,quad->pos.y }, quad->pos + quad->size);
    uniqueLines.emplace( glm::i32vec2{ quad->pos.x,quad->pos.y + quad->size.y }, quad->pos + quad->size);
    uniqueLines.emplace(quad->pos, glm::i32vec2{ quad->pos.x,quad->pos.y + quad->size.y });
}

/** Create the strings of hexes that will make our lines. */
std::vector<glm::i32vec2> CRandLevel::makeHexLine(const glm::i32vec2& a, const glm::i32vec2& b) {
    std::vector<glm::i32vec2> lineHexes;
    glm::i32vec2 dir = b - a;
    int dist = std::max(abs(dir.x), abs(dir.y));
    if (dist == 0)
        return lineHexes;
    dir = dir / dist;
    glm::i32vec2 hex = a;

    for (int c = 0; c < dist; c++) {
        lineHexes.push_back(hex);;
        hex += dir;
    }

    //remove 2 hexes to make a door.
    if (lineHexes.size() > 5) {
        if (a.x == 0)
            if (a.y == 0 || a.y == size.y)
                return lineHexes;
        if (a.x == size.x)
            return lineHexes; //unless they are the outline hexes


        int midPt = lineHexes.size() / 2;
        lineHexes.erase(lineHexes.begin() + midPt, lineHexes.begin() + midPt + 2);
    }

    return lineHexes;
}

void CRandLevel::makeHexLines() {
    hexLines.clear();
    for (auto const& line : uniqueLines) {
        hexLines.push_back(makeHexLine(line.A, line.B));
    }

}

void CRandLevel::split(CHexQuad* quad, bool splitHoriz) {
    if (!quad->childA && !quad->childB) {    
        if (quad->size.x > 2 * quad->size.y)   //force-split long quads
            splitHoriz = true;
        else if (quad->size.y > 2 * quad->size.x)
            splitHoriz = false;

        if (splitHoriz) {
            if (quad->size.x < minSplitable)
                return;

            int newX = findDivisor(quad->size.x);
            glm::i32vec2 newSize(newX, quad->size.y);
            quad->childA = std::make_shared<CHexQuad>(quad->pos, newSize);
            glm::i32vec2 newPos(quad->pos.x + newX, quad->pos.y);
            newSize = { quad->size.x - newX,quad->size.y };
            quad->childB = std::make_shared<CHexQuad>(newPos, newSize);
        }
        else {
            if (quad->size.y < minSplitable)
                return;

            int newY = findDivisor(quad->size.y);
            glm::i32vec2 newSize(quad->size.x, newY);
            quad->childA = std::make_shared<CHexQuad>(quad->pos, newSize);
            glm::i32vec2 newPos(quad->pos.x, quad->pos.y + newY);
            newSize = { quad->size.x,quad->size.y - newY };
            quad->childB = std::make_shared<CHexQuad>(newPos, newSize);

        }

    }
    else {
        bool horiz = rnd::dice(2) - 1;
        split(quad->childA.get(),horiz);
        split(quad->childB.get(),!horiz);

    }
}


/** Return a random value in this range, weighted to be central. */
int CRandLevel::findDivisor(int freeSpace) {
  
    int halfSpace = freeSpace / 2;
    std::normal_distribution<float> f(halfSpace, halfSpace / 2);
    float d = f(randEngine);

    const int minGap = 3;
    return std::clamp(int(d), minGap, freeSpace - minGap);
}


CHexArray CRandLevel::writeHexLinesToLevel(CHexArray& pArray) {
    for (auto& hexLine : hexLines) {
        for (auto& hex : hexLine) {
            pArray.getHexOffset(hex.x, hex.y).content = solidHex;
        }
    }
    return pArray;
}


