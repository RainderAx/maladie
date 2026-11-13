#include "Templier.hpp"

int Templier::id() const { return 6; }
std::string Templier::name() const { return "Templier"; }
std::string Templier::skill() const { return "Sceau du jugement"; }
std::string Templier::skillMethodName() const { return "sceauDuJugement"; }
int Templier::attackBonus() const { return 6; }
int Templier::sceauDuJugement(int level) const { return 30 + level * 4; }
int Templier::skillDamage(int level) const { return sceauDuJugement(level); }
int Templier::skillHeal(int maxHp) const { return maxHp / 5; }
