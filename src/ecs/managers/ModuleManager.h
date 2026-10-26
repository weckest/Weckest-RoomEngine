//
// Created by Weckest on 2026-10-14.
//

#ifndef ROOMENGINE_MODULEMANAGER_H
#define ROOMENGINE_MODULEMANAGER_H
#include <string>
#include <unordered_map>

struct ModuleConfig {
    std::unordered_map<int, int> inputs{};
    std::unordered_map<int, int> outputs{};
};


class ModuleManager {
    std::unordered_map<std::string, ModuleConfig> modules{};

public:
    ModuleManager() = default;
    ~ModuleManager() = default;

    void addInput(std::string deviceId, int pinId, int inputId);
    void addOutput(std::string deviceId, int pinId, int outputId);

    ModuleConfig* getModuleConfig(std::string deviceId);
};


#endif //ROOMENGINE_MODULEMANAGER_H