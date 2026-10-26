//
// Created by Weckest on 2026-10-19.
//

#ifndef ROOMENGINE_HEALTHSYSTEM_H
#define ROOMENGINE_HEALTHSYSTEM_H


class World;

class HealthSystem {
public:
    HealthSystem(World& world);

    void update(World& world);
};


#endif //ROOMENGINE_HEALTHSYSTEM_H