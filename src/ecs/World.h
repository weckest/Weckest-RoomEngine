//
// Created by Weckest on 2026-09-28.
//

#ifndef ROOMENGINE_WORLD_H
#define ROOMENGINE_WORLD_H
#include <iterator>
#include <memory>
#include <vector>

#include "Entity.h"
#include "event/EventManager.h"
#include "managers/ActionManager.h"
#include "managers/ModuleManager.h"
#include "managers/RoomEventManager.h"
#include "managers/SystemManager.h"
#include "network/TcpServer.h"
#include "services/visuals/RenderServiceClient.h"
#include "systems/ColorSystem.h"
#include "systems/CombatSystem.h"
#include "systems/HealthSystem.h"
#include "systems/LightSystem.h"
#include "systems/MovementSystem.h"
#include "systems/RenderSystem.h"
#include "systems/RoomEventCooldownSystem.h"
#include "systems/RoomEventSystem.h"
#include "systems/WaitSystem.h"


class World {
    TcpServer& tcpServer;
    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Entity>> deferredEntities;

    Entity* player;
    Entity* enemy;

    RenderServiceClient* renderService;

    EventManager eventManager;
    ActionManager actionManager{*this};
    RoomEventManager roomEventManager{*this};
    ModuleManager moduleManager;
    SystemManager systemManager;

    LightSystem lightSystem;
    RoomEventSystem roomEventSystem{*this};
    MovementSystem movementSystem{*this};
    ColorSystem colorSystem{*this};
    RenderSystem renderSystem{*this};
    CombatSystem combatSystem{*this};
    HealthSystem healthSystem{*this};
    WaitSystem waitSystem{*this};
    RoomEventCooldownSystem roomEventCooldownSystem;

public:
    World(TcpServer& tcpServer);

    void Update(float dt) {
        lightSystem.update(entities);
        roomEventSystem.update(*this, dt);
        roomEventCooldownSystem.update(*this);

        movementSystem.update(*this, dt);
        healthSystem.update(*this);

        synchronizeEntities();
        cleanup();
    }

    void createModule(TcpConnection& connection, std::string deviceId);

    void removeModule(TcpConnection& connection, std::string deviceId);

    Entity& createEntity() {
        //we use emplace instead of push so we dont create a copy
        entities.emplace_back(std::make_unique<Entity>());
        return *entities.back();
    }

    Entity& createEntity(Entity& e) {
        entities.emplace_back(std::make_unique<Entity>(e));
        return *entities.back();
    }

    Entity& createDeferredEntity() {
        deferredEntities.emplace_back(std::make_unique<Entity>());
        return *deferredEntities.back();
    }

    Entity& createDeferredEntity(Entity e) {
        deferredEntities.emplace_back(std::make_unique<Entity>(e));
        return *deferredEntities.back();
    }

    static Entity createTempEntity() {
        return *std::make_unique<Entity>();
    }

    std::vector<std::unique_ptr<Entity>>& getEntities() {
        return entities;
    }

    void cleanup() {
        //use lambda predicate to remove all inactive entities
        std::erase_if(
            entities,
            [this](std::unique_ptr<Entity>& e) {
                if (!e->isActive()) {
                    return !e->isActive();
                }
                return !e->isActive();
            }
        );
    }

    void synchronizeEntities() {
        if (!deferredEntities.empty()) {
            //push back all deferred entities to the entities vector
            //using move so we dont create a copy
            std::move(
                deferredEntities.begin(),
                deferredEntities.end(),
                std::back_inserter(entities)
            );
            //clearing the creation buffer
            deferredEntities.clear();
        }
    }

    void setPlayer(Entity* player) {
        this->player = player;
    }

    Entity* getPlayer() {
        return player;
    }

    RenderServiceClient& getRenderService() {
        return *renderService;
    }

    EventManager& getEventManager() {
        return eventManager;
    }

    ActionManager& getActionManager() {
        return actionManager;
    }

    RoomEventManager& getRoomEventManager() {
        return roomEventManager;
    }

    ModuleManager& getModuleManager() {
        return moduleManager;
    }

    SystemManager& getSystemManager() {
        return systemManager;
    }
};


#endif //ROOMENGINE_WORLD_H