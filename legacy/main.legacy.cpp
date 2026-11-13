#include <iostream>
#include "Personnage.hpp"
#include "guerrier.hpp"
#include "priest.hpp"
#include "vampire.hpp"
#include <ctime>
// Fonction pour choisir un élément
Element choisirElement() {
    int choix;
    std::cout << "Choisissez un élément :\n1. Eau\n2. Feu\n3. Plante\n";
    std::cin >> choix;

    switch (choix) {
        case 1: return Element::Eau;
        case 2: return Element::Feu;
        case 3: return Element::Plante;
        default: return Element::Eau; // Par défaut
    }
}

// Fonction pour choisir un nom
std::string choisirNom() {
    std::string nom;
    std::cout << "Entrez le nom du personnage : ";
    std::cin >> nom;
    return nom;
}

// Fonction pour choisir une classe et créer un personnage
Personnage* choisirClasse() {
    int choixClasse;
    Element elemJoueur = choisirElement();
    std::string nomJoueur = choisirNom();
    
    std::cout << "Choisissez une classe :\n1. Guerrier\n2. Prêtre\n3. Vampire\n";
    std::cin >> choixClasse;

    switch (choixClasse) {
        case 1: return new Guerrier(nomJoueur, 100, 20, elemJoueur);
        case 2: return new Priest(nomJoueur, 100, 20, elemJoueur);
        case 3: return new Vampire(nomJoueur, 100, 20, elemJoueur);
        default: 
            std::cout << "Choix invalide, Guerrier sélectionné par défaut.\n";
            return new Guerrier(nomJoueur, 100, 20, elemJoueur);
    }
}

int main() {
    // Initialisation de la graine pour les nombres aléatoires
    std::srand(static_cast<unsigned int>(std::time(0)));
    // Création du joueur
    std::cout << "Création du joueur :\n";
    Personnage* joueur = choisirClasse();

    // Création d'un ennemi
    Vampire ennemi("Monstre", 80, 15, Element::Feu);
    std::cout << "Création de l'ennemi :\n";

    // Affichage des statistiques
    std::cout << "\nStatistiques du joueur :\n";
    joueur->afficherStats();
    
    std::cout << "\nStatistiques de l'ennemi :\n";
    ennemi.afficherStats();

    // Simulation d'un combat (à améliorer plus tard)
    std::cout << "\nDébut du combat !\n";


    ennemi.attaquer(*joueur);   
    joueur->afficherStats();

    joueur->attaquer(ennemi);
    ennemi.afficherStats();
    joueur->afficherStats();

    // Libération de la mémoire allouée dynamiquement
    delete joueur;

    return 0;
}
