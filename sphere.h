#pragma once

#include "config.h"
#include "hit.h"
#include "ray.h"
#include "vec.h"

class sphere {
  public:
    sphere() {}
    sphere(point3 cen, Real r) : center(cen), radius(r){};

    bool hit(const ray &r, Real t_min, Real t_max, hit_info &rec) const {
        vec3 oc = r.origin() - center;
        auto a = lengthSqr(r.direction());
        auto half_b = dot(oc, r.direction());
        auto c = lengthSqr(oc) - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false;
        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        rec.normal = (rec.p - center) / radius;

        return true;
    }

  public:
    point3 center;
    Real radius;
};
