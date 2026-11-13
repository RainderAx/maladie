#pragma once

#include "../core/Player.hpp"
#include "../core/Types.hpp"
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

enum class PermanentStat { MaxHp, Attack, Defense };

struct UpgradeRanks {
    int maxHp = 0;
    int attack = 0;
    int defense = 0;
};

struct RogueliteProfile {
    std::vector<Player> roster;
    std::vector<UpgradeRanks> upgradeRanks;
    int fragments = 0;
};

struct Nemesis {
    std::string id;
    Enemy enemy;
    std::vector<Item> stolenEquipment;
};

struct RogueliteRun {
    std::vector<Player> team;
    std::vector<Nemesis> nemeses;
    std::uint32_t seed = 0;
    int completedRooms = 0;
    bool active = false;
    bool won = false;
    bool pendingReward = false;
};

struct Encounter {
    Enemy enemy;
    bool boss = false;
    std::string nemesisId;
};

enum class CombatAction { Attack, Skill, Potion };
enum class RoomReward { Potion, Rest };

struct CombatTurnResult {
    int playerDamage = 0;
    int enemyDamage = 0;
    int healing = 0;
    int targetIndex = -1;
    bool actionApplied = false;
    bool playerCritical = false;
    bool enemyCritical = false;
    bool heroDied = false;
    bool enemyDefeated = false;
    bool teamDefeated = false;
};

void initializeRogueliteProfile(RogueliteProfile& profile, std::vector<Player> roster);
bool buyPermanentUpgrade(RogueliteProfile& profile, std::size_t heroIndex, PermanentStat stat);
void awardRoomFragments(RogueliteProfile& profile, bool bossRoom);
bool startRogueliteRun(const RogueliteProfile& profile, RogueliteRun& run, std::uint32_t seed);
Nemesis createNemesisFromKill(const Enemy& enemy, Player& victim, const std::string& id);
void growNemesisFromKill(Nemesis& nemesis, Player& victim);
std::vector<Item> takeNemesisLoot(Nemesis& nemesis);
Encounter nextEncounter(const RogueliteRun& run);
CombatTurnResult resolveCombatAction(RogueliteRun& run, Encounter& encounter, std::size_t heroIndex,
                                     CombatAction action, std::mt19937& rng);
bool resolveEncounterVictory(RogueliteProfile& profile, RogueliteRun& run, Encounter& encounter);
bool claimRoomReward(RogueliteRun& run, std::size_t heroIndex, RoomReward reward);
