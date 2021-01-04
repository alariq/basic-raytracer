#pragma once
#include "config.h"
#include "vec.h"
#include "ray.h"

#if 0
class metal : public material {
    public:
        metal(const color& a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_info& rec, color& attenuation, ray& scattered
        ) const override {
            vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected);
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }

    public:
        color albedo;
};
#endif 

struct material {
    Real ka, kd, ks, exponent;
    Real reflectance;
    Real transmittance;
    Real refraction_iof;
    color albedo;

    material() = default;

    explicit material(color c)
        : ka(1), kd(1), ks(0), exponent(16), reflectance(0), transmittance(0),
          refraction_iof(1.0), albedo(c) {}

    material(color c, Real kamb, Real kdiff, Real kspec, Real exp)
        : ka(kamb), kd(kdiff), ks(kspec), exponent(exp), reflectance(0), transmittance(0),
          refraction_iof(1.0), albedo(c) {}

    //metal
    virtual bool scatter(const ray& r_in, const struct hit_info& rec, color& attenuation, ray& scattered) const;
};

