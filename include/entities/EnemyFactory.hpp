#pragma once
#include "../core/Types.hpp"
#include <vector>
Enemy makeEnemy(int zone, bool boss=false);
std::vector<Enemy> arenaRoster(int wave=1);
