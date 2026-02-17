#pragma once
#include "ECS.h"
#include "Components.h"
#include "Events.hpp"
#include <iostream>

class RenderSystem {
public:
    void draw(ECS& ecs) {
        // limpa o terminal
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif

        for (auto id : ecs.getEntities()) {
            auto name = ecs.getComponent<NameComponent>(id);
            auto hp   = ecs.getComponent<HealthComponent>(id);
            auto en   = ecs.getComponent<EnergyComponent>(id);

            if (name && hp && en) {
                std::cout << "=== " << name->value << " ===\n";
                std::cout << "HP: " << hp->currentHP << " / " << hp->maxHP << "\n";
                std::cout << "EN: " << en->currentEnergy << " / " << en->maxEnergy << "\n\n";
            }
        }
    }
};

class CombatSystem {
public:
    CombatSystem(EventBus& bus) : eventBus(bus) {}

    void registerHandlers(ECS& ecs) {
        eventBus.subscribe(EvtType::Attack, [&](const Event& e) {
            auto atk = ecs.getComponent<StatsComponent>(e.sourceID);
            auto targetHp = ecs.getComponent<HealthComponent>(e.targetID);

            if (atk && targetHp) {
                targetHp->currentHP -= atk->attack;
            }
        });

        eventBus.subscribe(EvtType::Defend, [&](const Event& e) {
            auto en = ecs.getComponent<EnergyComponent>(e.sourceID);
            if (en) {
                en->currentEnergy += 5;
            }
        });

        eventBus.subscribe(EvtType::Special, [&](const Event& e) {
            auto targetHp = ecs.getComponent<HealthComponent>(e.targetID);
            if (targetHp) {
                targetHp->currentHP -= 20;
            }
        });
    }

private:
    EventBus& eventBus;
};

class InputSystem {
public:
    int askChoice() {

        int c;
        std::cout << "1) Atacar\n2) Defender\n3) Especial" << std::endl;
        std::cin >> c;
        system("cls");
        return c;
    }
};
