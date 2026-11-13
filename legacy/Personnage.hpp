#ifndef PERSONNAGE_HPP
#define PERSONNAGE_HPP

#include <iostream>
#include <string>
#include <map>

// Définition des éléments
enum class Element { Eau, Feu, Plante };

// La variable faiblesse est définie dans faiblesses.cpp
extern std::map<Element, Element> faiblesse;

class Personnage {
protected:
    std::string nom;
    int pointsDeVie;
    int attaque;
    int luck;
    Element element;

public:
    Personnage(std::string n, int pv, int atk, int chance, Element elem);
    virtual void attaquer(Personnage &cible);
    virtual ~Personnage();
    void afficherStats();

    // Getters
    std::string getNom() const { return nom; }
    int getPointsDeVie() const { return pointsDeVie; }
    int getAttaque() const { return attaque; }
    int getLuck() const { return luck; }
    Element getElement() const { return element; }

    // Setters
    void setNom(const std::string &n) { nom = n; }
    void setPointsDeVie(int pv) { pointsDeVie = pv; }
    void setAttaque(int atk) { attaque = atk; }
    void setLuck(int chance) { luck = chance; }
    void setElement(Element elem) { element = elem; }
};

#include "guerrier.hpp"
#include "priest.hpp"
#include "vampire.hpp"

#endif