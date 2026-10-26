//
// Created by Weckest on 2026-10-14.
//

#ifndef ROOMENGINE_SYSTEMMANAGER_H
#define ROOMENGINE_SYSTEMMANAGER_H
#include <functional>
#include <string>

#include "data/Operation.h"


class World;
class ActionContext;

class SystemManager {
    std::unordered_map<std::string, Operation> operations;

public:
    bool addOperation(std::string name,
        std::function<int(ActionContext&, World&)> operation,
        std::vector<std::string> requirements = {},
        std::vector<std::string> outputs = {}
    );

    void removeOperation(std::string name);

    Operation* getOperation(std::string name);

    std::vector<std::string> getOperationsThatOutput(std::string requirement);
};


#endif //ROOMENGINE_SYSTEMMANAGER_H