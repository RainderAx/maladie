#pragma once
#include "../core/Player.hpp"
#include "SaveSystem.hpp"
#include <vector>
std::vector<Enemy> arenaRosterForWave(int wave);
void runArena(std::vector<Player>& team, Progress& progress);
