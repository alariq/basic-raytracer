#include "config.h"
#include "camera.h"
#include "vec.h"
#include "sphere.h"
#include "ray.h"
#include "hit.h"

#include <vector>
#include <cstdlib>
#include <cstdio>

const int g_samples_per_pixel = 1;
const Real r1 = Real(1.0);
const Real r0 = Real(0.0);
const Real r05 = Real(0.5);

class scene {
    std::vector<sphere> spheres;

    public:
    bool intersect(const ray &r, Real t_min, Real t_max, hit_info& hit) {
        bool b_hit = false;
        for(const auto& s: spheres) {
            if(s.hit(r, t_min, t_max, hit)) {
                t_max = hit.t;
                b_hit = true;
            }
        }

        return b_hit;
    }

    void add_sphere(const point3& pos, Real radius) {
        spheres.emplace_back(pos, radius);
    }
};

Real hit_sphere(const point3& center, Real radius, const ray& r) {
    vec3 oc = r.origin() - center;
    auto a = dot(r.direction(), r.direction());
    auto b = 2.0 * dot(oc, r.direction());
    auto c = dot(oc, oc) - radius*radius;
    auto discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-b - sqrt(discriminant) ) / (2.0*a);
    }
}

color intersect_scene(const ray& r) {
    auto t = hit_sphere(point3(0,0,-1), 0.5, r);
    if(t > r0) {
        vec3 n = normalize(r.at(t) - vec3(0,0,-1));
        return r05*color(n.x+r1, n.y+r1, n.z+r1);
    }

    t = r05 * (r.direction().y + r1);
    return (r1 - t)*color(r1,r1,r1) + t*color(r05, Real(0.7), r1);
}

void write_color(FILE* fh, color pixel) {

    constexpr Real scale = Real(1.0) / g_samples_per_pixel;
    pixel = scale * pixel;
    int ir = (int)(255 * clamp(pixel.x, Real(0), Real(1)));
    int ig = (int)(255 * clamp(pixel.y, Real(0), Real(1)));
    int ib = (int)(255 * clamp(pixel.z, Real(0), Real(1)));

    //fprintf(fh, "%d %d %d\n", ir, ig, ib);
    fprintf(fh, "%d %d %d\n", ir, ig, ib);
}

int main(void) {

    const int image_width = 256;
    const int image_height = 256;
    Real aspect_ratio = Real(image_width) / Real(image_height);

    Real viewport_height = Real(2.0);
    Real viewport_width = aspect_ratio * viewport_height;
    Real focal_length = 1.0f;

    vec3 lookfrom = vec3(0,0,0);
    vec3 lookat = vec3(0,0,1);
    vec3 vup = vec3(0,1,0);
    Real vfov = 45.0f;
    Real aperture = 1.0f;
    Real focus_dist = 1.0f;

    vec3 origin = lookfrom;
    vec3 horizontal = vec3(viewport_width, 0, 0);
    vec3 vertical = vec3(0, viewport_height, 0);
    vec3 lower_left_corner = origin - Real(0.5) * horizontal -
                             Real(0.5) * vertical - vec3(0, 0, focal_length);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, focus_dist);

    scene my_scene;
    my_scene.add_sphere(point3(0,0,-1), r05);

    FILE* f = fopen("result.ppm", "w");
    if(!f) {
        printf("Cannot open result.ppm file for writing\n");
        return -1;
    }
    fprintf(f, "P3\n%d %d\n255\n", image_width, image_height);

    Real oo_w = Real(1.0) / Real(image_width - 1);
    Real oo_h = Real(1.0) / Real(image_height - 1);
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            Real u = Real(i) * oo_w;
            Real v = Real(j) * oo_h;
            
            vec3 dir = normalize(lower_left_corner + u*horizontal + v*vertical - origin);
            ray r(origin, dir);
            hit_info hi;
            Real t_min = 1e-5f;
            Real t_max = 1e+5f;
            hi.t = t_max;
            color pixel;
            if(my_scene.intersect(r, t_min, t_max, hi)) {
                vec3 n = hi.normal;// normalize(r.at(hi.t) - vec3(0,0,-1));
                pixel = r05 * (n + r1);
            } else {
                Real t = r05 * (r.direction().y + r1);
                pixel = (r1 - t)*color(r1,r1,r1) + t*color(r05, Real(0.7), r1);
            }
             
            //color pixel = intersect_scene(r);
            //pixel = vec3(u, v, 0.25);
            write_color(f, pixel);
        }
    }
    fclose(f);

    return 0;
}
