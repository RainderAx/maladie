#include "Vampire.hpp"

int Vampire::id() const { return 7; }
std::string Vampire::name() const { return "Vampire"; }
std::string Vampire::skill() const { return "Morsure sanguine"; }
std::string Vampire::skillMethodName() const { return "morsureSanguine"; }
int Vampire::attackBonus() const { return 3; }
int Vampire::morsureSanguine(int level) const { return 18 + level * 3; }
int Vampire::skillDamage(int level) const { return morsureSanguine(level); }
int Vampire::skillHeal(int maxHp) const { return maxHp / 6; }
