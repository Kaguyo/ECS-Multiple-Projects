#pragma once
#include <unordered_map>
#include <cstdint>
#include "Components.h"

using Entity = std::uint32_t;


namespace Root {

    class ECS {

    private:
        Entity nextEntity = 0;

    public:
        Entity createEntity() {
            return nextEntity++;
        }

        // Armazenamento simples de componentes
        std::unordered_map<Entity, Position> positions;
        std::unordered_map<Entity, Velocity> velocities;
        std::unordered_map<Entity, Health> healths;
    };
}
