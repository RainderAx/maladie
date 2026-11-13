#include "../../include/systems/SaveSystem.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace SaveSystem {

static void writeItem(std::ofstream& out, const Item& item){
    out << std::quoted(item.name) << ' ' << std::quoted(item.description) << ' '
        << item.attack << ' ' << item.defense << ' ' << item.heal << ' ' << item.value << ' '
        << item.equipment << ' ' << elementId(item.element) << '\n';
}

static bool readItem(std::ifstream& in, Item& item){
    int element;
    return static_cast<bool>(in >> std::quoted(item.name) >> std::quoted(item.description)
        >> item.attack >> item.defense >> item.heal >> item.value >> item.equipment >> element)
        && (item.element = elementFromId(element), true);
}

bool save(const std::vector<Player>& team, const Progress& progress, const std::string& file){
    std::filesystem::create_directories(std::filesystem::path(file).parent_path());
    std::ofstream out(file);
    if(!out) return false;
    out << "CHRONIQUES_SAVE_V3\n" << team.size() << '\n';
    for(const auto& hero : team){
        out << std::quoted(hero.name) << ' ' << std::quoted(hero.className) << ' '
            << hero.characterClass->id() << ' ' << elementId(hero.element) << ' '
            << hero.level << ' ' << hero.xp << ' ' << hero.gold << ' ' << hero.hp << ' '
            << hero.maxHp << ' ' << hero.baseAttack << ' ' << hero.baseDefense << ' ' << hero.chance << '\n';
        writeItem(out, hero.weapon);
        writeItem(out, hero.armor);
        out << hero.inventory.size() << '\n';
        for(const auto& item : hero.inventory) writeItem(out, item);
    }
    const std::vector<Quest> quests = team.empty() ? std::vector<Quest>{} : team.front().quests;
    out << quests.size() << '\n';
    for(const auto& quest : quests){
        out << std::quoted(quest.name) << ' ' << std::quoted(quest.objective) << ' '
            << std::quoted(quest.reward) << ' ' << quest.goal << ' ' << quest.progress << ' '
            << quest.xp << ' ' << quest.gold << ' ' << quest.completed << ' ' << quest.claimed << '\n';
    }
    out << progress.dungeon1 << ' ' << progress.dungeon2 << ' ' << progress.dungeon3 << ' '
        << progress.dungeon4 << ' ' << progress.arenaBestWave << '\n';
    return true;
}

bool load(std::vector<Player>& team, Progress& progress, const std::string& file){
    std::ifstream in(file);
    std::string marker;
    if(!(in >> marker) || marker != "CHRONIQUES_SAVE_V3") return false;
    size_t teamSize;
    if(!(in >> teamSize) || teamSize == 0 || teamSize > 3) return false;
    team.clear();
    for(size_t index=0; index<teamSize; ++index){
        Player hero;
        int classId, element;
        if(!(in >> std::quoted(hero.name) >> std::quoted(hero.className) >> classId >> element
            >> hero.level >> hero.xp >> hero.gold >> hero.hp >> hero.maxHp
            >> hero.baseAttack >> hero.baseDefense >> hero.chance)) return false;
        hero.characterClass=makeCharacterClass(classId);
        hero.element=elementFromId(element);
        if(!readItem(in, hero.weapon) || !readItem(in, hero.armor)) return false;
        size_t itemCount;
        if(!(in >> itemCount)) return false;
        for(size_t itemIndex=0; itemIndex<itemCount; ++itemIndex){Item item;if(!readItem(in,item))return false;hero.inventory.push_back(item);}
        team.push_back(std::move(hero));
    }
    size_t questCount;
    if(!(in >> questCount)) return false;
    std::vector<Quest> quests;
    for(size_t index=0; index<questCount; ++index){
        Quest quest;
        if(!(in >> std::quoted(quest.name) >> std::quoted(quest.objective) >> std::quoted(quest.reward)
            >> quest.goal >> quest.progress >> quest.xp >> quest.gold >> quest.completed >> quest.claimed)) return false;
        quests.push_back(quest);
    }
    for(auto& hero:team) hero.quests=quests;
    return static_cast<bool>(in >> progress.dungeon1 >> progress.dungeon2 >> progress.dungeon3 >> progress.dungeon4 >> progress.arenaBestWave);
}
}
