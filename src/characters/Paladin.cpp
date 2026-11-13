#include "Paladin.hpp"

int Paladin::id() const { return 3; }
std::string Paladin::name() const { return "Paladin"; }
std::string Paladin::skill() const { return "Égide radieuse"; }
std::string Paladin::skillMethodName() const { return "egideRadieuse"; }
int Paladin::attackBonus() const { return 7; }
int Paladin::egideRadieuse(int level) const { return 28 + level * 4; }
int Paladin::skillDamage(int level) const { return egideRadieuse(level); }
int Paladin::skillHeal(int maxHp) const { return maxHp / 5; }
