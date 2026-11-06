//
// Created by Weckest on 2026-10-14.
//

#include "MovementSystem.h"

#include "data/ActionContext.h"
#include "ecs/World.h"

MovementSystem::MovementSystem(World &world) {
    world.getSystemManager().addOperation("movement.up.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.y = std::clamp(sourceVelocity.direction.y += 0.2f, -1.0f, 1.0f);
            return 1;
        });

    world.getSystemManager().addOperation("movement.up.stop",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.y = 0.0f;
            return 1;
        });

    world.getSystemManager().addOperation("movement.down.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.y = std::clamp(sourceVelocity.direction.y -= 0.2f, -1.0f, 1.0f);
            return 1;
    });

    world.getSystemManager().addOperation("movement.down.stop",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.y = 0.0f;
            return 1;
    });

    world.getSystemManager().addOperation("movement.left.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.x = std::clamp(sourceVelocity.direction.x -= 0.2f, -1.0f, 1.0f);
            return 1;
        });

    world.getSystemManager().addOperation("movement.left.stop",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.x = 0.0f;
            return 1;
        });

    world.getSystemManager().addOperation("movement.right.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.x = std::clamp(sourceVelocity.direction.x += 0.2f, -1.0f, 1.0f);
            return 1;
        });

    world.getSystemManager().addOperation("movement.right.stop",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.x = 0.0f;
            return 1;
        });

    world.getSystemManager().addOperation("movement.forward.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.z = std::clamp(sourceVelocity.direction.z += 0.2f, -1.0f, 1.0f);
            return 1;
        });

    world.getSystemManager().addOperation("movement.forward.stop",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.z = 0.0f;
            return 1;
        });

    world.getSystemManager().addOperation("movement.backward.start",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            auto& source = context.source;

            auto& sourceVelocity = source.value()->getComponent<Velocity>();

            sourceVelocity.direction.z = std::clamp(sourceVelocity.direction.z -= 0.2f, -1.0f, 1.0f);
            return 1;
        });

    world.getSystemManager().addOperation("movement.backward.stop",
       [](ActionContext& context, World& world) {
           if (!context.source) return -1;

           auto& source = context.source;

           auto& sourceVelocity = source.value()->getComponent<Velocity>();

           sourceVelocity.direction.z = 0.0f;
           return 1;
       });
}

void MovementSystem::update(World &world, float dt) {
    for (auto& e : world.getEntities()) {
        if (e->hasComponent<Transform>() && e->hasComponent<Velocity>()) {
            auto& transform = e->getComponent<Transform>();
            auto& velocity = e->getComponent<Velocity>();

            transform.oldPosition = transform.position;

            Vector3D directionVector = velocity.direction;

            directionVector.normalize();

            Vector3D velocityVector = directionVector * velocity.speed;
            transform.position += velocityVector * dt;

            if (e->hasComponent<Tag>() && e->getComponent<Tag>().rendered) {
                if (transform.position != transform.oldPosition) {
                    // std::cout << "Entity moved, calling renderer" << std::endl;
                    world.getRenderService().EntityMoved(e->getComponent<Tag>().tag, transform);
                }
            }
        }
    }
}
