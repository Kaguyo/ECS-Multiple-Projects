#pragma once
#include "ECS.h"
#include <iostream>

namespace Root {
    class MovementSystem {
    public:
        void update(ECS& ecs, float deltaTime) {
            for (auto& [entity, pos] : ecs.positions) {
                if (ecs.velocities.find(entity) != ecs.velocities.end()) {
                    auto& vel = ecs.velocities[entity];

                    pos.x += vel.vx * deltaTime;
                    pos.y += vel.vy * deltaTime;
                }
            }
        }
    };

    class HealthSystem {
    public:
        void update(ECS& ecs) {
            for (auto& [entity, health] : ecs.healths) {
                if (health.hp <= 0) {
                    std::cout << "Entity " << entity << " is dead\n";
                }
            }
        }
    };
}
