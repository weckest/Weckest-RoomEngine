//
// Created by Weckest on 2026-10-14.
//

#ifndef ROOMENGINE_ACTIONCONTEXT_H
#define ROOMENGINE_ACTIONCONTEXT_H
#include <any>
#include <optional>
#include <string>
#include <unordered_map>

namespace std {
    class any;
}

class Entity;

class ActionContext {
public:
    std::optional<Entity*> source;
    std::optional<Entity*> target;

    // Input values supplied when the action starts.
    template <typename T>
    void setParameter(const std::string& name, T value)
    {
        parameters.insert_or_assign(
            name,
            std::any(std::move(value))
        );
    }

    template <typename T>
    const T* getParameter(const std::string& name) const
    {
        auto it = parameters.find(name);

        if (it == parameters.end())
            return nullptr;

        return std::any_cast<T>(&it->second);
    }

    // Values produced by steps while the action runs.
    template <typename T>
    void setResult(const std::string& name, T value)
    {
        results.insert_or_assign(
            name,
            std::any(std::move(value))
        );
    }

    template <typename T>
    const T* getResult(const std::string& name) const
    {
        auto it = results.find(name);

        if (it == results.end())
            return nullptr;

        return std::any_cast<T>(&it->second);
    }

    [[nodiscard]] bool hasParameter(const std::string& name) const
    {
        auto it = parameters.find(name);
        if (it == parameters.end())
            return false;
        return true;
    }

private:
    std::unordered_map<std::string, std::any> parameters;
    std::unordered_map<std::string, std::any> results;
};

#endif //ROOMENGINE_ACTIONCONTEXT_H