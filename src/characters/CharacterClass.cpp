#include "CharacterClass.hpp"

CharacterClass::~CharacterClass() = default;

int CharacterClass::attackBonus() const { return 0; }
int CharacterClass::luckBonus() const { return 0; }
int CharacterClass::skillDamage(int level) const { return 8 + level * 2; }
int CharacterClass::skillHeal(int) const { return 0; }
