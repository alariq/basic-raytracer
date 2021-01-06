#pragma once

#include "config.h"
#include "hit.h"
#include "material.h"
#include "ray.h"

class mesh {
    public:
    mesh() = default;
    mesh(const mesh&) = delete;
    mesh(mesh&&) = delete;

    mesh(const struct ObjFile* obj, const material& m);
    bool hit(const ray &r, Real t_min, Real t_max, hit_info &rec) const;
    const material& get_material() const { return mat; }

    ~mesh();
    private:

    const struct ObjFile* obj_model;
    material mat;
};

