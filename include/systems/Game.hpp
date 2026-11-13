#pragma once
#include "../core/Player.hpp"
#include "SaveSystem.hpp"
#include <vector>

class Game {
    std::vector<Player> team;
    Progress progress;
public:
    void run();
private:
    void createTeam();
    void configureHero(Player& hero, int index);
    void hub();
    void explore();
    void showStats() const;
    void inventory();
    void quests();
    void shop();
    void autoSave() const;
};
