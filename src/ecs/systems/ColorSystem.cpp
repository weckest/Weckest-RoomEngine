//
// Created by Weckest on 2026-10-15.
//

#include "ColorSystem.h"

#include <random>

#include "data/ActionContext.h"
#include "ecs/World.h"

ColorSystem::ColorSystem(World &world) {
    // 1. Obtain a random seed from the hardware
    std::random_device rd;

    // 2. Initialize the standard Mersenne Twister engine with the seed
    std::mt19937 gen(rd());

    world.getSystemManager().addOperation("color.random",
        [&rd, *this](ActionContext& context, World& world) {

            std::mt19937 gen(rd());
            int r, g, b, a;
            r = generateRandomNumber(gen, 1, 255);
            g = generateRandomNumber(gen, 1, 255);
            b = generateRandomNumber(gen, 1, 255);
            a = generateRandomNumber(gen, 1, 255);

            context.setParameter("r", r);
            context.setParameter("g", g);
            context.setParameter("b", b);
            context.setParameter("a", a);
            return 1;
    }, {}, {"r", "g", "b", "a"});

    world.getSystemManager().addOperation("color.green",
        [&rd, *this](ActionContext& context, World& world) {

            int32_t r, g, b, a;
            r = 0;
            g = 255;
            b = 0;
            a = 255;

            context.setParameter("r", r);
            context.setParameter("g", g);
            context.setParameter("b", b);
            context.setParameter("a", a);
            return 1;
    }, {}, {"r", "g", "b", "a"});

    world.getSystemManager().addOperation("color.red",
        [&rd, *this](ActionContext& context, World& world) {

            int32_t r, g, b, a;
            r = 255;
            g = 0;
            b = 0;
            a = 255;

            context.setParameter("r", r);
            context.setParameter("g", g);
            context.setParameter("b", b);
            context.setParameter("a", a);
            return 1;
    }, {}, {"r", "g", "b", "a"});

    world.getSystemManager().addOperation("color.basic",
        [&rd, *this](ActionContext& context, World& world) {

            int32_t r, g, b, a;
            r = 0;
            g = 0;
            b = 0;
            a = 100;

            context.setParameter("r", r);
            context.setParameter("g", g);
            context.setParameter("b", b);
            context.setParameter("a", a);
            return 1;
    }, {}, {"r", "g", "b", "a"});
}

int ColorSystem::generateRandomNumber(std::mt19937 &gen, int min, int max) const {
    std::uniform_int_distribution<> distr(min, max);
    return distr(gen);
}
