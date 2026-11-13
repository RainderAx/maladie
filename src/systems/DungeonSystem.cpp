#include "../../include/systems/DungeonSystem.hpp"
#include "../../include/entities/EnemyFactory.hpp"
#include "../../include/systems/CombatSystem.hpp"
#include "../../include/core/Terminal.hpp"
#include <iostream>

bool runDungeon(std::vector<Player>& team, int id, Progress& progress){
    const std::string names[]={"","Cathédrale des Cendres","Jardins Engloutis","Faille du Roi","Sanctuaire inversé"};
    if(id<1 || id>4) return false;
    Terminal::title("DONJON : " + names[id]);
    std::cout << "Deux salles vous séparent du gardien.\n";
    for(int room=1; room<=2; ++room){
        std::cout << "Salle " << room << "/2 — une présence se manifeste.\n";
        if(!combat(team, makeEnemy(id))) return false;
    }
    if(!combat(team, makeEnemy(id,true))) return false;
    if(id==3){
        std::cout << Terminal::red << "La Reine aux Milles-Visages descend du trône.\n" << Terminal::reset;
        if(!combat(team, makeEnemy(5,true))) return false;
    }
    if(id==1) progress.dungeon1=true;
    if(id==2) progress.dungeon2=true;
    if(id==3) progress.dungeon3=true;
    if(id==4) progress.dungeon4=true;
    std::cout << Terminal::yellow << "Donjon purifié.\n" << Terminal::reset;
    return true;
}
