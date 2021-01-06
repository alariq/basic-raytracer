#pragma once

#include "config.h"
#include "vec.h"
#include "sphere.h"
#include "mesh.h"
#include "light.h"
#include "material.h"

#include <vector>
#include <string>
#include "tinyxml2/tinyxml2.h"

class scene {
    public:
    struct camera_params {
        point3 pos;
        point3 lookat;
        vec3 up;
        Real hfov;
        int res_x;
        int res_y;
        int max_bounces;
    };
    private:
    std::vector<sphere> spheres;
    std::vector<light> lights;
    std::vector<mesh*> meshes;
    color ambient_colour;
    color background_colour;
    camera_params cam_params;
    std::string scene_filename;
    std::string output_filename;

    public:

    bool load(const char* filename);

    scene():background_colour(-1,-1,-1) {}
    ~scene();

    const std::vector<light> &get_lights() const { return lights; }

    bool intersect(const ray &r, Real t_min, Real t_max, hit_info& hit) const {
        bool b_hit = false;
        for(const auto& s: spheres) {
            if(s.hit(r, t_min, t_max, hit)) {
                t_max = hit.t;
                hit.mat = s.get_material();
                b_hit = true;
            }
        }

        for(const auto& m: meshes) {
            if(m->hit(r, t_min, t_max, hit)) {
                t_max = hit.t;
                hit.mat = m->get_material();
                b_hit = true;
            }
        }

        return b_hit;
    }

    void add_sphere(const point3& pos, Real radius, const material& mat) {
        spheres.emplace_back(pos, radius, mat);
    }

    void add_mesh(const struct ObjFile* obj, const material& mat) {
        meshes.emplace_back(new mesh(obj, mat));
    }

    void add_light(const light& l) {
        lights.push_back(l);
    }

    void set_background(color bg) { background_colour = bg; }
    color get_background() const { return background_colour; }
    bool has_background() const { return background_colour.x>=0; }

    void set_ambient(color amb) { ambient_colour = amb; }
    color get_ambient() const { return ambient_colour; }

    const camera_params& get_camera_params() const { return cam_params; }
    const std::string get_output_filename() const { return output_filename; }

    private:
      bool read_camera(const class tinyxml2::XMLElement *el,
                       scene::camera_params *cp);
      bool read_lights(const class tinyxml2::XMLElement *el, color* ambient, std::vector<light>* lights);
      bool read_spheres(const class tinyxml2::XMLElement *el, std::vector<sphere>* spheres);
      bool read_meshes(const class tinyxml2::XMLElement *el, std::vector<mesh*>* meshes);
      bool read_material_solid(const class tinyxml2::XMLElement *el, material* mat);

      color read_colour(const class tinyxml2::XMLElement *el, bool *b_success);
      vec3 read_vec3(const class tinyxml2::XMLElement *el, bool *b_success);
      bool read_named_float_attr(const class tinyxml2::XMLElement *el,
                                 const char *name, float *value);
};
