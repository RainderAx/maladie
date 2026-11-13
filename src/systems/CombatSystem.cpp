#include "../../include/systems/CombatSystem.hpp"
#include "../../include/systems/CombatRules.hpp"
#include "../../include/core/Terminal.hpp"
#include <algorithm>
#include <random>

static std::mt19937 rng{std::random_device{}()};
static int roll(int min, int max){ return std::uniform_int_distribution<int>(min, max)(rng); }

static bool teamAlive(const std::vector<Player>& team){
    for(const auto& hero : team) if(hero.alive()) return true;
    return false;
}

void updateQuestProgress(std::vector<Player>& team, const std::string& objective){
    if(team.empty()) return;
    for(auto& quest : team.front().quests){
        if(!quest.completed && quest.objective == objective){
            ++quest.progress;
            if(quest.progress >= quest.goal){
                quest.progress = quest.goal;
                quest.completed = true;
                std::cout << Terminal::yellow << "Quête accomplie : " << quest.name << Terminal::reset << "\n";
            }
        }
    }
}

static int chooseHero(const std::vector<Player>& team){
    while(true){
        std::cout << "Choisissez le héros actif (1-3) : ";
        int choice = Terminal::ask(1, 3);
        if(team[choice - 1].alive()) return choice - 1;
        std::cout << "Ce héros est K.O. Choisissez un héros vivant.\n";
    }
}

bool combat(std::vector<Player>& team, Enemy enemy){
    const double hpMultiplier = enemy.boss ? 1.30 : 1.15;
    enemy.maxHp = static_cast<int>(enemy.maxHp * hpMultiplier);
    enemy.hp = enemy.maxHp;
    Terminal::title(enemy.boss ? "BOSS — " + enemy.name : "RENCONTRE — " + enemy.name);
    while(teamAlive(team) && enemy.hp > 0){
        std::cout << "\n" << Terminal::red << enemy.name << Terminal::reset
                  << " [" << enemy.className << "] PV " << enemy.hp << "/" << enemy.maxHp
                  << " | ATK " << enemy.attack << " | DEF " << enemy.defense
                  << " | Chance " << enemy.chance << "% | Élément " << elementName(enemy.element) << "\n";
        for(size_t i=0;i<team.size();++i){
            const auto& hero=team[i];
            std::cout << i+1 << ". " << hero.name << " — " << hero.className << " — "
                      << hero.hp << "/" << hero.maxHp << " PV — " << elementName(hero.element)
                      << (hero.alive()?"":" [K.O.]") << "\n";
        }
        int active = chooseHero(team);
        Player& hero = team[active];
        std::cout << "1. Frappe  2. Technique  3. Potion  4. Fuir\n> ";
        int choice = Terminal::ask(1,4);
        if(choice == 4 && !enemy.boss){
            std::cout << "L’équipe quitte la zone de combat.\n";
            return false;
        }
        if(choice == 3){
            auto potion = std::find_if(hero.inventory.begin(), hero.inventory.end(), [](const Item& item){return item.heal > 0;});
            if(potion == hero.inventory.end()){
                std::cout << "Ce héros n’a aucune potion.\n";
                continue;
            }
            hero.hp = std::min(hero.maxHp, hero.hp + potion->heal);
            std::cout << hero.name << " récupère " << potion->heal << " PV.\n";
            hero.inventory.erase(potion);
        } else {
            const int variation = choice == 2 ? roll(0,4) : roll(-3,6);
            const bool critical = roll(1,100) <= hero.criticalChance();
            int damage = CombatRules::playerDamage(hero, enemy, choice == 2, variation, critical);
            if(choice == 2){
                std::cout << Terminal::cyan << hero.name << " utilise " << hero.characterClass->skill() << " !" << Terminal::reset << "\n";
                std::cout << "Méthode C++ appelée : " << hero.characterClass->skillMethodName() << "()\n";
                int heal = hero.characterClass->skillHeal(hero.maxHp);
                if(heal){ hero.hp = std::min(hero.maxHp, hero.hp + heal); std::cout << "Soin : +" << heal << " PV.\n"; }
            }
            double multiplier = elementMultiplier(hero.element, enemy.element);
            if(multiplier > 1.0) std::cout << Terminal::green << "Avantage élémentaire !" << Terminal::reset << "\n";
            if(multiplier < 1.0) std::cout << Terminal::yellow << "Résistance élémentaire ennemie." << Terminal::reset << "\n";
            if(critical) std::cout << Terminal::yellow << "Coup critique !" << Terminal::reset << "\n";
            enemy.hp -= damage;
            std::cout << hero.name << " inflige " << damage << " dégâts.\n";
            updateQuestProgress(team, "vaincre");
        }
        if(enemy.hp <= 0) break;
        std::vector<int> living;
        for(size_t i=0;i<team.size();++i) if(team[i].alive()) living.push_back(static_cast<int>(i));
        int targetIndex = living[roll(0, static_cast<int>(living.size())-1)];
        Player& target = team[targetIndex];
        const int variation = roll(-3,5);
        const bool critical = roll(1,100) <= enemy.chance;
        int taken = CombatRules::enemyDamage(enemy, target, variation, critical);
        if(critical) std::cout << Terminal::red << "Coup critique ennemi !" << Terminal::reset << "\n";
        target.hp -= taken;
        std::cout << enemy.name << " attaque " << target.name << " et inflige " << taken << " dégâts.\n";
    }
    if(!teamAlive(team)){
        std::cout << Terminal::red << "Toute l’équipe est K.O. La Faille vous submerge.\n" << Terminal::reset;
        return false;
    }
    std::cout << Terminal::green << "Victoire ! " << enemy.name << " est vaincu." << Terminal::reset << "\n";
    for(auto& hero : team){ hero.gold += enemy.gold; hero.gainXp(enemy.xp); }
    updateQuestProgress(team, "vaincre");
    if(enemy.boss) updateQuestProgress(team, "boss");
    return true;
}
