#pragma once
#include <string>
#include <vector>

enum class Element { Eau, Feu, Plante, Ombre };
std::string elementName(Element element);
Element elementFromId(int id);
int elementId(Element element);
// Renvoie 1.5 si l'attaque est avantageuse, 0.75 si elle est résistée, sinon 1.0.
double elementMultiplier(Element attackElement, Element defenseElement);

struct Item {
    std::string name, description;
    int attack=0, defense=0, heal=0, value=0;
    bool equipment=false;
    Element element=Element::Ombre;
};

struct Quest {
    std::string name, objective, reward;
    int goal=0, progress=0, xp=0, gold=0;
    bool completed=false, claimed=false;
};

struct Enemy {
    std::string name, className;
    int hp=0, maxHp=0, attack=0, defense=0, chance=0, xp=0, gold=0;
    Element element=Element::Ombre;
    bool boss=false;
};
