#include "scene.h"

#include "config.h"
#include "tinyxml2/tinyxml2.h"
#include "vec.h"
#include "material.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>



const char* load_file(const char* fname, size_t* out_size = nullptr)
{
    assert(fname);
    FILE* f = fopen(fname, "r");
    if(!f)
    {
        printf("Can't open %s \n", fname);
        return nullptr;
    }

    fseek(f, 0, SEEK_SET);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    char* pdata = new char[size + 1];
    size_t  read_size = fread(pdata, size, 1, f);
    assert(read_size == size);
    pdata[size] = '\0';
    if(out_size)
        *out_size = size;

    fclose(f);
    return pdata;
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
    b_success &= read_surfaces(surfaces_el, &spheres);

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
            lights->emplace_back(dir,c);

        } while((parallel_light_el = parallel_light_el->NextSiblingElement("parallel_light")));
    }

    return b_succes;
}

bool scene::read_surfaces(const class tinyxml2::XMLElement *el, std::vector<sphere>* spheres) {

    using namespace tinyxml2;
    bool b_success = false;

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

