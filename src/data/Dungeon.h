//
// Created by Weckest on 2026-11-02.
//

#ifndef ROOMENGINE_DUNGEON_H
#define ROOMENGINE_DUNGEON_H
#include <string>
#include <vector>

#include "RoomEvent.h"

//all the information about the dungeon the player is currently in
class Dungeon {
    enum DungeonType {
        Battle,
        Puzzle,
        Resource
    };

public:
    const std::string name;
    const std::string sceneName;
    const DungeonType type;
    const std::vector<Entity> entities;
    const std::vector<RoomEvent> events;
    //win condition?????
    //time
    //enemy died
    //multiple enemies died
    //player died (lose condition)


    const int difficulty = 1;
    const int levelRequirement = 1;

};

#endif //ROOMENGINE_DUNGEON_H