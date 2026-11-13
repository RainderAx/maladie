#include "Personnage.hpp"
#include <cstdlib> // Pour rand()
#include <ctime>   // Pour time()

std::map<Element, Element> faiblesse = {
    {Element::Eau, Element::Feu},
    {Element::Feu, Element::Plante},
    {Element::Plante, Element::Eau}
};

// Constructeur
Personnage::Personnage(std::string n, int pv, int atk, int chance, Element elem)
    : nom(n), pointsDeVie(pv), attaque(atk), luck(chance), element(elem) {
        luck = (rand() % 100 + 1)+10; // Chance de 10 à 100
    }

// Méthode d'attaque
void Personnage::attaquer(Personnage &cible) {
    float modificateur = 1.0f;
    if (faiblesse[element] == cible.element) {
        modificateur = 1.5f; // Bonus contre l'élément faible
    }
    int degats = attaque * modificateur;
    cible.pointsDeVie -= degats;
    std::cout << nom << " attaque " << cible.nom << " et inflige " << degats << " dégâts !\n";
}

// Afficher les infos
void Personnage::afficherStats() {
    std::cout << "Nom : " << nom << " | PV : " << pointsDeVie 
              << " | Attaque : " << attaque << " | Chance : " << luck << " | Élément : ";
    switch (element) {
        case Element::Eau: std::cout << "Eau"; break;
        case Element::Feu: std::cout << "Feu"; break;
        case Element::Plante: std::cout << "Plante"; break;
    }
    std::cout << "\n";
}

// Destructeur
Personnage::~Personnage() {}