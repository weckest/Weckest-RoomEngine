//
// Created by Weckest on 2026-11-02.
//

#ifndef ROOMENGINE_SPAWNSYSTEM_H
#define ROOMENGINE_SPAWNSYSTEM_H
#include <random>


class World;

class SpawnSystem {

public:
    SpawnSystem(World& world);

protected:
    int generateRandomNumber(std::mt19937 &gen, int min, int max) const;
};


#endif //ROOMENGINE_SPAWNSYSTEM_H