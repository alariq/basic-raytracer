#include "obj_loader.h"

#include <stdio.h>

static bool read_vec3(const char* line, float* p)
{
    return 3 == sscanf(line, "%f %f %f", p, p+1, p+2);
}

static bool read_vec2(const char* line, float* p)
{
   return 2 == sscanf(line, "%f %f", p, p+1);
}

static bool read_face(const char* line, int32_t* face)
{
    while(!isdigit(*line)) line++;
    int num_read = 0;
    int count = 0;
    do {
        num_read = sscanf(line, "%d", face);
        // skip to next number or next triplet
        while(*line!='/' && *line!=' ' && *line!='\n' && *line!='\0') line++;
        // increment face
        while(*line=='/' || *line==' ') { line++; face++; }
        count += num_read>0 ? 1 : 0;
    } while(num_read>0);

    return count > 0;
}

ObjFile* load_obj_from_file(const char* file)
{
    FILE* fh = fopen(file, "r");
    if(!fh) 
        return nullptr;

    ObjFile* obj = new ObjFile;

    char buf[1024];
    bool is_ok = true;
    while(fgets(buf, 1023, fh))
    {
        float f[3];
        if(buf[0] == '#') 
            continue;

        if(buf[0] == 'v' && buf[1]==' ') {
            is_ok = read_vec3(buf+1, f);
            if(!is_ok)
                break;
            obj->p.push_back(vec3(f[0], f[1], f[2]));
            continue;
        }

        if(buf[0] == 'v' && buf[1]=='t' && buf[2]==' ') {
            is_ok = read_vec2(buf+2, f);
            if(!is_ok)
                break;
            continue;
        }

        if(buf[0] == 'v' && buf[1]=='n' && buf[2]==' ') {
            is_ok = read_vec3(buf+2, f);
            if(!is_ok)
                break;
            vec3 n = normalize(vec3(f[0], f[1], f[2]));
            obj->n.push_back(n);
            continue;
        }

        if(buf[0] == 'f' && buf[1]==' ') {
            int32_t i[9] = {0}; // 0 - no index, as all indices in obj start from 1
            is_ok = read_face(buf+1, i);
            if(!is_ok)
                break;
            obj->faces.push_back(ObjVertexId(i[0], i[2]));
            obj->faces.push_back(ObjVertexId(i[3], i[5]));
            obj->faces.push_back(ObjVertexId(i[6], i[8]));
            continue;
        }
    }

    if(!is_ok) {
        delete obj;
        obj = nullptr;
    }

    fclose(fh);
    return obj;
}


