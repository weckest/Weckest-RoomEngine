//
// Created by Weckest on 2026-10-14.
//

#ifndef ROOMENGINE_MOVEMENTSYSTEM_H
#define ROOMENGINE_MOVEMENTSYSTEM_H


class World;

class MovementSystem {
public:
    MovementSystem(World& world);

    void update(World &world, float dt);
};


#endif //ROOMENGINE_MOVEMENTSYSTEM_H