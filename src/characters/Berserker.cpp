#include "Berserker.hpp"

int Berserker::id() const { return 2; }
std::string Berserker::name() const { return "Berserker"; }
std::string Berserker::skill() const { return "Rage des braises"; }
std::string Berserker::skillMethodName() const { return "rageDesBraises"; }
int Berserker::attackBonus() const { return 5; }
int Berserker::rageDesBraises(int level) const { return 24 + level * 4; }
int Berserker::skillDamage(int level) const { return rageDesBraises(level); }
