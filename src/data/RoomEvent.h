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
    std::vector<ActionType> requirements;
    std::vector<std::string> operations;
    std::vector<OutputAction> outputActions;
    bool ready;
    float cooldown;
    float currentCooldown;
};

#endif //ROOMENGINE_ROOMEVENT_H