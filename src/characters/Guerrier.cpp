#include "Guerrier.hpp"

int Guerrier::id() const { return 1; }
std::string Guerrier::name() const { return "Guerrier"; }
std::string Guerrier::skill() const { return "Frappe tellurique"; }
std::string Guerrier::skillMethodName() const { return "frappeTellurique"; }
int Guerrier::attackBonus() const { return 2; }
int Guerrier::frappeTellurique(int level) const { return 16 + level * 3; }
int Guerrier::skillDamage(int level) const { return frappeTellurique(level); }
