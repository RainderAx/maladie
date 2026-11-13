#include "../../include/systems/RogueliteSystem.hpp"
#include "../../include/entities/EnemyFactory.hpp"
#include "../../include/systems/CombatRules.hpp"
#include <algorithm>
#include <random>
#include <utility>

namespace {
constexpr int maxHeroCount = 3;
constexpr int maxHpRanks = 5;
constexpr int maxAttackRanks = 8;
constexpr int maxDefenseRanks = 8;
constexpr int normalRoomCount = 4;
constexpr std::size_t maxArmoryCount = 1000;

int rankFor(const UpgradeRanks& ranks, PermanentStat stat) {
    switch (stat) {
        case PermanentStat::MaxHp: return ranks.maxHp;
        case PermanentStat::Attack: return ranks.attack;
        case PermanentStat::Defense: return ranks.defense;
    }
    return 0;
}

int rankLimit(PermanentStat stat) {
    switch (stat) {
        case PermanentStat::MaxHp: return maxHpRanks;
        case PermanentStat::Attack: return maxAttackRanks;
        case PermanentStat::Defense: return maxDefenseRanks;
    }
    return 0;
}

int baseUpgradeCost(PermanentStat stat) {
    return stat == PermanentStat::MaxHp ? 4 : 6;
}

void incrementRank(UpgradeRanks& ranks, PermanentStat stat) {
    switch (stat) {
        case PermanentStat::MaxHp: ++ranks.maxHp; break;
        case PermanentStat::Attack: ++ranks.attack; break;
        case PermanentStat::Defense: ++ranks.defense; break;
    }
}

void applyUpgrade(Player& hero, PermanentStat stat) {
    switch (stat) {
        case PermanentStat::MaxHp:
            hero.maxHp += 10;
            hero.hp = hero.maxHp;
            break;
        case PermanentStat::Attack:
            ++hero.baseAttack;
            break;
        case PermanentStat::Defense:
            ++hero.baseDefense;
            break;
    }
}

void stealEquippedItems(Nemesis& nemesis, Player& victim) {
    const Item weapon = victim.weapon;
    const Item armor = victim.armor;
    nemesis.stolenEquipment.push_back(weapon);
    nemesis.stolenEquipment.push_back(armor);
    victim.weapon = {"", "", 0, 0, 0, 0, false, victim.element};
    victim.armor = {"", "", 0, 0, 0, 0, false, victim.element};
    nemesis.enemy.attack += weapon.attack;
    nemesis.enemy.defense += armor.defense;
    nemesis.enemy.maxHp += 10 * std::max(1, victim.level);
    nemesis.enemy.chance = std::min(60, nemesis.enemy.chance + 2);
}

bool teamAlive(const std::vector<Player>& team) {
    return std::any_of(team.begin(), team.end(), [](const Player& hero) { return hero.alive(); });
}

std::size_t livingHeroCount(const std::vector<Player>& team) {
    return static_cast<std::size_t>(std::count_if(team.begin(), team.end(), [](const Player& hero) { return hero.alive(); }));
}

int roll(std::mt19937& rng, int min, int max) {
    return std::uniform_int_distribution<int>(min, max)(rng);
}

Nemesis* findNemesis(RogueliteRun& run, const std::string& id) {
    const auto found = std::find_if(run.nemeses.begin(), run.nemeses.end(),
        [&id](const Nemesis& nemesis) { return nemesis.id == id; });
    return found == run.nemeses.end() ? nullptr : &*found;
}

void promoteKiller(RogueliteRun& run, Encounter& encounter, Player& victim) {
    const int currentHp = encounter.enemy.hp;
    if (encounter.nemesisId.empty()) {
        encounter.nemesisId = "rival-" + std::to_string(run.seed) + "-" +
            std::to_string(run.completedRooms + 1) + "-" + std::to_string(run.nemeses.size() + 1);
        run.nemeses.push_back(createNemesisFromKill(encounter.enemy, victim, encounter.nemesisId));
    } else if (Nemesis* nemesis = findNemesis(run, encounter.nemesisId)) {
        growNemesisFromKill(*nemesis, victim);
    } else {
        run.nemeses.push_back(createNemesisFromKill(encounter.enemy, victim, encounter.nemesisId));
    }
    if (Nemesis* nemesis = findNemesis(run, encounter.nemesisId)) {
        nemesis->enemy.hp = std::clamp(currentHp, 0, nemesis->enemy.maxHp);
        encounter.enemy = nemesis->enemy;
    }
}
}

void initializeRogueliteProfile(RogueliteProfile& profile, std::vector<Player> roster) {
    profile.roster = std::move(roster);
    profile.upgradeRanks.assign(profile.roster.size(), UpgradeRanks{});
    profile.fragments = std::max(0, profile.fragments);
}

const std::vector<EquipmentOffer>& rogueliteEquipmentOffers() {
    static const std::vector<EquipmentOffer> offers{
        {{"Épée turquoise", "ATK +9 — Eau", 9, 0, 0, 120, true, Element::Eau}, 6},
        {{"Plastron forestier", "DEF +8 — Plante", 0, 8, 0, 120, true, Element::Plante}, 6},
        {{"Lame des braises", "ATK +12 — Feu", 12, 0, 0, 180, true, Element::Feu}, 9}
    };
    return offers;
}

bool buyRogueliteEquipment(RogueliteProfile& profile, std::size_t offerIndex) {
    const auto& offers = rogueliteEquipmentOffers();
    if (offerIndex >= offers.size() || profile.fragments < offers[offerIndex].fragmentCost ||
        profile.armory.size() >= maxArmoryCount) return false;
    profile.armory.push_back(offers[offerIndex].item);
    profile.fragments -= offers[offerIndex].fragmentCost;
    return true;
}

bool equipFromArmory(RogueliteProfile& profile, std::size_t heroIndex, std::size_t itemIndex, EquipmentSlot slot) {
    if (heroIndex >= profile.roster.size() || itemIndex >= profile.armory.size()) return false;
    const Item item = profile.armory[itemIndex];
    if (!item.equipment ||
        (slot == EquipmentSlot::Weapon && item.attack <= 0) ||
        (slot == EquipmentSlot::Armor && item.defense <= 0)) return false;

    Player& hero = profile.roster[heroIndex];
    Item& equipped = slot == EquipmentSlot::Weapon ? hero.weapon : hero.armor;
    const Item replaced = equipped;
    equipped = item;
    profile.armory.erase(profile.armory.begin() + static_cast<std::ptrdiff_t>(itemIndex));
    if (replaced.equipment && !replaced.name.empty()) profile.armory.push_back(replaced);
    return true;
}

bool equipRunItem(RogueliteRun& run, std::size_t heroIndex, std::size_t inventoryIndex) {
    if (!run.active || !run.pendingReward || heroIndex >= run.team.size() || !run.team[heroIndex].alive()) return false;
    Player& hero = run.team[heroIndex];
    if (inventoryIndex >= hero.inventory.size()) return false;
    const Item item = hero.inventory[inventoryIndex];
    if (!item.equipment || (item.attack <= 0 && item.defense <= 0)) return false;
    Item& equipped = item.attack > 0 ? hero.weapon : hero.armor;
    const Item replaced = equipped;
    equipped = item;
    hero.inventory.erase(hero.inventory.begin() + static_cast<std::ptrdiff_t>(inventoryIndex));
    if (replaced.equipment && !replaced.name.empty()) hero.inventory.push_back(replaced);
    return true;
}

bool upgradeRunEquipment(RogueliteRun& run, std::size_t heroIndex, EquipmentSlot slot) {
    if (!run.active || !run.pendingReward || heroIndex >= run.team.size() || !run.team[heroIndex].alive()) return false;
    Player& hero = run.team[heroIndex];
    Item& item = slot == EquipmentSlot::Weapon ? hero.weapon : hero.armor;
    if (!item.equipment || item.upgradeLevel < 0 || item.upgradeLevel >= 3 ||
        (slot == EquipmentSlot::Weapon && item.attack <= 0) ||
        (slot == EquipmentSlot::Armor && item.defense <= 0)) return false;
    const int cost = 45 + 30 * item.upgradeLevel;
    if (hero.gold < cost) return false;
    hero.gold -= cost;
    if (slot == EquipmentSlot::Weapon) item.attack += 2;
    else item.defense += 2;
    ++item.upgradeLevel;
    return true;
}

bool extractRunLoot(RogueliteProfile& profile, RogueliteRun& run, std::size_t heroIndex, std::size_t inventoryIndex) {
    if (!run.active || !run.pendingReward || run.roomLootExtracted || heroIndex >= run.team.size() ||
        inventoryIndex >= run.team[heroIndex].inventory.size() || profile.armory.size() >= maxArmoryCount) return false;
    Player& hero = run.team[heroIndex];
    const Item item = hero.inventory[inventoryIndex];
    if (!item.equipment || (item.attack <= 0 && item.defense <= 0)) return false;
    profile.armory.push_back(item);
    hero.inventory.erase(hero.inventory.begin() + static_cast<std::ptrdiff_t>(inventoryIndex));
    run.roomLootExtracted = true;
    return true;
}

bool buyPermanentUpgrade(RogueliteProfile& profile, std::size_t heroIndex, PermanentStat stat) {
    if (heroIndex >= profile.roster.size() || heroIndex >= profile.upgradeRanks.size()) return false;
    UpgradeRanks& ranks = profile.upgradeRanks[heroIndex];
    const int rank = rankFor(ranks, stat);
    if (rank < 0 || rank >= rankLimit(stat)) return false;
    const int cost = baseUpgradeCost(stat) + rank * 2;
    if (profile.fragments < cost) return false;

    profile.fragments -= cost;
    incrementRank(ranks, stat);
    applyUpgrade(profile.roster[heroIndex], stat);
    return true;
}

void awardRoomFragments(RogueliteProfile& profile, bool bossRoom) {
    profile.fragments += bossRoom ? 4 : 1;
}

bool startRogueliteRun(const RogueliteProfile& profile, RogueliteRun& run, std::uint32_t seed) {
    if (profile.roster.empty() || profile.roster.size() > maxHeroCount ||
        profile.upgradeRanks.size() != profile.roster.size()) return false;

    run = RogueliteRun{};
    run.team = profile.roster;
    run.seed = seed;
    run.active = true;
    for (Player& hero : run.team) {
        hero.level = 1;
        hero.xp = 0;
        hero.gold = 80;
        hero.hp = hero.maxHp;
        hero.inventory = {
            {"Potion solaire", "+45 PV", 0, 0, 45, 25, false, Element::Eau},
            {"Potion solaire", "+45 PV", 0, 0, 45, 25, false, Element::Eau}
        };
        hero.quests.clear();
    }
    return true;
}

Nemesis createNemesisFromKill(const Enemy& enemy, Player& victim, const std::string& id) {
    Nemesis nemesis{id, enemy, {}};
    stealEquippedItems(nemesis, victim);
    return nemesis;
}

void growNemesisFromKill(Nemesis& nemesis, Player& victim) {
    stealEquippedItems(nemesis, victim);
}

std::vector<Item> takeNemesisLoot(Nemesis& nemesis) {
    std::vector<Item> loot = std::move(nemesis.stolenEquipment);
    nemesis.stolenEquipment.clear();
    return loot;
}

Encounter nextEncounter(const RogueliteRun& run) {
    if (run.completedRooms >= normalRoomCount) {
        Enemy boss = makeEnemy(5, true);
        boss.maxHp = static_cast<int>(boss.maxHp * 1.30);
        boss.hp = boss.maxHp;
        return {boss, true, {}};
    }
    if (!run.nemeses.empty()) {
        const Nemesis& nemesis = run.nemeses.front();
        return {nemesis.enemy, false, nemesis.id};
    }

    std::vector<Enemy> roster = arenaRoster(1);
    for (Enemy& candidate : roster) {
        candidate.maxHp = static_cast<int>(candidate.maxHp * 1.15);
        candidate.hp = candidate.maxHp;
    }
    const auto power = [](const Enemy& candidate) {
        return candidate.maxHp + candidate.attack * 3 + candidate.defense * 2;
    };
    std::stable_sort(roster.begin(), roster.end(), [&power](const Enemy& left, const Enemy& right) {
        return power(left) < power(right);
    });
    const std::size_t room = static_cast<std::size_t>(std::clamp(run.completedRooms, 0, normalRoomCount - 1));
    constexpr std::size_t candidatesPerRoom = 5;
    const std::size_t first = room * candidatesPerRoom;
    const std::size_t last = std::min(first + candidatesPerRoom, roster.size());
    std::mt19937 rng(run.seed ^ (static_cast<std::uint32_t>(run.completedRooms + 1) * 2654435761U));
    const std::size_t index = std::uniform_int_distribution<std::size_t>(first, last - 1)(rng);
    Enemy enemy = roster[index];
    return {enemy, false, {}};
}

CombatTurnResult resolveCombatAction(RogueliteRun& run, Encounter& encounter, std::size_t heroIndex,
                                     CombatAction action, std::mt19937& rng) {
    CombatTurnResult result;
    if (!run.active || encounter.enemy.hp <= 0 || heroIndex >= run.team.size() || !run.team[heroIndex].alive()) return result;

    Player& hero = run.team[heroIndex];
    result.actionApplied = true;
    if (action == CombatAction::Potion) {
        const auto potion = std::find_if(hero.inventory.begin(), hero.inventory.end(),
            [](const Item& item) { return item.heal > 0; });
        if (potion == hero.inventory.end()) {
            result.actionApplied = false;
            return result;
        }
        result.healing = std::min(hero.maxHp - hero.hp, potion->heal);
        hero.hp += result.healing;
        hero.inventory.erase(potion);
    } else {
        result.playerCritical = roll(rng, 1, 100) <= hero.criticalChance();
        const int variation = action == CombatAction::Skill ? roll(rng, 0, 4) : roll(rng, -3, 6);
        result.playerDamage = CombatRules::playerDamage(hero, encounter.enemy, action == CombatAction::Skill,
                                                         variation, result.playerCritical);
        encounter.enemy.hp -= result.playerDamage;
        if (action == CombatAction::Skill) {
            const int skillHeal = hero.characterClass->skillHeal(hero.maxHp);
            result.healing = std::min(hero.maxHp - hero.hp, skillHeal);
            hero.hp += result.healing;
        }
        if (encounter.enemy.hp <= 0) {
            result.enemyDefeated = true;
            return result;
        }
    }

    if (livingHeroCount(run.team) == 0) {
        run.active = false;
        result.teamDefeated = true;
        return result;
    }

    std::vector<std::size_t> living;
    living.reserve(run.team.size());
    for (std::size_t index = 0; index < run.team.size(); ++index) {
        if (run.team[index].alive()) living.push_back(index);
    }
    const std::size_t targetIndex = living[std::uniform_int_distribution<std::size_t>(0, living.size() - 1)(rng)];
    Player& target = run.team[targetIndex];
    result.targetIndex = static_cast<int>(targetIndex);
    result.enemyCritical = roll(rng, 1, 100) <= encounter.enemy.chance;
    result.enemyDamage = CombatRules::enemyDamage(encounter.enemy, target, roll(rng, -3, 5), result.enemyCritical);
    target.hp = std::max(0, target.hp - result.enemyDamage);
    if (!target.alive()) {
        result.heroDied = true;
        ++run.roomDeaths;
        promoteKiller(run, encounter, target);
        if (!teamAlive(run.team)) {
            run.active = false;
            result.teamDefeated = true;
        }
    }
    return result;
}

bool resolveEncounterVictory(RogueliteProfile& profile, RogueliteRun& run, Encounter& encounter) {
    if (!run.active || encounter.enemy.hp > 0) return false;
    for (Player& hero : run.team) {
        hero.gold += encounter.enemy.gold;
        hero.gainXp(encounter.enemy.xp);
    }

    if (!encounter.nemesisId.empty()) {
        const auto found = std::find_if(run.nemeses.begin(), run.nemeses.end(),
            [&encounter](const Nemesis& nemesis) { return nemesis.id == encounter.nemesisId; });
        if (found != run.nemeses.end()) {
            std::vector<Item> loot = takeNemesisLoot(*found);
            const auto survivor = std::find_if(run.team.begin(), run.team.end(),
                [](const Player& hero) { return hero.alive(); });
            if (survivor != run.team.end()) {
                for (const Item& item : loot) survivor->addItem(item);
            }
            run.nemeses.erase(found);
        }
    }

    awardRoomFragments(profile, encounter.boss);
    const int xpStep = encounter.boss ? 250 : 100;
    profile.fragments += std::min(2, encounter.enemy.xp / xpStep);
    if (!encounter.nemesisId.empty()) ++profile.fragments;
    if (run.roomDeaths == 0) ++profile.fragments;
    run.roomDeaths = 0;
    if (encounter.boss) {
        const std::size_t heroCount = std::min(profile.roster.size(), run.team.size());
        for (std::size_t index = 0; index < heroCount; ++index) {
            profile.roster[index].weapon = run.team[index].weapon;
            profile.roster[index].armor = run.team[index].armor;
        }
        run.won = true;
        run.active = false;
    } else {
        ++run.completedRooms;
        run.roomLootExtracted = false;
        run.pendingReward = true;
    }
    return true;
}

bool claimRoomReward(RogueliteRun& run, std::size_t heroIndex, RoomReward reward) {
    if (!run.active || !run.pendingReward || heroIndex >= run.team.size() || !run.team[heroIndex].alive()) return false;
    if (reward == RoomReward::Potion) {
        run.team[heroIndex].addItem({"Potion solaire", "+45 PV", 0, 0, 45, 25, false, Element::Eau});
    } else {
        for (Player& hero : run.team) {
            if (hero.alive()) hero.hp = std::min(hero.maxHp, hero.hp + std::max(1, hero.maxHp / 5));
        }
    }
    run.pendingReward = false;
    return true;
}
