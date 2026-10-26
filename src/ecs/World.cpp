//
// Created by Weckest on 2026-09-28.
//

#include "World.h"
#include <grpcpp/grpcpp.h>

World::World(TcpServer &tcpServer) : tcpServer(tcpServer) {
    renderService = new RenderServiceClient(grpc::CreateChannel("127.0.0.1:5002", grpc::InsecureChannelCredentials()));
    renderService->Connect();
}

void World::createModule(TcpConnection &connection, std::string deviceId) {

    for (auto& e : entities) {
        if (e->hasComponent<Module>()) {
            auto& module = e->getComponent<Module>();
            if (module.deviceId == deviceId) {
                return;
            }
        }
    }

    auto& e = createEntity();

    this->setPlayer(&e);

    auto& module = e.addComponent<Module>(deviceId, &connection);
    auto& hardware = e.addComponent<Hardware>();

    hardware.lights.push_back({1, false});

    auto& transform = e.addComponent<Transform>();
    auto& velocity = e.addComponent<Velocity>();

    velocity.speed = 1.0f;

    auto& enemy = e.addComponent<Enemy>();
    auto& health = e.addComponent<Health>(100.0f, 15.0f);

    auto& player = e.addComponent<Player>(10.0f);
    auto& sword = e.addComponent<Sword>(5.0f);

    auto& tag = e.addComponent<Tag>("skeleton_minion", true);
    auto& animations = e.addComponent<Animations>();
    animations.animations.insert({"damage", "Rig_Medium_General/Hit_A"});
    animations.animations.insert({"death", "Rig_Medium_General/Death_A"});
    animations.animations.insert({"attack", "Rig_Medium_General/Throw"});
    animations.animations.insert({"heal", "Rig_Medium_General/Use_Item"});
    animations.animations.insert({"idle", "Rig_Medium_General/Idle_A"});

    std::cout << "Added module to Room Engine" << std::endl;

    std::cout << "Creating input action" << std::endl;
    //on/off
    moduleManager.addInput(module.deviceId, 18, 1);
    moduleManager.addInput(module.deviceId, 4, 2);

    //movement (Diamond)
    moduleManager.addInput(module.deviceId, 26, 3);
    moduleManager.addInput(module.deviceId, 12, 4);
    moduleManager.addInput(module.deviceId, 27, 5);
    moduleManager.addInput(module.deviceId, 14, 6);

    moduleManager.addOutput(module.deviceId, 21, 1);
    moduleManager.addOutput(module.deviceId, 22, 2);

    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 3, MOVEUP);
    actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 3, MOVEUP);
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 4, MOVEDOWN);
    actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 4, MOVEDOWN);
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 5, MOVELEFT);
    actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 5, MOVELEFT);
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 6, MOVERIGHT);
    actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 6, MOVERIGHT);
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 1, ATTACK);
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 2, HEAL);
    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 2, HEAL);

    roomEventManager.addRoomEvent(
        "General",
        std::vector<ActionType>{GENERAL},
        std::vector<std::string>{},
        std::vector<OutputAction>{{ModuleMessage_set_led_tag, module.deviceId, 2}});

    std::cout << "Added Room Events" << std::endl;

    // roomEventManager.addRoomEvent(
    //     "Heal",
    //     std::vector<ActionType>{HEAL},
    //     std::vector<std::string>{"damage.calculate"},
    //     std::vector<OutputAction>{{ModuleMessage_set_led_tag, module.deviceId, 2}});

    roomEventManager.addRoomEvent("Move Up", {MOVEUP}, {"movement.up"}, {});
    roomEventManager.addRoomEvent("Move Down", {MOVEDOWN}, {"movement.down"}, {});
    roomEventManager.addRoomEvent("Move Left", {MOVELEFT}, {"movement.left"}, {});
    roomEventManager.addRoomEvent("Move Right", {MOVERIGHT}, {"movement.right"}, {});

    roomEventManager.addRoomEvent("Random Color", {COLOR}, {"color.random", "render.color"}, {});
    roomEventManager.addRoomEvent("Heal", {HEAL}, {"health.heal", "color.green", "render.color"}, {});
    roomEventManager.addRoomEvent("Attack", {ATTACK}, {
        "combat.attack.calculate",
        "combat.attack.apply",
        "color.red",
        "render.color"
    }, {}, 60.0f);

    this->getRenderService().SpawnEntity(tag.tag, {{}, 1.0f});
}

void World::removeModule(TcpConnection &connection, std::string deviceId) {
    for (auto& e : entities) {
        if (e->hasComponent<Module>()) {
            auto& module = e->getComponent<Module>();
            if (module.deviceId == deviceId) {
                e->destroy();
            }
        }
    }
}
