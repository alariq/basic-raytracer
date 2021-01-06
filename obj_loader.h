#include "vec.h"

#include <vector>
#include <string>

struct ObjVertexId {
    int32_t p;
    int32_t n;

    ObjVertexId(int32_t p_, int32_t n_):
        p(p_), n(n_) { }
};

struct ObjVertex {
    vec3 p;
    vec3 n;
};

struct ObjFile {
    std::vector<vec3> p;
    std::vector<vec3> n;
    std::vector<ObjVertexId> faces;
    std::string material_name;
};

ObjFile* load_obj_from_file(const char* file);
