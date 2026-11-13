#include "vampire.hpp"

Vampire::Vampire(std::string n, int pv, int atk, Element elem)
    : Personnage(n, pv, atk, elem) {}

void Vampire::attaquer(Personnage &cible) {
    float modificateur = 1.0f;
    if (faiblesse[getElement()] == cible.getElement()) {
        modificateur = 2.0f; // Bonus contre l'élément faible
    }
    int degats = attaque * modificateur;
    cible.setPointsDeVie(cible.getPointsDeVie() - degats);
    pointsDeVie += degats / 3; 
    std::cout << nom << " attaque " << cible.getNom() << " et inflige " << degats << " dégâts !\n";
}
