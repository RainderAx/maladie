#ifndef VAMPIRE_HPP
#define VAMPIRE_HPP

#include "Personnage.hpp"

class Vampire : public Personnage {
public:
    Vampire(std::string n, int pv, int atk,int chance, Element elem);
    void attaquer(Personnage &cible) override;
};

#endif
