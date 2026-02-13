#include <iostream>
#include <chrono>
#include <thread>

#include "ECS.h"
#include "Components.h"
#include "Systems.h"

using namespace Root;

int main() {
    ECS ecs;

    MovementSystem movementSystem;
    HealthSystem healthSystem;

    Entity player = ecs.createEntity();

    ecs.positions[player] = {0.0f, 0.0f};
    ecs.velocities[player] = {100.0f, 50.0f}; // unidades por segundo
    ecs.healths[player] = {100};

    const float targetFPS = 60.0f;
    const float targetFrameTime = 1.0f / targetFPS;

    using clock = std::chrono::high_resolution_clock;

    auto previousTime = clock::now();

    for (;;) {
        auto currentTime = clock::now();
        std::chrono::duration<float> elapsed = currentTime - previousTime;
        previousTime = currentTime;

        float deltaTime = elapsed.count();

        movementSystem.update(ecs, deltaTime);
        healthSystem.update(ecs);

        std::cout << "Player position: "
                  << ecs.positions[player].x << ", "
                  << ecs.positions[player].y << "\n";

        auto frameEndTime = clock::now();
        std::chrono::duration<float> frameDuration = frameEndTime - currentTime;

        float timeToSleep = targetFrameTime - frameDuration.count();

        if (timeToSleep > 0) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(timeToSleep)
            );
        }
    }

    return 0;
}
