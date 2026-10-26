//
// Created by Weckest on 2026-09-28.
//

#ifndef ROOMENGINE_ENTITY_H
#define ROOMENGINE_ENTITY_H
#include <array>
#include <bitset>

#include "ComponentType.h"

//create a const that is evaluated at compile time
//32 components fits perfectly in a 32-bit integer
constexpr std::size_t MAX_COMPONENTS = 32;

//bitset[0] = 1 //entity has the component at 0
//bit operations are fast, and work well for checking each frame
using ComponentBitSet = std::bitset<MAX_COMPONENTS>;

//fixed size array, where each index is a component
//the index corresponds to the componentTypeID
//we use a fixed number of components, so there is no dynamic allocation for the array (efficient)
using ComponentArray = std::array<void*, MAX_COMPONENTS>;

class Entity {
private:
    bool active = true;
    ComponentArray componentArray{};
    ComponentBitSet componentBitSet{};
public:
    bool isActive() {
        return active;
    }
    void destroy() {
        active = false;
    }

    //templates need to be implemented in header
    //entity.hasComponent<Position>() -> returns true of false
    template <typename T>
    bool hasComponent() {
        return componentBitSet[getComponentTypeID<T>()];
    }

    //template can receive multiple arguments
    template <typename T, typename... mArgs>
    T& addComponent(mArgs&&...args) {
        //implements perfect forwarding
        //everything is not treated as an lvalue
        //this is more efficient with moves and temporaries
        T* component(new T{std::forward<mArgs>(args)...});
        //add the new component to the array
        componentArray[getComponentTypeID<T>()] = component;
        //update bitset to true
        componentBitSet[getComponentTypeID<T>()] = true;
        return *component;
    }

    template <typename T>
    T& getComponent() {
        auto ptr(componentArray[getComponentTypeID<T>()]);
        return *static_cast<T*>(ptr);
    }

    template <typename T>
    void deactivateComponent() {
        componentBitSet[getComponentTypeID<T>()] = false;
    }


};

#endif //ROOMENGINE_ENTITY_H