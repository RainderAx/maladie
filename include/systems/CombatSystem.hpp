#pragma once
#include "../core/Player.hpp"
#include "../core/Types.hpp"
#include <vector>

bool combat(std::vector<Player>& team, Enemy enemy);
void updateQuestProgress(std::vector<Player>& team, const std::string& objective);
