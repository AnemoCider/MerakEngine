#pragma once

#include "glm/glm.hpp"

namespace Merak {
    // calculate the new upper left corner given the origin center
    glm::vec2 getUpperLeft(float x, float y,
        float offsetx, float offsety,
        float scalex, float scaley);
};