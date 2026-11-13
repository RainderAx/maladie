#include "Priest.hpp"

int Priest::id() const { return 4; }
std::string Priest::name() const { return "Prêtre"; }
std::string Priest::skill() const { return "Lumière réparatrice"; }
std::string Priest::skillMethodName() const { return "lumiereReparatrice"; }
int Priest::attackBonus() const { return 1; }
int Priest::lumiereReparatrice(int level) const { return 12 + level * 2; }
int Priest::skillDamage(int level) const { return lumiereReparatrice(level); }
int Priest::skillHeal(int maxHp) const { return maxHp / 3; }
