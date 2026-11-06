//
// Created by Weckest on 2026-10-19.
//

#include "HealthSystem.h"

#include "data/ActionContext.h"
#include "ecs/World.h"

HealthSystem::HealthSystem(World &world) {
    world.getSystemManager().addOperation("health.heal",
        [](ActionContext& context, World& world) {
            if (!context.source || !context.hasParameter("player.tag")) return -1;

            if (!context.source.value()->hasComponent<Health>() ||
                !context.source.value()->hasComponent<Tag>()) return -2;

            auto& health = context.source.value()->getComponent<Health>();
            auto& tag = context.source.value()->getComponent<Tag>();
            auto& animations = context.source.value()->getComponent<Animations>();

            health.health = std::clamp(health.health + health.healAmount, 0.0f, 100.0f);

            std::cout << health.health << std::endl;

            if (tag.rendered && context.source.value()->hasComponent<Enemy>()) {
                capstone::render::RenderEvent event;
                event.set_entity_id(tag.tag);
                event.set_type(capstone::render::RENDER_PLAY_ANIMATION);
                auto* a = event.mutable_play_animation();
                a->set_animation_name(animations.animations.find("heal")->second);
                a->set_animation_after(animations.animations.find("idle")->second);
                world.getRenderService().SendEvent(event);
            }

            return 1;
        }, {"player.tag"}, {});

    world.getSystemManager().addOperation("health.update",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("player.tag")) return -1;

            for (auto& entity : world.getEntities()) {
                if (entity->hasComponent<Tag>() && entity->hasComponent<Health>()) {
                    auto& health = entity->getComponent<Health>();
                    auto& tag = entity->getComponent<Tag>();
                    if (tag.tag == *context.getParameter<std::string>("player.tag")) {
                        if (tag.rendered) {
                            capstone::render::RenderEvent event;
                            event.set_entity_id(*context.getParameter<std::string>("player.tag"));
                            event.set_type(capstone::render::RENDER_CHANGE_HEALTH);
                            auto* h = event.mutable_change_health();
                            h->set_health(health.health);
                            world.getRenderService().SendEvent(event);
                        }
                    }
                }
            }




            return 1;
        }, {"player.tag"});

}

void HealthSystem::update(World &world) {
    auto& entities = world.getEntities();
    for (auto& entity : entities) {
        if (entity->hasComponent<Health>() &&
            entity->hasComponent<Tag>() &&
            entity->hasComponent<Animations>()) {
            auto& health = entity->getComponent<Health>();
            auto& tag = entity->getComponent<Tag>();
            auto& animations = entity->getComponent<Animations>();

            if (health.health <= 0 && tag.rendered) {
                capstone::render::RenderEvent event;
                event.set_type(capstone::render::RENDER_PLAY_ANIMATION);
                event.set_entity_id(tag.tag);
                event.mutable_play_animation()->set_animation_name(animations.animations.find("death")->second);
                event.mutable_play_animation()->set_animation_after(animations.animations.find("death_pose")->second);
                world.getRenderService().SendEvent(event);
                tag.rendered = false;

                world.getRoomEventManager().triggerRoomEvent("Spawn Win");
            }
        }
    }
}
