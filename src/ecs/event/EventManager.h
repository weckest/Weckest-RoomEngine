//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_EVENTMANAGER_H
#define ROOMENGINE_EVENTMANAGER_H
#include <functional>

#include "BaseEvent.h"

class EventManager {
public:
    using Handler = std::function<void(const BaseEvent&)>;

    void emit(const BaseEvent& event) const {
        for (const auto& listener: listeners) {
            listener(event);
        }
    }

    void subscribe(const Handler& callback) {
        listeners.emplace_back(callback);
    }

private:
    std::vector<Handler> listeners;
};

#endif //ROOMENGINE_EVENTMANAGER_H