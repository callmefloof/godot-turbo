#pragma once

#include "../thirdparty/flecs/distr/flecs.h"
#include "scene/main/node.h"
#include "servers/rendering_server.h"

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


