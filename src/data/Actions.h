//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_INPUTACTION_H
#define ROOMENGINE_INPUTACTION_H
#include <string>

#include "pb.h"

class Entity;

enum ActionType {
    GENERAL,
    MOVEUP,
    MOVEDOWN,
    MOVELEFT,
    MOVERIGHT,
    COLOR,
    ATTACK,
    BLOCK,
    HEAL
};

struct InputAction {
    pb_size_t inputType{};
    const std::string deviceId{};
    int inputId{};
    ActionType actionType{};
};

struct OutputAction {
    pb_size_t outputType{};
    std::string deviceId{};
    int outputId{};
};

struct Action {
    std::string deviceId{};
    Entity* entity{};
    ActionType actionType{};
};

#endif //ROOMENGINE_INPUTACTION_H