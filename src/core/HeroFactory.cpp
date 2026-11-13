#include "../../include/core/HeroFactory.hpp"
#include "../../include/characters/CharacterFactory.hpp"

Player createHero(const std::string& name, int classId, Element element) {
    Player hero;
    hero.name = name.empty() ? "Héros" : name;
    hero.characterClass = makeCharacterClass(classId);
    hero.className = hero.characterClass->name();
    hero.element = element;

    if (classId == 1) {
        hero.maxHp = 125;
        hero.hp = 125;
        hero.baseDefense = 7;
    } else if (classId <= 3) {
        hero.maxHp = 115;
        hero.hp = 115;
        hero.baseDefense = 6;
    } else if (classId <= 6) {
        hero.maxHp = 105;
        hero.hp = 105;
        hero.baseDefense = 5;
    } else if (classId <= 9) {
        hero.maxHp = 110;
        hero.hp = 110;
        hero.baseDefense = 4;
    } else {
        hero.maxHp = 100;
        hero.hp = 100;
        hero.baseDefense = 3;
        hero.chance = 35;
    }

    hero.weapon = {"Lame de " + elementName(element), "Une arme accordée à l’élément du héros.",
                   2, 0, 0, 0, true, element};
    hero.armor = {"Manteau de " + elementName(element), "Une armure accordée à l’élément du héros.",
                  0, 2, 0, 0, true, element};
    hero.inventory = {
        {"Potion solaire", "+45 PV", 0, 0, 45, 25, false, Element::Eau},
        {"Potion solaire", "+45 PV", 0, 0, 45, 25, false, Element::Eau}
    };
    return hero;
}
