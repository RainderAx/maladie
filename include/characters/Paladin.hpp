#pragma once
#include "Berserker.hpp"

class Paladin : public Berserker {
public:
    int id() const override;
    std::string name() const override;
    std::string skill() const override;
    std::string skillMethodName() const override;
    int attackBonus() const override;
    int skillDamage(int level) const override;

    // Méthode nommée de la compétence unique de cette classe.
    int egideRadieuse(int level) const;
    int skillHeal(int maxHp) const override;
};
