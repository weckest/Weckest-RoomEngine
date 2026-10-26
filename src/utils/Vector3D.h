//
// Created by Weckest on 2026-10-14.
//

#ifndef ROOMENGINE_VECTOR3D_H
#define ROOMENGINE_VECTOR3D_H
#include <iostream>


class Vector3D {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3D() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3D& normalize();

    float length() const;

    float dot(const Vector3D& vector) const;

    Vector3D operator*(float scalar) const;
    Vector3D operator/(float scalar) const;
    Vector3D operator+(const Vector3D& vector) const;
    Vector3D operator-(const Vector3D& vector) const;
    Vector3D operator+=(const Vector3D& vector);
    Vector3D operator-=(const Vector3D& vector);
    Vector3D operator*=(float scalar);
    Vector3D operator/=(float scalar);

    bool operator==(const Vector3D& vector) const;
    bool operator!=(const Vector3D& vector) const;

    Vector3D operator-() const;
};

inline std::ostream& operator<<(std::ostream& os, const Vector3D& vector) {

    os << "(" << static_cast<int>(vector.x) << ", " << static_cast<int>(vector.y) << ", " << static_cast<int>(vector.z) << ")";
    return os;
}



#endif //ROOMENGINE_VECTOR3D_H