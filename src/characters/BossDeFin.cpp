#include "BossDeFin.hpp"

int BossDeFin::id() const { return 11; }
std::string BossDeFin::name() const { return "BossDeFin"; }
std::string BossDeFin::skill() const { return "Anéantissement de la Faille"; }
std::string BossDeFin::skillMethodName() const { return "aneantissementDeLaFaille"; }
int BossDeFin::attackBonus() const { return 18; }
int BossDeFin::aneantissementDeLaFaille(int level) const { return 55 + level * 7; }
int BossDeFin::skillDamage(int level) const { return aneantissementDeLaFaille(level); }
int BossDeFin::skillHeal(int maxHp) const { return maxHp / 2; }
