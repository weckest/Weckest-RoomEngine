//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_BASEEVENT_H
#define ROOMENGINE_BASEEVENT_H
#include "data/Actions.h"

class Entity;

enum class EventType {
    InputAction,
    Damage,
    Heal,
    SpawnEntity,
};

struct BaseEvent {
    EventType type;
    virtual ~BaseEvent() = default;
};

struct ActionEvent : BaseEvent {
    Action* action;
    ActionEvent(Action* action) : action(action) {
        type = EventType::InputAction;
    }
};

#endif //ROOMENGINE_BASEEVENT_H