#include <iostream>
#include "ECS.h"
#include "Components.h"
#include "Systems.h"
#include "Events.hpp"

using namespace std;

int main() {
    ECS ecs;
    EventBus eventBus;

    // Criar player
    int player = ecs.createEntity();
    ecs.addComponent(player, NameComponent{"Player"});
    ecs.addComponent(player, HealthComponent{100, 100});
    ecs.addComponent(player, EnergyComponent{50, 50});
    ecs.addComponent(player, StatsComponent{15, 5});

    // Criar inimigo
    int enemy  = ecs.createEntity();
    ecs.addComponent(enemy, NameComponent{"CPU"});
    ecs.addComponent(enemy, HealthComponent{80, 80});
    ecs.addComponent(enemy, EnergyComponent{30, 30});
    ecs.addComponent(enemy, StatsComponent{10, 4});

    RenderSystem renderer;
    CombatSystem combat(eventBus);
    InputSystem input;

    // Registra handlers
    combat.registerHandlers(ecs);

    bool playerTurn = true;
    bool running = true;

    while (running) {
        // Render
        renderer.draw(ecs);

        if (playerTurn) {
            int choice = input.askChoice();

            switch (choice) {
                case 1: eventBus.send(Event{EvtType::Attack, player, enemy}); break;
                case 2: eventBus.send(Event{EvtType::Defend, player, player}); break;
                case 3: eventBus.send(Event{EvtType::Special, player, enemy}); break;
            }

            playerTurn = false;
        } else {
            eventBus.send(Event{EvtType::Attack, enemy, player});
            playerTurn = true;
        }

        // Condição de fim
        auto ph = ecs.getComponent<HealthComponent>(player);
        auto eh = ecs.getComponent<HealthComponent>(enemy);

        if (ph && eh && (ph->currentHP <= 0 || eh->currentHP <= 0)) {
            renderer.draw(ecs);
            cout << "\n=== FIM DO COMBATE ===\n";
            running = false;
        }
    }

    return 0;
}
