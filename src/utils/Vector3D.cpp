//
// Created by Weckest on 2026-10-14.
//

#include "Vector3D.h"

Vector3D & Vector3D::normalize() {
    float length = this->length();

    if (length > 0) {
        this->x /= length;
        this->y /= length;
        this->z /= length;
    }

    return *this;
}

float Vector3D::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3D::dot(const Vector3D &vector) const {
    return (x * vector.x + y * vector.y + z * vector.z);
}

Vector3D Vector3D::operator*(float scalar) const {
    return {x * scalar, y * scalar, z * scalar};
}

Vector3D Vector3D::operator/(float scalar) const {
    return {x / scalar, y / scalar, z / scalar};
}

Vector3D Vector3D::operator+(const Vector3D &vector) const {
    return {x + vector.x, y + vector.y, z + vector.z};
}

Vector3D Vector3D::operator-(const Vector3D &vector) const {
    return {x - vector.x, y - vector.y, z - vector.z};
}

Vector3D Vector3D::operator+=(const Vector3D &vector) {
    x += vector.x;
    y += vector.y;
    z += vector.z;
    return *this;
}

Vector3D Vector3D::operator-=(const Vector3D &vector) {
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
    return *this;
}

Vector3D Vector3D::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3D Vector3D::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Vector3D::operator==(const Vector3D &vector) const {
    return (x == vector.x && y == vector.y && z == vector.z);
}

bool Vector3D::operator!=(const Vector3D &vector) const {
    return (x != vector.x || y != vector.y || z != vector.z);
}

Vector3D Vector3D::operator-() const {
    return {-x, -y, -z};
}
