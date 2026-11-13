#include "SeigneurNocturne.hpp"

int SeigneurNocturne::id() const { return 9; }
std::string SeigneurNocturne::name() const { return "Seigneur Nocturne"; }
std::string SeigneurNocturne::skill() const { return "Éclipse prédatrice"; }
std::string SeigneurNocturne::skillMethodName() const { return "eclipsePredatrice"; }
int SeigneurNocturne::attackBonus() const { return 9; }
int SeigneurNocturne::eclipsePredatrice(int level) const { return 36 + level * 5; }
int SeigneurNocturne::skillDamage(int level) const { return eclipsePredatrice(level); }
int SeigneurNocturne::skillHeal(int maxHp) const { return maxHp / 3; }
