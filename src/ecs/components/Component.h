//
// Created by Weckest on 2026-10-05.
//

#ifndef ROOMENGINE_COMPONENT_H
#define ROOMENGINE_COMPONENT_H
#include "utils/Vector3D.h"

struct Transform {
    Vector3D position{};
    float scale{};
    Vector3D oldPosition{};
};

struct Velocity {
    Vector3D direction{};
    float speed{};
};

struct Tag {
    std::string tag{};
    bool rendered;
};

struct Player {
    float damage{};
};

struct Enemy {
};

struct Health {
    float health{};
    float healAmount{};
    float passive{};
};

struct Sword {
    float damage{};
};

struct Animations {
    std::unordered_map<std::string, std::string> animations;
};

#endif //ROOMENGINE_COMPONENT_H