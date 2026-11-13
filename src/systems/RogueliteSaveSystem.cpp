#include "../../include/systems/RogueliteSaveSystem.hpp"
#include "../../include/characters/CharacterFactory.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <system_error>

namespace {
constexpr std::size_t maxHeroCount = 3;
constexpr std::size_t maxInventoryCount = 1000;
constexpr std::size_t maxNemesisCount = 3;
constexpr std::size_t maxStolenItems = 6;

bool validElement(int id) { return id >= 0 && id <= 3; }

bool validItem(const Item& item) {
    return item.attack >= 0 && item.defense >= 0 && item.heal >= 0 && item.value >= 0 &&
        item.upgradeLevel >= 0 && item.upgradeLevel <= 3 && validElement(elementId(item.element));
}

bool validPlayer(const Player& hero) {
    if (!hero.characterClass || hero.level < 1 || hero.maxHp < 1 || hero.hp < 0 || hero.hp > hero.maxHp ||
        hero.xp < 0 || hero.gold < 0 || hero.baseAttack < 0 || hero.baseDefense < 0 ||
        hero.chance < 0 || hero.chance > 100 || !validElement(elementId(hero.element)) ||
        !validItem(hero.weapon) || !validItem(hero.armor) || hero.inventory.size() > maxInventoryCount) return false;
    return std::all_of(hero.inventory.begin(), hero.inventory.end(), validItem);
}

bool validEnemy(const Enemy& enemy) {
    return enemy.maxHp >= 1 && enemy.hp >= 0 && enemy.hp <= enemy.maxHp && enemy.attack >= 0 &&
        enemy.defense >= 0 && enemy.chance >= 0 && enemy.chance <= 100 && enemy.xp >= 0 && enemy.gold >= 0 &&
        validElement(elementId(enemy.element));
}

bool validRanks(const UpgradeRanks& ranks) {
    return ranks.maxHp >= 0 && ranks.maxHp <= 5 && ranks.attack >= 0 && ranks.attack <= 8 &&
        ranks.defense >= 0 && ranks.defense <= 8;
}

void writeItem(std::ostream& out, const Item& item) {
    out << std::quoted(item.name) << ' ' << std::quoted(item.description) << ' '
        << item.attack << ' ' << item.defense << ' ' << item.heal << ' ' << item.value << ' '
        << item.equipment << ' ' << elementId(item.element) << ' ' << item.upgradeLevel << '\n';
}

bool readItem(std::istream& in, Item& item, bool hasUpgradeLevel) {
    int element = -1;
    if (!(in >> std::quoted(item.name) >> std::quoted(item.description)
          >> item.attack >> item.defense >> item.heal >> item.value >> item.equipment >> element)) return false;
    item.upgradeLevel = 0;
    if (hasUpgradeLevel && !(in >> item.upgradeLevel)) return false;
    if (!validElement(element) || item.attack < 0 || item.defense < 0 || item.heal < 0 || item.value < 0 ||
        item.upgradeLevel < 0 || item.upgradeLevel > 3) return false;
    item.element = elementFromId(element);
    return true;
}

void writePlayer(std::ostream& out, const Player& hero) {
    out << std::quoted(hero.name) << ' ' << std::quoted(hero.className) << ' '
        << hero.characterClass->id() << ' ' << elementId(hero.element) << ' '
        << hero.level << ' ' << hero.xp << ' ' << hero.gold << ' ' << hero.hp << ' '
        << hero.maxHp << ' ' << hero.baseAttack << ' ' << hero.baseDefense << ' ' << hero.chance << '\n';
    writeItem(out, hero.weapon);
    writeItem(out, hero.armor);
    out << hero.inventory.size() << '\n';
    for (const Item& item : hero.inventory) writeItem(out, item);
}

bool readPlayer(std::istream& in, Player& hero, bool hasUpgradeLevel) {
    int classId = 0;
    int element = -1;
    if (!(in >> std::quoted(hero.name) >> std::quoted(hero.className) >> classId >> element
          >> hero.level >> hero.xp >> hero.gold >> hero.hp >> hero.maxHp
          >> hero.baseAttack >> hero.baseDefense >> hero.chance)) return false;
    if (classId < 1 || classId > 10 || !validElement(element) || hero.level < 1 || hero.maxHp < 1 ||
        hero.hp < 0 || hero.hp > hero.maxHp || hero.xp < 0 || hero.gold < 0 || hero.baseAttack < 0 ||
        hero.baseDefense < 0 || hero.chance < 0 || hero.chance > 100) return false;
    hero.characterClass = makeCharacterClass(classId);
    hero.element = elementFromId(element);
    if (!readItem(in, hero.weapon, hasUpgradeLevel) || !readItem(in, hero.armor, hasUpgradeLevel)) return false;
    std::size_t inventoryCount = 0;
    if (!(in >> inventoryCount) || inventoryCount > maxInventoryCount) return false;
    hero.inventory.clear();
    for (std::size_t index = 0; index < inventoryCount; ++index) {
        Item item;
        if (!readItem(in, item, hasUpgradeLevel)) return false;
        hero.inventory.push_back(std::move(item));
    }
    hero.quests.clear();
    return true;
}

void writeEnemy(std::ostream& out, const Enemy& enemy) {
    out << std::quoted(enemy.name) << ' ' << std::quoted(enemy.className) << ' '
        << enemy.hp << ' ' << enemy.maxHp << ' ' << enemy.attack << ' ' << enemy.defense << ' '
        << enemy.chance << ' ' << enemy.xp << ' ' << enemy.gold << ' '
        << elementId(enemy.element) << ' ' << enemy.boss << '\n';
}

bool readEnemy(std::istream& in, Enemy& enemy) {
    int element = -1;
    if (!(in >> std::quoted(enemy.name) >> std::quoted(enemy.className)
          >> enemy.hp >> enemy.maxHp >> enemy.attack >> enemy.defense >> enemy.chance
          >> enemy.xp >> enemy.gold >> element >> enemy.boss)) return false;
    if (!validElement(element) || enemy.maxHp < 1 || enemy.hp < 0 || enemy.hp > enemy.maxHp ||
        enemy.attack < 0 || enemy.defense < 0 || enemy.chance < 0 || enemy.chance > 100 ||
        enemy.xp < 0 || enemy.gold < 0) return false;
    enemy.element = elementFromId(element);
    return true;
}

template<typename Writer>
bool writeAtomically(const std::string& file, Writer writer) {
    const std::filesystem::path destination(file);
    const std::filesystem::path temporary = destination.string() + ".tmp";
    if (!destination.parent_path().empty()) {
        std::error_code error;
        std::filesystem::create_directories(destination.parent_path(), error);
        if (error) return false;
    }

    std::ofstream out(temporary, std::ios::out | std::ios::trunc);
    if (!out || !writer(out)) {
        out.close();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    out.flush();
    if (!out) {
        out.close();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    out.close();
    if (!out) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }

    std::error_code error;
    std::filesystem::rename(temporary, destination, error);
    if (error) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
    return true;
}
}

namespace RogueliteSaveSystem {
bool saveProfile(const RogueliteProfile& profile, const std::string& file) {
    if (profile.roster.empty() || profile.roster.size() > maxHeroCount ||
        profile.upgradeRanks.size() != profile.roster.size() || profile.fragments < 0 ||
        profile.armory.size() > maxInventoryCount ||
        !std::all_of(profile.armory.begin(), profile.armory.end(), validItem)) return false;
    for (std::size_t index = 0; index < profile.roster.size(); ++index) {
        if (!validPlayer(profile.roster[index]) || !validRanks(profile.upgradeRanks[index])) return false;
    }
    return writeAtomically(file, [&profile](std::ostream& out) {
        out << "ROGUELITE_PROFILE_V2\n" << profile.roster.size() << '\n';
        for (std::size_t index = 0; index < profile.roster.size(); ++index) {
            writePlayer(out, profile.roster[index]);
            const UpgradeRanks& ranks = profile.upgradeRanks[index];
            out << ranks.maxHp << ' ' << ranks.attack << ' ' << ranks.defense << '\n';
        }
        out << profile.fragments << '\n';
        out << profile.armory.size() << '\n';
        for (const Item& item : profile.armory) writeItem(out, item);
        return static_cast<bool>(out);
    });
}

bool loadProfile(RogueliteProfile& profile, const std::string& file) {
    std::ifstream in(file);
    std::string marker;
    std::size_t count = 0;
    if (!(in >> marker) || (marker != "ROGUELITE_PROFILE_V1" && marker != "ROGUELITE_PROFILE_V2") ||
        !(in >> count) || count == 0 || count > maxHeroCount) return false;
    const bool hasUpgradeLevel = marker == "ROGUELITE_PROFILE_V2";
    RogueliteProfile loaded;
    loaded.roster.reserve(count);
    loaded.upgradeRanks.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        Player hero;
        UpgradeRanks ranks;
        if (!readPlayer(in, hero, hasUpgradeLevel) || !(in >> ranks.maxHp >> ranks.attack >> ranks.defense) || !validRanks(ranks)) return false;
        loaded.roster.push_back(std::move(hero));
        loaded.upgradeRanks.push_back(ranks);
    }
    if (!(in >> loaded.fragments) || loaded.fragments < 0) return false;
    if (hasUpgradeLevel) {
        std::size_t armoryCount = 0;
        if (!(in >> armoryCount) || armoryCount > maxInventoryCount) return false;
        loaded.armory.reserve(armoryCount);
        for (std::size_t index = 0; index < armoryCount; ++index) {
            Item item;
            if (!readItem(in, item, true)) return false;
            loaded.armory.push_back(std::move(item));
        }
    }
    profile = std::move(loaded);
    return true;
}

bool saveRun(const RogueliteRun& run, const std::string& file) {
    if (run.team.empty() || run.team.size() > maxHeroCount || run.nemeses.size() > maxNemesisCount ||
        run.completedRooms < 0 || run.completedRooms > 4 || run.roomDeaths < 0 || (run.active && run.won)) return false;
    if (!std::all_of(run.team.begin(), run.team.end(), validPlayer)) return false;
    for (const Nemesis& nemesis : run.nemeses) {
        if (!validEnemy(nemesis.enemy) || nemesis.stolenEquipment.size() > maxStolenItems ||
            !std::all_of(nemesis.stolenEquipment.begin(), nemesis.stolenEquipment.end(), validItem)) return false;
    }
    return writeAtomically(file, [&run](std::ostream& out) {
        out << "ROGUELITE_RUN_V2\n" << run.team.size() << '\n';
        for (const Player& hero : run.team) writePlayer(out, hero);
        out << run.seed << ' ' << run.completedRooms << ' ' << run.active << ' ' << run.won << ' ' << run.pendingReward << '\n';
        out << run.roomDeaths << ' ' << run.roomLootExtracted << '\n';
        out << run.nemeses.size() << '\n';
        for (const Nemesis& nemesis : run.nemeses) {
            out << std::quoted(nemesis.id) << '\n';
            writeEnemy(out, nemesis.enemy);
            out << nemesis.stolenEquipment.size() << '\n';
            for (const Item& item : nemesis.stolenEquipment) writeItem(out, item);
        }
        return static_cast<bool>(out);
    });
}

bool loadRun(RogueliteRun& run, const std::string& file) {
    std::ifstream in(file);
    std::string marker;
    std::size_t teamCount = 0;
    if (!(in >> marker) || (marker != "ROGUELITE_RUN_V1" && marker != "ROGUELITE_RUN_V2") ||
        !(in >> teamCount) || teamCount == 0 || teamCount > maxHeroCount) return false;
    const bool hasUpgradeLevel = marker == "ROGUELITE_RUN_V2";
    RogueliteRun loaded;
    loaded.team.reserve(teamCount);
    for (std::size_t index = 0; index < teamCount; ++index) {
        Player hero;
        if (!readPlayer(in, hero, hasUpgradeLevel)) return false;
        loaded.team.push_back(std::move(hero));
    }
    if (!(in >> loaded.seed >> loaded.completedRooms >> loaded.active >> loaded.won >> loaded.pendingReward) ||
        loaded.completedRooms < 0 || loaded.completedRooms > 4 || (loaded.active && loaded.won)) return false;
    if (hasUpgradeLevel && (!(in >> loaded.roomDeaths >> loaded.roomLootExtracted) || loaded.roomDeaths < 0)) return false;
    std::size_t nemesisCount = 0;
    if (!(in >> nemesisCount) || nemesisCount > maxNemesisCount) return false;
    loaded.nemeses.reserve(nemesisCount);
    for (std::size_t index = 0; index < nemesisCount; ++index) {
        Nemesis nemesis;
        if (!(in >> std::quoted(nemesis.id)) || !readEnemy(in, nemesis.enemy)) return false;
        std::size_t itemCount = 0;
        if (!(in >> itemCount) || itemCount > maxStolenItems) return false;
        for (std::size_t itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
            Item item;
            if (!readItem(in, item, hasUpgradeLevel)) return false;
            nemesis.stolenEquipment.push_back(std::move(item));
        }
        loaded.nemeses.push_back(std::move(nemesis));
    }
    run = std::move(loaded);
    return true;
}
}
