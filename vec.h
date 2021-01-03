#pragma once

#include <math.h>

using Real = float;

struct vec3 {
    Real x, y, z;
    vec3() = default;
    vec3(Real xx, Real yy, Real zz):x(xx), y(yy), z(zz) {}
};

using point3 = vec3;
using color = vec3;

template <typename T>
T degrees_to_radians(T degrees) {
    return degrees * Real(M_PI) / Real(180.0);
}


vec3 operator + (const vec3 &u, const vec3 &v){
	return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

vec3 operator + (const vec3 &v, const float s){
	return vec3(v.x + s, v.y + s, v.z + s);
}

vec3 operator - (const vec3 &u, const vec3 &v){
	return vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

vec3 operator - (const vec3 &v, const float s){
	return vec3(v.x - s, v.y - s, v.z - s);
}

vec3 operator - (const vec3 &v){
	return vec3(-v.x, -v.y, -v.z);
}

vec3 operator * (const vec3 &u, const vec3 &v){
	return vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

vec3 operator * (const float s, const vec3 &v){
	return vec3(v.x * s, v.y * s, v.z * s);
}

vec3 operator * (const vec3 &v, const float s){
	return vec3(v.x * s, v.y * s, v.z * s);
}

vec3 operator / (const vec3 &u, const vec3 &v){
	return vec3(u.x / v.x, u.y / v.y, u.z / v.z);
}

vec3 operator / (const vec3 &v, const float s){
	return vec3(v.x / s, v.y / s, v.z / s);
}

bool operator == (const vec3 &u, const vec3 &v){
	return (u.x == v.x && u.y == v.y && u.z == v.z);
}

bool operator != (const vec3 &u, const vec3 &v){
	return (u.x != v.x || u.y != v.y || u.z != v.z);
}

vec3 cross(const vec3 &u, const vec3 &v){
	return vec3(u.y * v.z - v.y * u.z, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

vec3 normalize(const vec3 &v){
	Real invLen = Real(1.0) / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return v * invLen;
}

float lengthSqr(const vec3 &v){
	return v.x * v.x + v.y * v.y + v.z * v.z;
}


////////////////////////////////////////////////////////////////////////////////

inline Real random_Real() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline Real random_Real(Real min, Real max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_Real();
}

vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_Real(-1,1), random_Real(-1,1), 0);
        if (lengthSqr(p) >= 1) continue;
        return p;
    }
}

