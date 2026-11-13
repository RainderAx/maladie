#ifndef priest_HPP
#define priest_HPP

#include "Personnage.hpp"

class Priest : public Personnage {
public:
    Priest(std::string n, int pv, int atk,int chance, Element elem);
    void attaquer(Personnage &cible) override;
    int crit(int degats, int chance);

};

#endif
