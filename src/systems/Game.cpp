#include "../../include/systems/Game.hpp"
#include "../../include/core/Terminal.hpp"
#include "../../include/core/HeroFactory.hpp"
#include "../../include/systems/DungeonSystem.hpp"
#include "../../include/systems/ArenaSystem.hpp"
#include <filesystem>
#include <iostream>
#include <string>
using namespace Terminal;

void Game::autoSave() const { SaveSystem::save(team, progress); }

void Game::configureHero(Player& hero, int index){
    std::cout << "\nNom du héros " << index << " : ";
    std::string heroName;
    std::getline(std::cin, heroName);
    if(heroName.empty()) heroName="Héros "+std::to_string(index);
    title("CLASSE DU HÉROS " + std::to_string(index));
    std::cout << "1 Guerrier\n2 Berserker\n3 Paladin\n4 Prêtre\n5 Oracle\n6 Templier\n7 Vampire\n8 Sanguinaire\n9 Seigneur Nocturne\n10 Pile Face\n> ";
    int classId=ask(1,10);
    title("ÉLÉMENT DE " + heroName);
    std::cout << "1 Eau\n2 Feu\n3 Plante\n4 Ombre\n> ";
    const Element element=elementFromId(ask(1,4)-1);
    hero=createHero(heroName,classId,element);
    std::cout << hero.name << " rejoint l’équipe avec l’élément " << elementName(hero.element) << ".\n";
}

void Game::createTeam(){
    team.clear();
    for(int index=1; index<=3; ++index){team.emplace_back();configureHero(team.back(),index);}
    team.front().quests.push_back({"Premier éclat","vaincre","Vaincre votre premier adversaire",1,0,60,30});
    for(size_t index=1;index<team.size();++index) team[index].quests=team.front().quests;
    autoSave();
}

void Game::showStats() const {
    title("FICHE DE L’ÉQUIPE");
    for(size_t index=0;index<team.size();++index){
        const auto& hero=team[index];
        std::cout << index+1 << ". " << yellow << hero.name << reset << " — Niveau " << hero.level
                  << " — Classe : " << hero.className << " — Élément : " << elementName(hero.element) << "\n"
                  << "PV : " << hero.hp << "/" << hero.maxHp << " | ATK : " << hero.attack()
                  << " | DEF : " << hero.defense() << " | Chance : " << hero.criticalChance() << "% | Or : " << hero.gold << "\n"
                  << "XP : " << hero.xp << "/" << hero.xpNext() << " | Compétence : " << hero.characterClass->skill() << "\n"
                  << "Méthode C++ : " << hero.characterClass->skillMethodName() << "()\n"
                  << "Bonus d’équipement assorti : +" << hero.matchingEquipmentBonus() << "\n";
    }
}

void Game::inventory(){
    std::cout << "Choisissez le héros dont vous voulez gérer le sac (1-3) : ";
    int heroIndex=ask(1,3)-1;
    auto& hero=team[heroIndex];
    while(true){
        title("SAC — "+hero.name);
        for(size_t index=0;index<hero.inventory.size();++index){const auto& item=hero.inventory[index];std::cout<<index+1<<". "<<item.name<<" — "<<item.description<<" — Élément : "<<elementName(item.element)<<"\n";}
        std::cout<<"0 Retour\n> ";int choice=ask(0,static_cast<int>(hero.inventory.size()));if(choice==0)return;
        Item item=hero.inventory[choice-1];
        if(item.equipment){if(item.attack>0)hero.weapon=item;else hero.armor=item;hero.inventory.erase(hero.inventory.begin()+choice-1);std::cout<<"Équipement changé. Bonus élémentaire actuel : +"<<hero.matchingEquipmentBonus()<<".\n";}
        else if(item.heal){hero.hp=std::min(hero.maxHp,hero.hp+item.heal);hero.inventory.erase(hero.inventory.begin()+choice-1);std::cout<<"Potion utilisée.\n";}
        autoSave();
    }
}

void Game::quests(){
    title("QUÊTES DE L’ÉQUIPE");
    if(team.empty()) return;
    for(auto& quest:team.front().quests) std::cout<<(quest.claimed?"[RÉCOMPENSÉE] ":quest.completed?"[TERMINÉE] ":"[EN COURS] ")<<quest.name<<" — "<<quest.progress<<"/"<<quest.goal<<" — "<<quest.reward<<"\n";
    for(auto& quest:team.front().quests) if(quest.completed&&!quest.claimed){quest.claimed=true;for(auto& hero:team){hero.gold+=quest.gold;hero.gainXp(quest.xp);}}
    for(size_t index=1;index<team.size();++index) team[index].quests=team.front().quests;
    autoSave();
}

void Game::shop(){
    title("ÉCHOPPE");
    std::vector<Item> stock={{"Potion solaire","+45 PV",0,0,45,25,false,Element::Eau},{"Épée turquoise","ATK +9 — Eau",9,0,0,120,true,Element::Eau},{"Plastron forestier","DEF +8 — Plante",0,8,0,120,true,Element::Plante},{"Lame des braises","ATK +12 — Feu",12,0,0,180,true,Element::Feu}};
    for(size_t index=0;index<stock.size();++index) std::cout<<index+1<<". "<<stock[index].name<<" — "<<stock[index].value<<" or — Élément : "<<elementName(stock[index].element)<<"\n";
    std::cout<<"0 Retour\nChoisissez un objet : ";int choice=ask(0,static_cast<int>(stock.size()));if(choice==0)return;
    std::cout<<"Héros destinataire (1-3) : ";int heroIndex=ask(1,3)-1;
    if(team[heroIndex].gold>=stock[choice-1].value){team[heroIndex].gold-=stock[choice-1].value;team[heroIndex].addItem(stock[choice-1]);}else std::cout<<"La bourse de ce héros est insuffisante.\n";autoSave();
}

void Game::explore(){
    title("EXPLORATION");
    std::cout<<"1 Cathédrale des Cendres"<<(progress.dungeon1?" [PURIFIÉE]":"")<<"\n2 Jardins Engloutis"<<(progress.dungeon2?" [PURIFIÉE]":"")<<"\n3 Faille du Roi"<<(progress.dungeon3?" [PURIFIÉE]":"")<<"\n4 Sanctuaire inversé — optionnel"<<(progress.dungeon4?" [PURIFIÉ]":"")<<"\n0 Retour\n> ";
    int id=ask(0,4);
    if(id==1&&!progress.dungeon1)runDungeon(team,1,progress);else if(id==2&&progress.dungeon1&&!progress.dungeon2)runDungeon(team,2,progress);else if(id==3&&progress.dungeon2&&!progress.dungeon3)runDungeon(team,3,progress);else if(id==4&&progress.dungeon3&&!progress.dungeon4)runDungeon(team,4,progress);else if(id)std::cout<<"Le chemin est verrouillé ou déjà purifié.\n";
    autoSave();
}

void Game::hub(){
    while(!team.empty()){
        bool anyoneAlive=false;for(const auto& hero:team)if(hero.alive())anyoneAlive=true;if(!anyoneAlive)break;
        title("PLACE NOIRE — HUB");std::cout<<"1 Explorer les donjons\n2 Fiche de l’équipe\n3 Sac et équipement\n4 Quêtes\n5 Échoppe\n6 Repos\n7 Arène infinie\n0 Quitter\n> ";
        int choice=ask(0,7);if(choice==0){autoSave();return;}if(choice==1)explore();else if(choice==2)showStats();else if(choice==3)inventory();else if(choice==4)quests();else if(choice==5)shop();else if(choice==6){for(auto& hero:team)if(hero.alive())hero.hp=hero.maxHp;autoSave();}else if(choice==7){runArena(team,progress);autoSave();}
    }
}

void Game::run(){
    title("CHRONIQUES DE LA FAILLE");
    bool resumed=false;
    if(std::filesystem::exists("sauvegarde/savegame.txt")){std::cout<<"Sauvegarde d’équipe trouvée.\n1 Reprendre\n2 Nouvelle équipe\n> ";if(ask(1,2)==1)resumed=SaveSystem::load(team,progress);}
    if(!resumed)createTeam();
    hub();
    std::cout<<"Merci d’avoir écrit cette légende.\n";
}
