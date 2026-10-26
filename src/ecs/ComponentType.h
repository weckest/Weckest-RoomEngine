//
// Created by Weckest on 2026-09-28.
//

#ifndef ROOMENGINE_COMPONENTTYPE_H
#define ROOMENGINE_COMPONENTTYPE_H
#include <cstddef>

//creating a type alias for std::size_t
//size_t is designed to represent sizes and indices because its unsigned and its very large (long long)
using ComponentTypeID = std::size_t;


//global countere generator, returns a unique number every time we call it
//making id static, means it will keep its value through multiple calls of this function
inline ComponentTypeID getComponentTypeID() {
    static ComponentTypeID id = 0;
    return id++;
}

//can overload this function because they have different signatures
//template allows us to use generic types
//template is inline by default (don't have to mark it by default)
//getComponentTypeID<Position>() -> always return 0
//getComponentTypeID<Health>() -> always return 1
//a static local variable is created and initialized once
template <typename T>
ComponentTypeID getComponentTypeID() {
    static ComponentTypeID typeID = getComponentTypeID();
    return typeID;
}

#endif //ROOMENGINE_COMPONENTTYPE_H