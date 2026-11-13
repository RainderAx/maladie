#pragma once
#include <memory>
#include <string>

class CharacterClass {
public:
    virtual ~CharacterClass();

    virtual int id() const = 0;
    virtual std::string name() const = 0;
    virtual std::string skill() const = 0;
    virtual std::string skillMethodName() const = 0;
    virtual int attackBonus() const;
    virtual int luckBonus() const;
    virtual int skillDamage(int level) const;
    virtual int skillHeal(int maxHp) const;
};
