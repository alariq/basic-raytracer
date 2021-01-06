#include "mesh.h"
#include "config.h"
#include "vec.h"
#include "obj_loader.h"

mesh::~mesh() {
    delete obj_model;
}

mesh::mesh(const struct ObjFile* obj, const material& m):obj_model(obj), mat(m) {
}

bool ray_tri_intersect( 
    const vec3 &orig, const vec3 &dir, 
    const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3* normal,
    vec3& p, Real &t) 
{ 
    vec3 n;
    if(!normal) {
        vec3 v0v1 = v1 - v0; 
        vec3 v0v2 = v2 - v0; 
        // no need to normalize (we will divide on dot so length will cancel out)
        n = cross(v0v1, v0v2);
    } else {
        n = *normal;
    }
 
    // check if ray and plane are parallel ?
    const Real n_dot_ray = dot(n, dir); 
    if (fabs(n_dot_ray) < kEps)
        return false;
 
    // compute distance to the plane
    const Real dist = dot(n, v0); 
 
    // compute t 
    t = (-dot(n,orig) + dist) / n_dot_ray; 
    if (t < 0) return false; // the triangle is behind 
 
    // ray dir vs. plane intersection point
    p = orig + t * dir; 
 
    // inside-outside tests
    vec3 c;
 
    // edge 0
    vec3 edge0 = v1 - v0; 
    vec3 vp0 = p - v0; 
    c = cross(edge0, vp0); 
    if (dot(n, c) < 0) return false; // P is on the right side 
 
    // edge 1
    vec3 edge1 = v2 - v1; 
    vec3 vp1 = p - v1; 
    c = cross(edge1, vp1); 
    if (dot(n, c) < 0)  return false; // P is on the right side 
 
    // edge 2
    vec3 edge2 = v0 - v2; 
    vec3 vp2 = p - v2; 
    c = cross(edge2, vp2); 
    if (dot(n, c) < 0) return false; // P is on the right side; 
 
    return true; // this ray hits the triangle 
} 

bool mesh::hit(const ray &r, Real t_min, Real t_max, hit_info &rec) const {

    int num_tris = (int)obj_model->faces.size() / 3;
    rec.t = t_max;
    bool b_intersected = false;
    for(int i=0;i<num_tris;++i) {
        vec3 v0 = obj_model->p[obj_model->faces[3*i + 0].p - 1];
        vec3 v1 = obj_model->p[obj_model->faces[3*i + 1].p - 1];
        vec3 v2 = obj_model->p[obj_model->faces[3*i + 2].p - 1];
        // get normal from first point normal(assume flat normals)
        vec3 n = obj_model->n[obj_model->faces[3*i + 0].n - 1];

        Real t;
        vec3 p;
        // flip normal because we want to check for interior
        if(ray_tri_intersect(r.origin(), r.direction(), v0, v1, v2, &n, p, t) && t > t_min && t < t_max) {
            if(t < rec.t) {
                rec.t = t;
                rec.p = p;
                rec.normal = n;
                b_intersected = true;
            }
        }
    }

    if(b_intersected) {
        rec.mat = mat;
    }

    return b_intersected;

}
