//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ROOMEVENTMANAGER_H
#define ROOMENGINE_ROOMEVENTMANAGER_H
#include <any>
#include <unordered_map>
#include <vector>

#include "data/RoomEvent.h"



class World;
enum class EventType;

class RoomEventManager {
    std::unordered_map<std::string, RoomEvent> roomEvents;
    World& world;

public:
    RoomEventManager(World& world) : world(world) {};

    void addRoomEvent(
        std::string name,
        std::vector<ActionType> requirements,
        std::vector<std::string> operations,
        std::vector<OutputAction> outputActions,
        float cooldown = 0.0f
        );

    std::unordered_map<std::string, RoomEvent>& getRoomEvents() {
        return roomEvents;
    }

    RoomEvent& getRoomEvent(std::string name) {
        return roomEvents[name];
    }
};


#endif //ROOMENGINE_ROOMEVENTMANAGER_H