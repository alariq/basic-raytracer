#pragma once

#include "vec.h"

class light {
  public:
    enum Type { Directional, Point };

    light(point3 pos_or_dir, color c, Type t) {
        if(Directional == t) {
            dir = pos_or_dir;
        } else {
            pos = pos_or_dir;
        }
        type = t;
        intensity = c;
    }

    point3 get_position() const { return pos; };
    Type get_type() const { return type; };
    color get_color() const { return intensity; };
    //Real get_radius() const { return radius; };
    vec3 get_direction() const { return dir; };

  private:
    point3 pos;
    vec3 dir;
    //Real radius;
    color intensity;
    Type type;

};
