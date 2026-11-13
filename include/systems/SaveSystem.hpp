#pragma once
#include "../core/Player.hpp"
#include <string>
#include <vector>
struct Progress {
    bool dungeon1=false, dungeon2=false, dungeon3=false, dungeon4=false;
    int arenaBestWave=0;
};
namespace SaveSystem {
bool save(const std::vector<Player>& team, const Progress& progress, const std::string& file="sauvegarde/savegame.txt");
bool load(std::vector<Player>& team, Progress& progress, const std::string& file="sauvegarde/savegame.txt");
}
