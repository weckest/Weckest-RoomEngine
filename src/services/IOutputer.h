//
// Created by Weckest on 2026-10-07.
//

#ifndef ROOMENGINE_IOUTPUTER_H
#define ROOMENGINE_IOUTPUTER_H
#include "data/Actions.h"

class IOutputer {
public:
    virtual ~IOutputer() = default;

    virtual void HandleOutputAction(OutputAction& outputAction) = 0;
};

#endif //ROOMENGINE_IOUTPUTER_H