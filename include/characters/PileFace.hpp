#pragma once
#include "CharacterClass.hpp"

class PileFace : public CharacterClass {
public:
    int id() const override;
    std::string name() const override;
    std::string skill() const override;
    std::string skillMethodName() const override;
    int attackBonus() const override;
    int skillDamage(int level) const override;

    // Méthode nommée de la compétence unique de cette classe.
    int destinHasardeux(int level) const;
    int luckBonus() const override;
};
