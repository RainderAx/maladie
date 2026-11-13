#include "guerrier.hpp"

Guerrier::Guerrier(std::string n, int pv, int atk, Element elem)
    : Personnage(n, pv, atk, elem) {}

void Guerrier::attaquer(Personnage &cible) {
    float modificateur = 1.0f;
    float bonus = 1.0f;
    if (dynamic_cast<Priest*>(&cible)) {
        bonus = 2.0f; // Attaque +2x sur les Prêtres
    }
    if (faiblesse[getElement()] == cible.getElement()) {
        modificateur = 1.5f; // Bonus contre l'élément faible
    }
    int degats = attaque * modificateur * bonus + 5; // Bonus de 5 points de dégâts
    cible.setPointsDeVie(cible.getPointsDeVie() - degats);
    std::cout << getNom() << " attaque " << cible.getNom() << " et inflige " << degats << " dégâts !\n";
}
