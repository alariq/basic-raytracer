#pragma once

#include "config.h"
#include "vec.h"

struct hit_info {
    point3 p;
    vec3 normal;
    Real t;
};
