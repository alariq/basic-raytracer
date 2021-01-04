#pragma once

#include "config.h"
#include "material.h"
#include "vec.h"

struct hit_info {
    point3 p;
    vec3 normal;
    Real t;
    material mat;
};
