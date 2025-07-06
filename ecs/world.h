#pragma once

#include "../thirdparty/flecs/flecs.h"
#include "scene/main/node.h"

class World : public Node
{
GDCLASS(World,Node)
private:
    flecs::world world;
    /* data */
protected:
    static void _bind_methods();
public:
    World(/* args */);
    ~World();
    void _ready();
    void _process(const double delta);

};


