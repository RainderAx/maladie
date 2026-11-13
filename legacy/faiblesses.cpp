#include "Personnage.hpp"

// Initialisation des faiblesses entre les éléments
std::map<Element, Element> faiblesse = {
    {Element::Eau, Element::Feu},    // L'eau bat le feu
    {Element::Feu, Element::Plante}, // Le feu bat la plante
    {Element::Plante, Element::Eau}  // La plante bat l'eau
};