#pragma once

#include "config.h"
#include "vec.h"
#include "ray.h"

class camera {
  public:
    camera(point3 lookfrom, point3 lookat, vec3 vup,
           Real vfov, // vertical field-of-view in degrees
           Real aspect_ratio, Real aperture, Real focus_dist) {
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / Real(2.0));
        auto viewport_height = Real(2.0) * h;
        auto viewport_width = aspect_ratio * viewport_height;

        w = normalize(lookfrom - lookat);
        u = normalize(cross(vup, w));
        v = cross(w, u);

        origin = lookfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner =
            origin - horizontal / 2 - vertical / 2 - focus_dist * w;

        lens_radius = aperture / 2;
    }

    ray get_ray(Real s, Real t) const {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x + v * rd.y;

        return ray(origin + offset, lower_left_corner + s * horizontal +
                                        t * vertical - origin - offset);
    }

  private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    Real lens_radius;
};
