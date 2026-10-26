//
// Created by Weckest on 2026-10-20.
//

#ifndef ROOMENGINE_OPERATION_H
#define ROOMENGINE_OPERATION_H
#include <functional>
#include <string>

#include "ActionContext.h"

class World;

struct Operation {
    std::string name;
    std::function<int(ActionContext&, World&)> function;
    std::vector<std::string> requirements;
    std::vector<std::string> outputs;
};

#endif //ROOMENGINE_OPERATION_H