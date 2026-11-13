#pragma once

#include "../core/Player.hpp"
#include "../core/Types.hpp"

namespace CombatRules {
int playerDamage(const Player& hero, const Enemy& enemy, bool useSkill, int variation, bool critical);
int enemyDamage(const Enemy& enemy, const Player& hero, int variation, bool critical);
}
