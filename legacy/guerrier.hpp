#ifndef guerrier_hpp
#define guerrier_hpp

#include "Personnage.hpp"

class Guerrier : public Personnage {
public:
    Guerrier(std::string n, int pv, int atk,int chance, Element elem);
        void attaquer(Personnage &cible) override ;
};

#endif // guerrier_hpp