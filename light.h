#pragma once

#include "vec.h"

class light {
  public:
    enum Type { Directional, Point };

    light(point3 p, Type t) : pos(p), type(t) {}

    point3 get_pos() const { return pos; };
    Type get_type() const { return type; };
    color get_color() const { return intensity; };

  private:
    point3 pos;
    Type type;
    color intensity;
};
