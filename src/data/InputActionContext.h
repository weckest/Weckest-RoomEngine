//
// Created by Weckest on 2026-11-02.
//

#ifndef ROOMENGINE_INPUTACTIONCONTEXT_H
#define ROOMENGINE_INPUTACTIONCONTEXT_H
#include <string>
#include <vector>

struct InputAction;

class InputActionContext {
public:
    std::string name;
    std::vector<InputAction> actions;
};

#endif //ROOMENGINE_INPUTACTIONCONTEXT_H