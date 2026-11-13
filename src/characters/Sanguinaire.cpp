#include "Sanguinaire.hpp"

int Sanguinaire::id() const { return 8; }
std::string Sanguinaire::name() const { return "Sanguinaire"; }
std::string Sanguinaire::skill() const { return "Marée écarlate"; }
std::string Sanguinaire::skillMethodName() const { return "mareeEcarlate"; }
int Sanguinaire::attackBonus() const { return 6; }
int Sanguinaire::mareeEcarlate(int level) const { return 27 + level * 4; }
int Sanguinaire::skillDamage(int level) const { return mareeEcarlate(level); }
int Sanguinaire::skillHeal(int maxHp) const { return maxHp / 4; }
