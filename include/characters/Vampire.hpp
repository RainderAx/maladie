#pragma once
#include "CharacterClass.hpp"

class Vampire : public CharacterClass {
public:
    int id() const override;
    std::string name() const override;
    std::string skill() const override;
    std::string skillMethodName() const override;
    int attackBonus() const override;
    int skillDamage(int level) const override;

    // Méthode nommée de la compétence unique de cette classe.
    int morsureSanguine(int level) const;
    int skillHeal(int maxHp) const override;
};
