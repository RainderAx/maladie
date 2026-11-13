#include "PileFace.hpp"

int PileFace::id() const { return 10; }
std::string PileFace::name() const { return "Pile Face"; }
std::string PileFace::skill() const { return "Destin hasardeux"; }
std::string PileFace::skillMethodName() const { return "destinHasardeux"; }
int PileFace::attackBonus() const { return 1; }
int PileFace::destinHasardeux(int level) const { return 12 + level * 3; }
int PileFace::skillDamage(int level) const { return destinHasardeux(level); }
int PileFace::luckBonus() const { return 35; }
