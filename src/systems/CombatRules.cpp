#include "../../include/systems/CombatRules.hpp"
#include <algorithm>

namespace CombatRules {
int playerDamage(const Player& hero, const Enemy& enemy, bool useSkill, int variation, bool critical) {
    const int rawDamage = useSkill
        ? hero.attack() + hero.characterClass->skillDamage(hero.level) + variation
        : hero.attack() + variation;
    int damage = std::max(1, static_cast<int>(rawDamage * elementMultiplier(hero.element, enemy.element)) - enemy.defense);
    if (critical) damage *= 2;
    return damage;
}

int enemyDamage(const Enemy& enemy, const Player& hero, int variation, bool critical) {
    int damage = std::max(1, static_cast<int>(enemy.attack * elementMultiplier(enemy.element, hero.element)) - hero.defense() + variation);
    if (critical) damage *= 2;
    return damage;
}
}
