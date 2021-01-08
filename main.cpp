#include "config.h"
#include "camera.h"
#include "scene.h"
#include "vec.h"
#include "sphere.h"
#include "light.h"
#include "material.h"
#include "ray.h"
#include "hit.h"

#include "tinyxml2/tinyxml2.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>

const int g_samples_per_pixel = 1;
const Real r1 = Real(1.0);
const Real r0 = Real(0.0);
const Real r05 = Real(0.5);


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

bool ray_shadow(const ray& r, const scene& world, Real t_max) {
    hit_info rec;
    Real t_min = 1e-3f;
    return world.intersect(r, t_min, t_max, rec);
}

Real fresnel(Real n1, Real n2, vec3 normal, vec3 incident, Real refl_k)
{
        // Schlick aproximation
        Real r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        Real cosX = -dot(normal, incident);
        if (n1 > n2)
        {
            Real n = n1/n2;
            Real sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return 1.0;
            cosX = sqrt(1.0-sinT2);
        }
        Real x = 1.0-cosX;
        Real ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        ret = (refl_k + (1.0 - refl_k) * ret);
        return ret;
}

color ray_color(const ray& r, const scene& world, int depth_level) {

    hit_info rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth_level <= 0)
        return color(0,0,0);

    Real t_min = 1e-3f;
    Real t_max = 1e+5f;
    if (world.intersect(r, t_min, t_max, rec)) {
        color ambient = rec.mat.ka*world.get_ambient();
        color diffuse = vec3(0,0,0);
        color specular = vec3(0,0,0);
        for(const auto& l: world.get_lights()) {
            switch(l.get_type()) {
                case light::Directional:
                    {
                        const vec3& light_dir = normalize(l.get_direction());

                        ray sh_r(rec.p, -light_dir);
                        bool b_in_shadow = ray_shadow(sh_r, world, Real(1e+5));
                        if(b_in_shadow)
                            break;

                        const color& light_color = l.get_color();
                        Real ndotl = max(dot(rec.normal, -light_dir), r0);
                        diffuse = diffuse + ndotl * rec.mat.kd * light_color;
                        
                        vec3 view_dir = normalize(r.origin() - rec.p);
                        vec3 reflected_dir = reflect(light_dir, rec.normal); 

                        Real spec = pow(max(dot(view_dir, reflected_dir), r0), rec.mat.exponent);
                        specular = specular + rec.mat.ks * spec * light_color;
                        break;
                    }
                case light::Point:
                    {
                        vec3 light_dir = (rec.p - l.get_position());
                        Real dist2light = length(light_dir);
                        light_dir = light_dir / dist2light;
                        Real dist_sqr = 1;//dist2light*dist2light;

                        ray sh_r(rec.p, -light_dir);
                        bool b_in_shadow = ray_shadow(sh_r, world, dist2light);
                        if(b_in_shadow)
                            break;

                        const color& light_color = l.get_color();
                        Real ndotl = max(dot(rec.normal, -light_dir), r0);
                        diffuse = diffuse + ndotl * light_color * rec.mat.kd / dist_sqr;

                        vec3 view_dir = normalize(r.origin() - rec.p);
                        vec3 reflected_dir = reflect(light_dir, rec.normal); 

                        // Blinn-Phong
                        //vec3 h = Real(0.5)*(-light_dir + rec.normal);
                        //Real spec = pow(max(dot(h, rec.normal), r0), rec.mat.exponent);
                        // Phong
                        Real spec = pow(max(dot(view_dir, reflected_dir), r0), rec.mat.exponent);
                        specular = specular + rec.mat.ks * spec * light_color / dist_sqr;  
                        break;
                    }
            }
        }
        
        color refl = color(1,1,1);
        Real k_refl = 0;
        if(rec.mat.reflectance > r0) {
            vec3 reflected = reflect(normalize(r.direction()), rec.normal);
            if(dot(reflected, rec.normal) > 0) {
                ray r_refl(rec.p, reflected);
                refl = ray_color(r_refl, world, depth_level-1);
                k_refl = rec.mat.reflectance;
#ifdef USE_FRESNEL
                vec3 incident = -normalize(r.origin() - rec.p);
                k_refl = fresnel(1.0, rec.mat.refraction_iof, rec.normal, incident, k_refl); 
#endif
            }
        }

        return ((ambient + diffuse) * rec.mat.albedo + specular) * (1-k_refl) + refl * k_refl;
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
#ifdef USE_GAMMA_CORRECTION
    pixel.x = std::sqrt(scale * pixel.x);
    pixel.y = std::sqrt(scale * pixel.y);
    pixel.z = std::sqrt(scale * pixel.z);
#else
    pixel = scale * pixel;
#endif

    int ir = (int)(255 * clamp(pixel.x, Real(0), Real(1)));
    int ig = (int)(255 * clamp(pixel.y, Real(0), Real(1)));
    int ib = (int)(255 * clamp(pixel.z, Real(0), Real(1)));

    //fprintf(fh, "%d %d %d\n", ir, ig, ib);
    fprintf(fh, "%d %d %d\n", ir, ig, ib);
}

int main(int argc, char** argv) {

    std::string scene_filename;
    std::string output_filename = "result.ppm";
    if(argc > 1) {
        scene_filename = argv[1];
    } else {
        printf("No filename given, usage:\n\t %s <scene xml file>\n", argv[0]);
        return -1;
    }

    int image_width = 512;
    int image_height = 512;
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
    if (scene_filename.empty()) {
        my_scene.add_sphere(point3(0, 0, -1), r05,
                            material(color(0.7, 0.3, 0.3)));
        my_scene.add_sphere(point3(0, -100.5, -1), Real(100),
                            material(color(0.8, 0.8, 0)));
    } else {
        if(!my_scene.load(scene_filename.c_str())) {
            return -1;
        }

        const scene::camera_params& cp = my_scene.get_camera_params();
        output_filename = my_scene.get_output_filename();
        // change extension to ppm
        size_t dot_pos = output_filename.find_last_of('.');
        if(dot_pos!=std::string::npos && dot_pos < output_filename.size()-1) {
            output_filename = output_filename.substr(0, dot_pos + 1);
            output_filename.append("ppm");
        }

        image_width = cp.res_x;
        image_height = cp.res_y;
        aspect_ratio = Real(image_width) / Real(image_height);
        vfov = (2.0*cp.hfov) / aspect_ratio;
        cam = camera(cp.pos, cp.lookat, cp.up, vfov, aspect_ratio, aperture, focus_dist);
    }

    FILE* f = fopen(output_filename.c_str(), "w");
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
