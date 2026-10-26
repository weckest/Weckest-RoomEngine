//
// Created by Weckest on 2026-10-14.
//

#include "ModuleManager.h"

void ModuleManager::addInput(std::string deviceId, int pinId, int inputId) {
    modules[deviceId].inputs[pinId] = inputId;
}

void ModuleManager::addOutput(std::string deviceId, int pinId, int outputId) {
    modules[deviceId].outputs[pinId] = outputId;
}

ModuleConfig * ModuleManager::getModuleConfig(std::string deviceId) {
    const auto it = modules.find(deviceId);

    if (it == modules.end())
    {
        return nullptr;
    }

    return &it->second;
}
