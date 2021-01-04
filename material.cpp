#include "material.h"
#include "vec.h"
#include "hit.h"

#define LAMBERTIAN 1

#if METAL
bool material::scatter(const ray &r_in, const hit_info &rec, color &attenuation,
                       ray &scattered) const {
    vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
    scattered = ray(rec.p, reflected);
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
}
#elif LAMBERTIAN
bool material::scatter(const ray &r_in, const hit_info &rec, color &attenuation,
                       ray &scattered) const {
    auto scatter_direction = rec.normal + random_unit_vector();

    // Catch degenerate scatter direction
    if (near_zero(scatter_direction))
        scatter_direction = rec.normal;

    scattered = ray(rec.p, scatter_direction);
    attenuation = albedo;
    return true;
}
#endif

class lambertian : public material {
  public:
    lambertian(const color &a) : material(a) {}

    virtual bool scatter(const ray &r_in, const hit_info &rec,
                         color &attenuation, ray &scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (near_zero(scatter_direction))
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

  public:
};
