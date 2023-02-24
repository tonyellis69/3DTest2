#include "levelGen.h"



std::unique_ptr<CLevel> CRandLevel::makeLevel()
{
    //make a level with an empty nxn map and return it. 
    //adapt existing level-make code
    //get hexWorld to load this rather than current saved level
    //then start expanding: size control. Live size control.

    auto pLvl = std::make_unique<CLevel>();

    pLvl->init(size.x, size.y);


    return pLvl;
}

/** Grow or shrink the map, keeping its general proportions. */
void CRandLevel::resize(int dSize) {
    size.x += dSize;
    size.y += dSize;
}



