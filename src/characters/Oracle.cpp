#include "Oracle.hpp"

int Oracle::id() const { return 5; }
std::string Oracle::name() const { return "Oracle"; }
std::string Oracle::skill() const { return "Présage astral"; }
std::string Oracle::skillMethodName() const { return "presageAstral"; }
int Oracle::attackBonus() const { return 3; }
int Oracle::presageAstral(int level) const { return 22 + level * 3; }
int Oracle::skillDamage(int level) const { return presageAstral(level); }
int Oracle::skillHeal(int maxHp) const { return maxHp / 4; }
