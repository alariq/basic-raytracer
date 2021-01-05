#pragma once

#include "vec.h"

class light {
  public:
    enum Type { Directional, Point };

    light(point3 p, Real r, color c) : pos(p), radius(r), intensity(c), type(Point) {}
    light(vec3 direction, color c) : dir(normalize(direction)), intensity(c), type(Directional) {}

    point3 get_pos() const { return pos; };
    Type get_type() const { return type; };
    color get_color() const { return intensity; };
    Real get_radius() const { return radius; };
    vec3 get_direction() const { return dir; };

  private:
    point3 pos;
    vec3 dir;
    Real radius;
    color intensity;
    Type type;

};
