#pragma once
#include "Types.hpp"
#include "../characters/CharacterFactory.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

class Player {
public:
    std::string name, className;
    std::shared_ptr<CharacterClass> characterClass = makeCharacterClass(1);
    Element element = Element::Ombre;
    int level=1, xp=0, gold=80, hp=100, maxHp=100, baseAttack=12, baseDefense=4, chance=10;
    std::vector<Item> inventory;
    Item weapon{"Lame rouillée","Une arme de fortune.",2,0,0,0,true,Element::Ombre};
    Item armor{"Manteau de voyageur","Une protection légère.",0,2,0,0,true,Element::Ombre};
    std::vector<Quest> quests;

    int matchingEquipmentBonus() const {
        int bonus = 0;
        if (weapon.equipment && weapon.element == element) bonus += 4;
        if (armor.equipment && armor.element == element) bonus += 4;
        return bonus;
    }
    int attack() const { return baseAttack + weapon.attack + characterClass->attackBonus() + matchingEquipmentBonus(); }
    int defense() const { return baseDefense + armor.defense + matchingEquipmentBonus(); }
    int criticalChance() const { return std::min(60, chance + characterClass->luckBonus() + (weapon.equipment && weapon.element == element ? 5 : 0)); }
    int xpNext() const { return 100 + (level - 1) * 75; }
    bool alive() const { return hp > 0; }
    void gainXp(int amount) { if (!alive() || amount <= 0) return; xp += amount; while (xp >= xpNext()) { xp -= xpNext(); ++level; maxHp += 18; baseAttack += 4; baseDefense += 2; ++chance; hp = maxHp; } }
    void addItem(const Item& item) { inventory.push_back(item); }
    bool equipItemFromInventory(std::size_t index) {
        if (index >= inventory.size()) return false;
        const Item incoming = inventory[index];
        if (!incoming.equipment || (incoming.attack <= 0 && incoming.defense <= 0)) return false;
        Item& slot = incoming.attack > 0 ? weapon : armor;
        const Item replaced = slot;
        slot = incoming;
        inventory.erase(inventory.begin() + static_cast<std::ptrdiff_t>(index));
        if (replaced.equipment && !replaced.name.empty()) inventory.push_back(replaced);
        return true;
    }
};
