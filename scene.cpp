#include "scene.h"

#include "config.h"
#include "tinyxml2/tinyxml2.h"
#include "vec.h"
#include "obj_loader.h"
#include "material.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>

scene::~scene() {
    for(auto& mesh: meshes) {
        delete mesh;
    }
}

bool scene::load(const char* filename) {
    
    using namespace tinyxml2;

    XMLError err = XML_SUCCESS;
	XMLDocument doc;
    err = doc.LoadFile(filename);
    if(err != XML_SUCCESS) {
        printf("Failed to parse XML file: %s\n", filename);
        return false;
    }

    scene_filename = filename;

	XMLElement* scene_el = doc.FirstChildElement("scene");
    const char* output_file = nullptr;
    err = scene_el->QueryStringAttribute("output_file", &output_file);
    if(err != XML_SUCCESS) {
        printf("Failed to parse XML file: %s\n", filename);
        return false;
    }
    output_filename = output_file;

    XMLElement* bg_colour = scene_el->FirstChildElement("background_color");
    bool b_success = false;
    color c = read_colour(bg_colour, &b_success);
    set_background(c);

    XMLElement* camera_el = scene_el->FirstChildElement("camera");
    b_success &= read_camera(camera_el, &cam_params);

    lights.clear();
    XMLElement* lights_el = scene_el->FirstChildElement("lights");
    b_success &= read_lights(lights_el, &ambient_colour, &lights);

    spheres.clear();
    XMLElement* surfaces_el = scene_el->FirstChildElement("surfaces");
    b_success &= read_spheres(surfaces_el, &spheres);

    meshes.clear();
    b_success &= read_meshes(surfaces_el, &meshes);

    return b_success;
}

bool scene::read_camera(const class tinyxml2::XMLElement* el, scene::camera_params* cp) {
    using namespace tinyxml2;

    bool b_success = false;

    const XMLElement* position_el = el->FirstChildElement("position");
    cp->pos = read_vec3(position_el, &b_success);

    const XMLElement* lookat_el = el->FirstChildElement("lookat");
    cp->lookat = read_vec3(lookat_el, &b_success);

    const XMLElement* up_el = el->FirstChildElement("up");
    cp->up = read_vec3(up_el, &b_success);

    const XMLElement* hfov_el = el->FirstChildElement("horizontal_fov");
    const XMLElement* res_el = el->FirstChildElement("resolution");
    const XMLElement* max_bounces_el = el->FirstChildElement("max_bounces");

    float hfov, res_x, res_y;
    float max_bounces;
    b_success = read_named_float_attr(hfov_el, "angle", &hfov);
    b_success &= read_named_float_attr(res_el, "horizontal", &res_x);
    b_success &= read_named_float_attr(res_el, "vertical", &res_y);
    b_success &= read_named_float_attr(max_bounces_el, "n", &max_bounces);

    if(!b_success) {
        printf("Error reading camera\n");
        return false;
    }

    cp->hfov = (Real)hfov;
    cp->res_x = (int)res_x;
    cp->res_y = (int)res_y;
    cp->max_bounces = (int)max_bounces;

    return true;
}

bool scene::read_lights(const class tinyxml2::XMLElement *el, color* ambient, std::vector<light>* lights) {

    using namespace tinyxml2;
    bool b_succes = false;
    const XMLElement* amb_col_el = el->FirstChildElement("ambient_light")->FirstChildElement("color");
    *ambient = read_colour(amb_col_el, &b_succes);

    const XMLElement* parallel_light_el = el->FirstChildElement("parallel_light");
    if (parallel_light_el)
    {
        do {
            const XMLElement* par_col_el = parallel_light_el->FirstChildElement("color");
            const XMLElement* par_dir_el = parallel_light_el->FirstChildElement("direction");
            color c = read_colour(par_col_el, &b_succes);
            vec3 dir = read_vec3(par_dir_el, &b_succes);
            lights->emplace_back(dir,c, light::Directional);

        } while((parallel_light_el = parallel_light_el->NextSiblingElement("parallel_light")));
    }

    const XMLElement* point_light_el = el->FirstChildElement("point_light");
    if (point_light_el)
    {
        do {
            const XMLElement* pt_col_el = point_light_el->FirstChildElement("color");
            const XMLElement* pt_pos_el = point_light_el->FirstChildElement("position");
            color c = read_colour(pt_col_el, &b_succes);
            vec3 pos = read_vec3(pt_pos_el, &b_succes);
            lights->emplace_back(pos, c, light::Point);

        } while((point_light_el = point_light_el->NextSiblingElement("point_light")));
    }

    return b_succes;
}

bool scene::read_spheres(const class tinyxml2::XMLElement *el, std::vector<sphere>* spheres) {

    using namespace tinyxml2;
    bool b_success = true;

    const XMLElement* sphere_el = el->FirstChildElement("sphere");
    if (sphere_el)
    {
        do {
            float radius;
            b_success &= read_named_float_attr(sphere_el, "radius", &radius);

            const XMLElement* pos_el = sphere_el->FirstChildElement("position");
            vec3 pos = read_vec3(pos_el, &b_success);

            const XMLElement* material_solid_el = sphere_el->FirstChildElement("material_solid");
            material mat;
            b_success &= read_material_solid(material_solid_el, &mat);

            if(!b_success) {
                printf("Failed reading surfaces: %s:%d\n", __FILE__, __LINE__);
            }

            spheres->emplace_back(pos, Real(radius), mat);

        } while((sphere_el = sphere_el->NextSiblingElement("sphere")));
    }

    return b_success;

}

bool scene::read_meshes(const class tinyxml2::XMLElement *el, std::vector<mesh*>* meshes) {

    using namespace tinyxml2;
    bool b_success = true;

    const XMLElement* mesh_el = el->FirstChildElement("mesh");
    if (mesh_el)
    {
        do {
            const char* name_attr;
            XMLError err = mesh_el->QueryStringAttribute("name", &name_attr);
            if(err != XML_SUCCESS) {
                printf("Error reading mesh name attr\n");
                return false;
            }
            std::string obj_filename;
            // assume file name is relative to scene xml file
            size_t path_end = scene_filename.find_last_of('/');
            if(path_end) {
                obj_filename = scene_filename.substr(0, path_end+1);
                obj_filename.append(name_attr);
            } else {
                obj_filename = name_attr;
            }

            ObjFile* obj_model = load_obj_from_file(obj_filename.c_str());
            if(!obj_model) {
                printf("Failed to load obj model from: %s\n", obj_filename.c_str());
                return false;
            }

            const XMLElement* material_solid_el = mesh_el->FirstChildElement("material_solid");
            material mat;
            b_success &= read_material_solid(material_solid_el, &mat);

            if(!b_success) {
                printf("Failed reading surfaces: %s:%d\n", __FILE__, __LINE__);
            }

            meshes->push_back(new mesh(obj_model, mat));

        } while((mesh_el = mesh_el->NextSiblingElement("mesh")));
    }

    return b_success;
}

bool scene::read_material_solid(const class tinyxml2::XMLElement *el, material* mat) {

    using namespace tinyxml2;
    bool b_success = false;
    const XMLElement* color_el = el->FirstChildElement("color");
    mat->albedo = read_colour(color_el, &b_success);

    const XMLElement* phong_el = el->FirstChildElement("phong");
    b_success &= read_named_float_attr(phong_el, "ka", &mat->ka);
    b_success &= read_named_float_attr(phong_el, "kd", &mat->kd);
    b_success &= read_named_float_attr(phong_el, "ks", &mat->ks);
    b_success &= read_named_float_attr(phong_el, "exponent", &mat->exponent);

    const XMLElement* reflectance_el = el->FirstChildElement("reflectance");
    b_success &= read_named_float_attr(reflectance_el, "r", &mat->reflectance);

    const XMLElement* transmittance_el = el->FirstChildElement("transmittance");
    b_success &= read_named_float_attr(transmittance_el, "t", &mat->transmittance);

    const XMLElement* refraction_el = el->FirstChildElement("refraction");
    b_success &= read_named_float_attr(refraction_el, "iof", &mat->refraction_iof);

    return b_success;
}

bool scene::read_named_float_attr(const class tinyxml2::XMLElement* el, const char* name, float* value) {

    using namespace tinyxml2;
    XMLError err = el->QueryFloatAttribute(name, value);
    if(err != XML_SUCCESS) {
        printf("Error reading float attr: %s\n", name);
        return false;
    }
    return true;
}

color scene::read_colour(const tinyxml2::XMLElement* el, bool* b_success) {

    using namespace tinyxml2;
    color c;
    *b_success = true;
    XMLError err0 = el->QueryFloatAttribute("r", &c.x);
    XMLError err1 = el->QueryFloatAttribute("g", &c.y);
    XMLError err2 = el->QueryFloatAttribute("b", &c.z);
    if(err0 != XML_SUCCESS || err1 !=XML_SUCCESS || err2 != XML_SUCCESS) {
        printf("Error reading colour from %s\n", el->Name());
        *b_success = false;
    }
    return c;
}

vec3 scene::read_vec3(const class tinyxml2::XMLElement* el, bool* b_success) {

    using namespace tinyxml2;
    vec3 v;
    *b_success = true;
    XMLError err0 = el->QueryFloatAttribute("x", &v.x);
    XMLError err1 = el->QueryFloatAttribute("y", &v.y);
    XMLError err2 = el->QueryFloatAttribute("z", &v.z);
    if(err0 != XML_SUCCESS || err1 !=XML_SUCCESS || err2 != XML_SUCCESS) {
        printf("Error reading colour from %s", el->Name());
        *b_success = false;
    }
    return v;
}

