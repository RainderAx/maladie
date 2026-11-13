#include "priest.hpp"
#include <cstdlib> // Pour rand()
#include <ctime>   // Pour time()

Priest::Priest(std::string n, int pv, int atk, int chance, Element elem)
    : Personnage(n, pv, atk, chance, elem) {}

    void Priest::attaquer(Personnage &cible) {
        float modificateur = 1.0f;
        float bonus = 1.0f;
        if (dynamic_cast<Vampire*>(&cible)) {
            bonus = 2.0f; // Attaque +2x sur les vampires
        }
        if (faiblesse[getElement()] == cible.getElement()) {
            modificateur = 1.5f; // Bonus contre l'élément faible
        }
        int degats = attaque * modificateur * bonus;
        int damages = crit(degats, getLuck());
        cible.setPointsDeVie(cible.getPointsDeVie() - damages);
        std::cout << getNom() << " attaque " << cible.getNom() << " et inflige " << damages << " dégâts !\n";
    };

    int Priest::crit(int degats, int chance) {
        int critique = 210 - (rand() % 100 + chance);
    
        if (critique > 100) {
            return degats; // Coup normal
        } else if (critique > 45) {
            std::cout << "Coup critique !\n";
            return degats + 20; // Coup critique faible
        } else {
            std::cout << "Super coup critique !\n";
            return static_cast<int>(degats * 1.5) + 20; // Coup critique fort
        }
    }