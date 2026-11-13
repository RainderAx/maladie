#include "../../include/characters/CharacterClasses.hpp"
std::shared_ptr<CharacterClass> makeCharacterClass(int id){
    switch(id){
        case 1:return std::make_shared<Guerrier>(); case 2:return std::make_shared<Berserker>(); case 3:return std::make_shared<Paladin>();
        case 4:return std::make_shared<Priest>(); case 5:return std::make_shared<Oracle>(); case 6:return std::make_shared<Templier>();
        case 7:return std::make_shared<Vampire>(); case 8:return std::make_shared<Sanguinaire>(); case 9:return std::make_shared<SeigneurNocturne>();
        case 10:return std::make_shared<PileFace>(); case 11:return std::make_shared<BossDeFin>(); default:return std::make_shared<Guerrier>();
    }
}
