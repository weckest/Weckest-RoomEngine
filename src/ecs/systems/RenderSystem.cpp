//
// Created by Weckest on 2026-10-15.
//

#include "RenderSystem.h"

#include "data/ActionContext.h"
#include "ecs/World.h"

RenderSystem::RenderSystem(World &world) {
    world.getSystemManager().addOperation("render.color",
        [](ActionContext& context, World& world) {
            if (!context.source || !context.target) return -1;
            if (!context.source.value()->hasComponent<Tag>()) return -2;

            auto& tag = context.source.value()->getComponent<Tag>();

            capstone::render::RenderEvent event;

            event.set_type(capstone::render::RENDER_CHANGE_COLOR);

            event.set_entity_id(tag.tag);

            auto* color = event.mutable_change_color();
            color->set_r(*context.getParameter<int>("r"));
            std::cout << color->r();
            color->set_g(*context.getParameter<int>("g"));
            std::cout << color->g();
            color->set_b(*context.getParameter<int>("b"));
            std::cout << color->b();
            color->set_a(*context.getParameter<int>("a"));
            std::cout << color->a();

            if (tag.rendered) {
                world.getRenderService().SendEvent(event);
            }

            return 1;
        }, {"r", "g", "b", "a"});

    world.getSystemManager().addOperation("render.text",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("text")) return -1;

            // std::cout << *context.getParameter<std::string>("text") << std::endl;
            capstone::render::RenderEvent event;
            event.set_type(capstone::render::RENDER_DISPLAY_MESSAGE);
            auto dm = event.mutable_display_message();
            dm->set_message(*context.getParameter<std::string>("text"));
            world.getRenderService().SendEvent(event);

            return 1;
        }, {"text"});

    world.getSystemManager().addOperation("render.actions",
        [](ActionContext& context, World& world) {
            auto* inputContext = world.getActionManager().getCurrentContext();
            capstone::render::RenderEvent event;
            event.set_type(capstone::render::RENDER_MODULE_MAPPING);
            auto mm = event.mutable_module_mapping();
            for (auto& action : inputContext->actions) {
                if (action.inputId == 1) {
                    mm->set_select(action.actionType);
                }
                if (action.inputId == 3) {
                    mm->set_top(action.actionType);
                }
                if (action.inputId == 4) {
                    mm->set_bottom(action.actionType);
                }
                if (action.inputId == 5) {
                    mm->set_left(action.actionType);
                }
                if (action.inputId == 6) {
                    mm->set_right(action.actionType);
                }
            }

            world.getRenderService().SendEvent(event);
            return 1;
        });
}
