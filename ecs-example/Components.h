#pragma once
#include <string>

struct NameComponent {
    std::string value;
};

struct HealthComponent {
    int maxHP;
    int currentHP;
};

struct EnergyComponent {
    int maxEnergy;
    int currentEnergy;
};

struct StatsComponent {
    int attack;
    int defense;
};
