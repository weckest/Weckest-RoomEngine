//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ROOMEVENT_H
#define ROOMENGINE_ROOMEVENT_H
#include <vector>
#include <unordered_map>

#include "Actions.h"

struct RoomEvent {
    std::string name;
    std::vector<Action> actions;
    std::vector<std::string> requirements;
    std::vector<std::string> operations;
    std::vector<std::string> outputActions;
    std::unordered_map<std::string, std::string> parameters;
    bool ready;
    float cooldown;
    float currentCooldown;
};

#endif //ROOMENGINE_ROOMEVENT_H