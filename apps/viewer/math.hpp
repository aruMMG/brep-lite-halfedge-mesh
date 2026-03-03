#pragma once
#include <cmath>

struct Vec3 { float x,y,z; };

inline Vec3 operator-(Vec3 a, Vec3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vec3 operator+(Vec3 a, Vec3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vec3 operator*(Vec3 a, float s){ return {a.x*s,a.y*s,a.z*s}; }

inline float dot(Vec3 a, Vec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b){
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
inline Vec3 normalize(Vec3 v){
    float l = std::sqrt(dot(v,v));
    return (l > 0.0f) ? Vec3{v.x/l, v.y/l, v.z/l} : Vec3{0,0,0};
}

struct Mat4 {
    // column-major (OpenGL)
    float m[16]{};

    static Mat4 identity() {
        Mat4 r;
        r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f;
        return r;
    }
};

inline Mat4 mul(const Mat4& A, const Mat4& B){
    Mat4 R{};
    for(int c=0;c<4;c++){
        for(int r=0;r<4;r++){
            R.m[c*4+r] =
                A.m[0*4+r]*B.m[c*4+0] +
                A.m[1*4+r]*B.m[c*4+1] +
                A.m[2*4+r]*B.m[c*4+2] +
                A.m[3*4+r]*B.m[c*4+3];
        }
    }
    return R;
}

inline Mat4 perspective(float fovy_rad, float aspect, float znear, float zfar){
    Mat4 r{};
    float f = 1.0f / std::tan(fovy_rad * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zfar + znear) / (znear - zfar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zfar * znear) / (znear - zfar);
    return r;
}

inline Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up){
    Vec3 f = normalize(center - eye);
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 r = Mat4::identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8]  = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9]  = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;

    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] =  dot(f, eye);
    return r;
}

inline Mat4 rotateY(float rad){
    Mat4 r = Mat4::identity();
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0] = c;  r.m[8] = s;
    r.m[2] = -s; r.m[10] = c;
    return r;
}