//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ROOMEVENTMANAGER_H
#define ROOMENGINE_ROOMEVENTMANAGER_H
#include <any>
#include <unordered_map>
#include <vector>

#include "game.pb.h"
#include "data/RoomEvent.h"
#include "utils/ProtoAlias.h"


class World;
enum class EventType;

class RoomEventManager {
    std::vector<RoomEvent> roomEvents;
    World& world;

public:
    RoomEventManager(World& world) : world(world) {};

    void addRoomEvent(
        std::string name,
        std::vector<Action> requirements,
        std::vector<System> operations,
        std::vector<Output> outputActions,
        float cooldown = 0.0f,
        std::vector<Parameter> parameters = {}
        );

    RoomEvent* addRoomEvent(std::string name);

    void addRoomEvent(RoomEvent* roomEvent);

    std::vector<RoomEvent>& getRoomEvents() {
        return roomEvents;
    }

   RoomEvent* getRoomEvent(std::string name);

    void triggerRoomEvent(std::string name);
};


#endif //ROOMENGINE_ROOMEVENTMANAGER_H