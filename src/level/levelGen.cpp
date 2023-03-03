#include "levelGen.h"



std::unique_ptr<CLevel> CRandLevel::makeLevel() {
    level = std::make_unique<CLevel>();
    level->init(size.x, size.y);
    if (!rootQuad)
        rootQuad = std::make_shared<CHexQuad>(glm::i32vec2{ 0,0 }, size);

    draw(rootQuad.get());

    return std::move(level);
}

/** Grow or shrink the map, keeping its general proportions. */
void CRandLevel::resize(int dSize) {
    size.x += dSize;
    size.y += dSize;
}

void CRandLevel::draw(CHexQuad* quad) {
    if (quad->childA && quad->childB) {
        draw(quad->childA.get());
        draw(quad->childB.get());
    }
    else {
        fillLine(quad->pos, glm::i32vec2{quad->pos.x + quad->size.x,quad->pos.y });
        fillLine(glm::i32vec2{ quad->pos.x + quad->size.x,quad->pos.y }, quad->pos + quad->size);
        fillLine(quad->pos + quad->size, glm::i32vec2{ quad->pos.x,quad->pos.y + quad->size.y });
        fillLine(glm::i32vec2{ quad->pos.x,quad->pos.y + quad->size.y }, quad->pos);
    }
}

/** Subdivided all existing quads. */
void CRandLevel::subdivide() {
    rootQuad->split();

}



/** Fill the line of hexes between these points. */
void CRandLevel::fillLine(glm::i32vec2& a, glm::i32vec2& b) {
    glm::i32vec2 dir = b - a;
    int dist = std::max(abs(dir.x), abs(dir.y));
    if (dist == 0)
        return;
    dir = dir / dist;
    glm::i32vec2 hex = a;
    for (int c = 0; c < dist; c++) {
        level->hexArray.getHexOffset(hex.x, hex.y).content = solidHex;
        hex += dir;

    }
}



