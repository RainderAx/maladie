#include "../../include/systems/ArenaSystem.hpp"
#include "../../include/entities/EnemyFactory.hpp"
#include "../../include/systems/CombatSystem.hpp"
#include "../../include/core/Terminal.hpp"
#include <algorithm>
#include <iostream>
#include <random>

void runArena(std::vector<Player>& team, Progress& progress){
    Terminal::title("ARÈNE INFINIE");
    std::cout << "Meilleur score : vague " << progress.arenaBestWave << ".\n";
    int wave=1;
    std::mt19937 rng{std::random_device{}()};
    while(true){
        auto roster=arenaRoster(wave);
        Enemy enemy=roster[std::uniform_int_distribution<int>(0, static_cast<int>(roster.size())-1)(rng)];
        std::cout << Terminal::yellow << "VAGUE " << wave << " — " << enemy.name << " [" << enemy.className << "]" << Terminal::reset << "\n";
        if(!combat(team, enemy)) break;
        for(auto& hero:team){ hero.gold += wave*25; hero.gainXp(wave*30); if(hero.alive()) hero.hp=std::min(hero.maxHp,hero.hp+hero.maxHp/5); }
        progress.arenaBestWave=std::max(progress.arenaBestWave,wave);
        std::cout << "Récompense d’équipe : " << wave*25 << " or par héros.\n1. Vague suivante  2. Quitter\n> ";
        if(Terminal::ask(1,2)==2) break;
        ++wave;
    }
    std::cout << "Meilleure vague : " << progress.arenaBestWave << ".\n";
}
