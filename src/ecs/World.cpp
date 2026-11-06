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

    auto& transform = e.addComponent<Transform>();
    auto& velocity = e.addComponent<Velocity>();

    velocity.speed = 1.0f;

    auto& enemy = e.addComponent<Enemy>();
    auto& eHealth = e.addComponent<Health>(100.0f, 15.0f);

    auto& eTag = e.addComponent<Tag>("skeleton_minion", false);
    auto& animations = e.addComponent<Animations>();
    animations.animations.insert({"damage", "Rig_Medium_General/Hit_A"});
    animations.animations.insert({"damageB", "Rig_Medium_General/Hit_B"});
    animations.animations.insert({"death", "Rig_Medium_General/Death_A"});
    animations.animations.insert({"death_pose", "Rig_Medium_General/Death_A_Pose"});
    animations.animations.insert({"attack", "Rig_Medium_General/Throw"});
    animations.animations.insert({"heal", "Rig_Medium_General/Use_Item"});
    animations.animations.insert({"idle", "Rig_Medium_General/Idle_A"});


    //Player
    auto& p = createEntity();

    auto& module = p.addComponent<Module>(deviceId, &connection);
    auto& hardware = p.addComponent<Hardware>();
    hardware.lights.push_back({1, false});

    this->setPlayer(&p);

    auto& pHealth = p.addComponent<Health>(100.0f, 15.0f);

    auto& player = p.addComponent<Player>(10.0f);
    auto& sword = p.addComponent<Sword>(5.0f);

    auto& pTag = p.addComponent<Tag>("player1", false);
    pTag.rendered = false;


    std::cout << "Added module to Room Engine" << std::endl;

    std::cout << "Creating input action" << std::endl;
    //on/off
    //Top
    moduleManager.addInput(module.deviceId, 18, 1);
    //Bottom
    // moduleManager.addInput(module.deviceId, 4, 2);

    //movement (Diamond)
    //up
    moduleManager.addInput(module.deviceId, 26, 3);
    //Down
    moduleManager.addInput(module.deviceId, 12, 4);
    //Left
    moduleManager.addInput(module.deviceId, 27, 5);
    //Right
    moduleManager.addInput(module.deviceId, 14, 6);

    moduleManager.addOutput(module.deviceId, 21, 1);
    moduleManager.addOutput(module.deviceId, 22, 2);

    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 3, "Move.Up.Start");
    // actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 3, "Move.Up.Stop");
    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 4, "Move.Down.Start");
    // actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 4, "Move.Down.Stop");
    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 5, "Move.Left.Start");
    // actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 5, "Move.Left.Stop");
    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 6, "Move.Right.Start");
    // actionManager.addAction(InputMessage_button_released_tag, module.deviceId, 6, "Move.Right.Stop");

    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 3, "Switch.Attack");
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 4, "Switch.Heal");
    actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 5, "Block");
    // actionManager.addAction(InputMessage_button_pressed_tag, module.deviceId, 2, HEAL);

    actionManager.switchCurrentContext("base");


    //Context Switching
    roomEventManager.addRoomEvent("Base Context", {"Return.Base"}, {
        "context.switch"
    }, {}, 0.0f, {
        {"context", "base"}
    });

    roomEventManager.addRoomEvent("Combat Context", {"Switch.Attack"}, {
        "context.switch"
    }, {}, 0.0f, {
        {"context", "combat"}
    });

    roomEventManager.addRoomEvent("Heal Context", {"Switch.Heal"}, {
        "context.switch"
    }, {}, 20.0f, {
        {"context", "heal"}
    });

    //Base Room Events
    roomEventManager.addRoomEvent("Block", {"Block"}, {
        "block"
    }, {}, 0.0f);

    //Combat Room Events
    setupBattleAttackInputs(module);

    //Heal Room Events
    setupBattleHealInputs(module);

    //Menu Room Events
    setupMenuInputs(module);
    actionManager.switchCurrentContext("menu");

    //Entity Spawning Room Events
    roomEventManager.addRoomEvent("Spawn Enemy", {"Spawn.Enemy"}, {
        "spawn.entity"
    }, {}, 0.0f, {
            {"entity.tag", eTag.tag}
    });

    roomEventManager.addRoomEvent("Spawn Player", {"Spawn.Player"}, {
        "spawn.entity"
    }, {}, 0.0f, {
            {"entity.tag", pTag.tag}
    });

    roomEventManager.addRoomEvent("Spawn Win", {"NOTHING"}, {
        "spawn.entity"
    }, {}, 0.0f, {
            {"entity.tag", "Win"}
    });

    //Enemy Room Events
    roomEventManager.addRoomEvent("Enemy Attack", {}, {
        "enemy.ai.attack",
        "health.update"
    }, {}, 10.0f, {
        {"player.tag",pTag.tag}
    });

    //Scene Switch Room Events



    /* roomEventManager.addRoomEvent(
        "General",
        std::vector<std::string>{"General"},
        std::vector<std::string>{},
        std::vector<OutputAction>{{ModuleMessage_set_led_tag, module.deviceId, 2}}); */

    // roomEventManager.addRoomEvent(
    //     "Heal",
    //     std::vector<ActionType>{HEAL},
    //     std::vector<std::string>{"damage.calculate"},
    //     std::vector<OutputAction>{{ModuleMessage_set_led_tag, module.deviceId, 2}});

    // // Movement
    // roomEventManager.addRoomEvent("Move Up Start", {"Move.Up.Start"}, {"movement.up.start"}, {});
    // roomEventManager.addRoomEvent("Move Down Start", {"Move.Down.Start"}, {"movement.down.start"}, {});
    // roomEventManager.addRoomEvent("Move Left Start", {"Move.Left.Start"}, {"movement.left.start"}, {});
    // roomEventManager.addRoomEvent("Move Right Start", {"Move.Right.Start"}, {"movement.right.start"}, {});
    //
    // roomEventManager.addRoomEvent("Move Up Stop", {"Move.Up.Stop"}, {"movement.up.stop"}, {});
    // roomEventManager.addRoomEvent("Move Down Stop", {"Move.Down.Stop"}, {"movement.down.stop"}, {});
    // roomEventManager.addRoomEvent("Move Left Stop", {"Move.Left.Stop"}, {"movement.left.stop"}, {});
    // roomEventManager.addRoomEvent("Move Right Stop", {"Move.Right.Stop"}, {"movement.right.stop"}, {});

    // roomEventManager.addRoomEvent("Random Color", {"Color"}, {"color.random", "render.color"}, {});
    // roomEventManager.addRoomEvent("Heal", {"Heal"}, {"health.heal", "color.green", "render.color"}, {});





    // roomEventManager.addRoomEvent("Test Large Attack", {"Combat.Attack"}, {
    //     "combat.attack.large"
    // }, {"Combat.Return"});

    /* roomEventManager.addRoomEvent("Test Scene Switch", {"Test"}, {
        "scene.switch",
        "spawn.puzzle.piece",
        "spawn.puzzle.piece",
        "spawn.puzzle.piece",
        "spawn.puzzle.piece"
    }, {}, 0.0f,{
        {"sceneName", "Puzzle"},
        {"spawn", "cube"}
    }); */

    // roomEventManager.addRoomEvent("Attack", {"Attack"}, {
    //     "combat.attack.calculate",
    //     "combat.attack.apply",
    //     "color.red",
    //     "render.color"
    // }, {}, 60.0f);
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

void World::setupMenuInputs(Module &module) {

    actionManager.addActionContext({"menu", {
        {InputMessage_button_pressed_tag, module.deviceId, 3, "Menu.Attack"},
        {InputMessage_button_pressed_tag, module.deviceId, 4, "Menu.Heal"},
        {InputMessage_button_pressed_tag, module.deviceId, 5, "Menu.Block"},
        {InputMessage_button_pressed_tag, module.deviceId, 6, "Menu.Blank"},
        {InputMessage_button_pressed_tag, module.deviceId, 1, "Menu.Return"}
    }});

    actionManager.addActionContext({"menu-start", {
        {InputMessage_button_pressed_tag, module.deviceId, 3, "Menu.Attack"},
        {InputMessage_button_pressed_tag, module.deviceId, 4, "Menu.Heal"},
        {InputMessage_button_pressed_tag, module.deviceId, 5, "Menu.Block"},
        {InputMessage_button_pressed_tag, module.deviceId, 6, "Menu.Blank"},
        {InputMessage_button_pressed_tag, module.deviceId, 1, "Scene.Battle"}
    }});

    roomEventManager.addRoomEvent("Render Input Actions", {"NOTHING"}, {
    "render.actions"
    }, {}, 0.0f);

    roomEventManager.triggerRoomEvent("Render Input Actions");

    roomEventManager.addRoomEvent("Switch to Battle Scene", {"Scene.Battle"}, {
        "scene.switch"
    }, {"Spawn.Enemy", "Spawn.Player", "Return.Base"}, 0.0f, {
            {"sceneName", "Main"}
    });

    roomEventManager.addRoomEvent("Menu Attack", {"Menu.Attack"}, {
        "render.text"
    }, {}, 0.0f, {
            {"text", "This lets you select the attack you wish to perform. Either the top button for a light attack or the bottom button for a heavy attack."}
    });

    roomEventManager.addRoomEvent("Menu Heal", {"Menu.Heal"}, {
        "render.text"
    }, {}, 0.0f, {
            {"text", "This lets you select the player you which to heal using the 4 buttons in the diamond pattern."}
    });

    roomEventManager.addRoomEvent("Menu Block", {"Menu.Block"}, {
        "render.text"
    }, {}, 0.0f, {
            {"text", "This lets you block the next attack within 2 seconds (NOT IMPLEMENTED)"}
    });

    roomEventManager.addRoomEvent("Menu Blank", {"Menu.Blank"}, {
        "render.text"
    }, {}, 0.0f, {
            {"text", "This button does not do anything in the current context. but action selections may use this button. (Healing, Attacking)"}
    });

    roomEventManager.addRoomEvent("Menu Return", {"Menu.Return"}, {
        "render.text",
        "context.switch",
        "render.actions"
    }, {}, 0.0f, {
            {"text", "This lets you return to the main button layout from any action selection you might be doing. (Healing, Attacking) \n "
                     "THE NEXT TIME THIS BUTTON IS PRESSED THE GAME WILL START"},
        {"context", "menu-start"}
    });

}

void World::setupBattleAttackInputs(Module& module) {
    actionManager.addActionContext({"combat", {
        {InputMessage_button_pressed_tag, module.deviceId, 3, "Combat.Attack.Normal"},
        {InputMessage_button_pressed_tag, module.deviceId, 4, "Combat.Attack.Large"},
        {InputMessage_button_pressed_tag, module.deviceId, 1, "Return.Base"}
    }});

    roomEventManager.addRoomEvent("Attack Normal", {"Combat.Attack.Normal"}, {
        "combat.attack.calculate",
        "combat.attack.apply",
        "health.update"
    }, {"Return.Base"}, 5.0f, {
        {"player.tag", "skeleton_minion"}
    });

    roomEventManager.addRoomEvent("Attack Large", {"Combat.Attack.Large"}, {
        "combat.attack.calculate",
        "combat.attack.large",
        "health.update"
    }, {"Return.Base"}, 20.0f, {
        {"player.tag", "skeleton_minion"}
    });
}

void World::setupBattleHealInputs(Module &module) {
    actionManager.addActionContext({"heal", {
        {InputMessage_button_pressed_tag, module.deviceId, 3, "Heal.Player1"},
        {InputMessage_button_pressed_tag, module.deviceId, 4, "Heal.Player2"},
        {InputMessage_button_pressed_tag, module.deviceId, 5, "Heal.Player3"},
        {InputMessage_button_pressed_tag, module.deviceId, 6, "Heal.Player4"},
        {InputMessage_button_pressed_tag, module.deviceId, 1, "Return.Base"}
    }});

    roomEventManager.addRoomEvent("Heal Player 1", {"Heal.Player1"}, {
        "health.heal",
        "health.update"
    }, {"Return.Base"}, 0.0f, {
        {"player.tag", "player1"}
    });

    roomEventManager.addRoomEvent("Heal Player 2", {"Heal.Player2"}, {
        "health.heal",
        "health.update"
    }, {"Return.Base"}, 0.0f, {
        {"player.tag", "player1"}
    });

    roomEventManager.addRoomEvent("Heal Player 3", {"Heal.Player3"}, {
        "health.heal",
        "health.update"
    }, {"Return.Base"}, 0.0f, {
        {"player.tag", "player1"}
    });

    roomEventManager.addRoomEvent("Heal Player 4", {"Heal.Player4"}, {
        "health.heal",
        "health.update"
    }, {"Return.Base"}, 0.0f, {
        {"player.tag", "player1"}
    });
}
