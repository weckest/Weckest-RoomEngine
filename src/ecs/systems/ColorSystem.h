//
// Created by Weckest on 2026-10-15.
//

#ifndef ROOMENGINE_COLORSYSTEM_H
#define ROOMENGINE_COLORSYSTEM_H
#include <random>


class World;

class ColorSystem {
public:
    ColorSystem(World& world);

private:
    int generateRandomNumber(std::mt19937& gen, int min, int max) const;
};


#endif //ROOMENGINE_COLORSYSTEM_H