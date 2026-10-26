//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ROOMEVENTSYSTEM_H
#define ROOMENGINE_ROOMEVENTSYSTEM_H
#include <vector>

#include "data/RoomEvent.h"

class World;

class RoomEventSystem {

public:
    RoomEventSystem(World& world);

    void update(World& world, float dt);

};

#endif //ROOMENGINE_ROOMEVENTSYSTEM_H