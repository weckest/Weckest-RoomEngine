//
// Created by Weckest on 2026-10-19.
//

#include "CombatSystem.h"

#include "data/ActionContext.h"
#include "ecs/World.h"

CombatSystem::CombatSystem(World &world) {
    world.getSystemManager().addOperation("combat.attack.calculate",
        [](ActionContext& context, World& world) {
            if (!context.source) return -1;

            if (!context.source.value()->hasComponent<Player>() || !context.source.value()->hasComponent<Sword>()) {
                std::cout << "Source is not a player and does not have a sword" << std::endl;
                return -1;
            }

            auto& player = context.source.value()->getComponent<Player>();
            auto& sword = context.source.value()->getComponent<Sword>();

            context.setParameter("damage", player.damage * sword.damage);
            return 1;
        }, {}, {"damage"});

    world.getSystemManager().addOperation("combat.attack.apply",
        [](ActionContext& context, World& world) {
            if (!context.source || !context.target) return -1;

            auto damage = context.getParameter<float>("damage");
            if (damage) {
                if (context.target.value()->hasComponent<Enemy>() &&
                    context.target.value()->hasComponent<Health>() &&
                    context.target.value()->hasComponent<Animations>() &&
                    context.target.value()->hasComponent<Tag>()) {

                    auto& enemy = context.target.value()->getComponent<Enemy>();
                    auto& health = context.target.value()->getComponent<Health>();
                    auto& animations = context.target.value()->getComponent<Animations>();
                    auto& tag = context.target.value()->getComponent<Tag>();

                    health.health = std::clamp(health.health - *damage, 0.0f, 100.0f);
                    std::cout << "Enemy Health: " << health.health << " Damage: " << *damage << std::endl;

                    capstone::render::RenderEvent event;
                    if (tag.rendered) {
                        event.set_entity_id(tag.tag);
                        event.set_type(capstone::render::RENDER_PLAY_ANIMATION);
                        auto* a = event.mutable_play_animation();
                        a->set_animation_name(animations.animations.find("damage")->second);
                        a->set_animation_after(animations.animations.find("idle")->second);
                        world.getRenderService().SendEvent(event);
                    }
                }
            }

            return 1;
        }, {"damage"});
}
