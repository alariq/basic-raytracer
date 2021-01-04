#include "config.h"
#include "camera.h"
#include "vec.h"
#include "sphere.h"
#include "light.h"
#include "material.h"
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
    std::vector<light> lights;
    color ambient_colour;
    color background_colour;

    public:

    scene():background_colour(-1,-1,-1) {}

    bool intersect(const ray &r, Real t_min, Real t_max, hit_info& hit) const {
        bool b_hit = false;
        for(const auto& s: spheres) {
            if(s.hit(r, t_min, t_max, hit)) {
                t_max = hit.t;
                hit.mat = s.get_material();
                b_hit = true;
            }
        }

        return b_hit;
    }

    void add_sphere(const point3& pos, Real radius, const material& mat) {
        spheres.emplace_back(pos, radius, mat);
    }

    void add_light(const light& l) {
        lights.push_back(l);
    }

    void set_background(color bg) { background_colour = bg; }
    color get_background() const { return background_colour; }
    bool has_background() const { return background_colour.x>=0; }

    void set_ambient(color amb) { ambient_colour = amb; }
    color get_ambient() const { return ambient_colour; }
};

#if 0
color ray_color(const ray& r, const scene& world, int depth_level) {

    hit_info rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth_level <= 0)
        return color(0,0,0);

    Real t_min = 1e-3f;
    Real t_max = 1e+5f;
    if (world.intersect(r, t_min, t_max, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat.scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth_level-1);
        return color(0,0,0);
    }

    vec3 unit_direction = r.direction();
    auto t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}
#endif

color ray_color(const ray& r, const scene& world, int depth_level) {

    hit_info rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth_level <= 0)
        return color(0,0,0);

    Real t_min = 1e-3f;
    Real t_max = 1e+5f;
    if (world.intersect(r, t_min, t_max, rec)) {
        return rec.mat.albedo*rec.mat.ka*world.get_ambient();
    }

    if (world.has_background()) {
        return world.get_background();
    } else {
        vec3 unit_direction = r.direction();
        auto t = 0.5 * (unit_direction.y + 1.0);
        return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
    }
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

    const int image_width = 512;
    const int image_height = 512;
    Real aspect_ratio = Real(image_width) / Real(image_height);
    const Real hor_fov = 45.0f; // half, so multiplied by 2
    //----------------------------------------------------------

    vec3 lookfrom = vec3(0,0,1);
    vec3 lookat = vec3(0,0,-2.5);
    vec3 vup = vec3(0,1,0);
    Real vfov = (2.0*hor_fov) / aspect_ratio;
    Real aperture = 0.0f;
    Real focus_dist = 1.0f;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, focus_dist);

    scene my_scene;
    const int scene_idx = 1;
    if (scene_idx == 0) {
        my_scene.add_sphere(point3(0, 0, -1), r05,
                            material(color(0.7, 0.3, 0.3)));
        my_scene.add_sphere(point3(0, -100.5, -1), Real(100),
                            material(color(0.8, 0.8, 0)));
    } else if (scene_idx == 1) {
        my_scene.add_sphere(point3(-2.1, 2.0, -3.0), r1,
                            material(color(0.17, 0.18, 0.5), 0.3, 0.9, 1.0, 200));
        my_scene.add_sphere(point3(0, 0, -3.0), r1,
                            material(color(0.5, 0.17, 0.18), 0.3, 0.9, 1.0, 200));
        my_scene.add_sphere(point3(2.1, -2.0, -3.0), r1,
                            material(color(0.18, 0.5, 0.17), 0.3, 0.9, 1.0, 200));
        my_scene.set_background(color(0,0,0));
        my_scene.set_ambient(color(1,1,1));
    }

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
            
            ray r = cam.get_ray(u, v);
            color pixel;
            pixel = ray_color(r, my_scene, 8);
            write_color(f, pixel);
        }
    }
    fclose(f);

    return 0;
}
