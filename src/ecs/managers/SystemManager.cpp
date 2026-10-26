//
// Created by Weckest on 2026-10-14.
//

#include "SystemManager.h"

#include <iostream>

#include "data/Operation.h"

bool SystemManager::addOperation(std::string name,
                                 std::function<int(ActionContext &, World &)> operation,
                                 std::vector<std::string> requirements,
                                 std::vector<std::string> outputs
) {
    operations.insert({name, Operation{name, operation, requirements, outputs}});

    if (operations.find(name) == operations.end()) {
        return false;
    }
    return true;
}

void SystemManager::removeOperation(std::string name) {
    operations.erase(name);
}

Operation* SystemManager::getOperation(std::string name) {
    auto it = operations.find(name);
    if (it != operations.end()) {
        return &it->second;
    }

    std::cerr << "Operation \"" << name << "\" not found" << std::endl;

    // Create a safe, permanent "null" Operation to return
    return nullptr;
}

std::vector<std::string> SystemManager::getOperationsThatOutput(std::string requirement) {
    std::vector<std::string> outputs;
    for (std::pair<std::string, Operation> operation : operations) {
        for (std::string output : operation.second.outputs) {
            if (output == requirement) {
                outputs.push_back(operation.first);
                break;
            }
        }
    }
    return outputs;
}
