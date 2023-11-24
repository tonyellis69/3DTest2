#include "baseRect.h"

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
