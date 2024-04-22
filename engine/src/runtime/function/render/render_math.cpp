#include "runtime/function/render/render_math.h"
#include "render_math.h"
#include <cmath>

glm::vec2 Merak::getUpperLeft(float x, float y,
    float offsetx, float offsety,
    float scalex, float scaley) {
        return {
            x - offsetx * scalex,
            y - offsety * scaley
        };
    }