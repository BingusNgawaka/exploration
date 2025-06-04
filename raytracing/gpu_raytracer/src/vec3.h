#include <cmath>
#include <iostream>

struct Vec3 {
    float x, y, z;

    Vec3(): x(0), y(0), z(0){}
    Vec3(float x, float y, float z): x(x), y(y), z(z){}

    float operator[](int i) const {
        if(i == 0){
            return x;
        }else if(i == 1){
            return y;
        }else if(i == 2){
            return z;
        }else{
            std::cerr << "oops, vec3 out of index! owo";
            return 0;
        }
    }

    const Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    const Vec3 operator-(const Vec3& other) const{
        return Vec3(x-other.x, y-other.y, z-other.z);
    }

    const Vec3 operator+(const Vec3& other) const{
        return Vec3(x+other.x, y+other.y, z+other.z);
    }

    const Vec3 operator*(const float t){
        return Vec3(x*t, y*t, z*t);
    }

    const Vec3 operator*(const Vec3& other){
        return Vec3(x*other.x, y*other.y, z*other.z);
    }

    Vec3 operator/(const float t){
        return Vec3(x/t, y/t, z/t);
    }

    Vec3 operator=(const Vec3& other){
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Vec3 operator+=(const Vec3& other){
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 operator-=(const Vec3& other){
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3 operator*=(const float t){
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3 operator/=(const float t){
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    const float magSqrd() const{
        return x*x+y*y+z*z;
    }

    const float mag() const{
        return std::sqrt(magSqrd());
    }

    const Vec3 normalized() const{
        float n = 1/mag();
        return Vec3(x*n,y*n,z*n);
    }

    const bool near_zero() const{
        float epsilon {1e-8};
        return (std::fabs(x)<epsilon && std::fabs(y)<epsilon && std::fabs(z)<epsilon);
    }
    Vec3 rotate(const Vec3& axisOfRot, float theta) const;
};
const Vec3 operator*(float t, const Vec3& v){
    return Vec3(t*v.x, t*v.y, t*v.z);
}
const std::ostream& operator<<(std::ostream& out, const Vec3& v){
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
float dot(const Vec3& v1, const Vec3& v2){
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}
Vec3 cross(const Vec3& v1, const Vec3& v2){
    return Vec3(
            v1.y*v2.z-v1.z*v2.y,
            v1.z*v2.x-v1.x*v2.z,
            v1.x*v2.y-v1.y*v2.x
    );
}
Vec3 Vec3::rotate(const Vec3& axisOfRot, float theta) const{
    // axisOfRot is assumed to be normalized already
    // rodrigues' formula cause i dont wanna use quaternions
    // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    return cos(theta)*(*this) + sin(theta)*cross(axisOfRot, *this) + dot(axisOfRot, *this)*(1-cos(theta))*axisOfRot;
}
