#include "baseRect.h"

#include <algorithm>

bool isEqual(float a, float b) { return std::abs(a - b) < 0.0001; }

bool CBaseRect::isOnBoundary(glm::vec3& p) {
    return (isEqual(p.x, a.x) && p.y >= d.y && p.y <= a.y) ||   // Left side
        (isEqual(p.x, b.x) && p.y >= c.y && p.y <= b.y) ||   // Right side
        (isEqual(p.y, a.y) && p.x >= a.x && p.x <= b.x) ||   // Top side
        (isEqual(p.y, d.y) && p.x >= d.x && p.x <= c.x);     // Bottom side
}

glm::vec3 CBaseRect::getVert(int i) {
    switch (i) {
    case 0: return a;
    case 1: return b;
    case 2: return c;
    case 3: return d;
    }
}

/** Redefine dimensions using these diagonally opposite points.*/
void CBaseRect::setVerts(glm::vec3& v, glm::vec3& op) {
    glm::vec3 BL;
    glm::vec3 TR;

    BL.x = std::min(v.x, op.x);
    BL.y = std::min(v.y, op.y);
    TR.x = std::max(v.x, op.x);
    TR.y = std::max(v.y, op.y);

    a = { BL.x,TR.y,0 };
    b = { TR.x,TR.y,0 };
    c = { TR.x, BL.y,0 };
    d = { BL.x, BL.y, 0 };

}

bool CBaseRect::operator ==(const CBaseRect& other) {
    return a == other.a && b == other.b && c == other.c && d == other.d;
}

