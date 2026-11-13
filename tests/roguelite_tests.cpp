#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "core/HeroFactory.hpp"
#include "systems/CombatRules.hpp"
#include "systems/RogueliteSystem.hpp"
#include "systems/RogueliteSaveSystem.hpp"
#include "systems/SaveSystem.hpp"

namespace {
int failures = 0;

void check(bool condition, const char* expression, int line) {
    if (!condition) {
        std::cerr << "Échec ligne " << line << " : " << expression << '\n';
        ++failures;
    }
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __LINE__)

Player heroFixture() {
    Player hero;
    hero.name = "Aube";
    hero.level = 3;
    hero.xp = 42;
    hero.gold = 230;
    hero.hp = 61;
    hero.maxHp = 100;
    hero.baseAttack = 10;
    hero.baseDefense = 4;
    hero.weapon = {"Épée héritée", "Une arme de test.", 7, 0, 0, 80, true, Element::Feu};
    hero.armor = {"Cuirasse héritée", "Une armure de test.", 0, 3, 0, 70, true, Element::Eau};
    hero.inventory.clear();
    return hero;
}

void permanentUpgradesCostMoreAndPersistInTheNextRun() {
    RogueliteProfile profile;
    profile.roster.push_back(heroFixture());
    profile.upgradeRanks.resize(1);
    profile.fragments = 10;

    CHECK(buyPermanentUpgrade(profile, 0, PermanentStat::MaxHp));
    CHECK(profile.fragments == 6);
    CHECK(profile.roster[0].maxHp == 110);
    CHECK(buyPermanentUpgrade(profile, 0, PermanentStat::MaxHp));
    CHECK(profile.fragments == 0);
    CHECK(profile.roster[0].maxHp == 120);
    CHECK(!buyPermanentUpgrade(profile, 0, PermanentStat::MaxHp));

    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 1234));
    CHECK(run.seed == 1234);
    CHECK(run.active);
    CHECK(run.team.size() == 1);
    CHECK(run.team[0].level == 1);
    CHECK(run.team[0].xp == 0);
    CHECK(run.team[0].gold == 80);
    CHECK(run.team[0].maxHp == 120);
    CHECK(run.team[0].hp == 120);
    CHECK(run.team[0].weapon.name == "Épée héritée");
}

void completedRoomsBankFragmentsEvenWhenTheRunLaterFails() {
    RogueliteProfile profile;
    profile.roster.push_back(heroFixture());
    profile.upgradeRanks.resize(1);

    awardRoomFragments(profile, false);
    CHECK(profile.fragments == 1);
    awardRoomFragments(profile, true);
    CHECK(profile.fragments == 5);
}

void aNemesisStealsAndAccumulatesTheEquipmentAndStatsOfItsVictims() {
    Enemy enemy{"Garde de Fer", "Guerrier", 30, 30, 10, 2, 10, 0, 0, Element::Feu, false};
    Player first = heroFixture();
    first.level = 2;
    Nemesis rival = createNemesisFromKill(enemy, first, "nemesis-1");

    CHECK(rival.id == "nemesis-1");
    CHECK(rival.enemy.attack == 17);
    CHECK(rival.enemy.defense == 5);
    CHECK(rival.enemy.maxHp == 50);
    CHECK(rival.enemy.hp == 30);
    CHECK(rival.enemy.chance == 12);
    CHECK(rival.stolenEquipment.size() == 2);
    CHECK(rival.stolenEquipment[0].name == "Épée héritée");
    CHECK(rival.stolenEquipment[1].name == "Cuirasse héritée");
    CHECK(first.weapon.attack == 0);
    CHECK(first.armor.defense == 0);

    Player second = heroFixture();
    second.name = "Brume";
    second.level = 1;
    second.weapon.attack = 4;
    second.armor.defense = 2;
    growNemesisFromKill(rival, second);
    CHECK(rival.enemy.attack == 21);
    CHECK(rival.enemy.defense == 7);
    CHECK(rival.enemy.maxHp == 60);
    CHECK(rival.enemy.chance == 14);
    CHECK(rival.stolenEquipment.size() == 4);

    const std::vector<Item> recovered = takeNemesisLoot(rival);
    CHECK(recovered.size() == 4);
    CHECK(rival.stolenEquipment.empty());
}

void heroFactoryPreservesTheExistingClassAndElementDefaults() {
    const Player warrior = createHero("Ada", 1, Element::Feu);
    CHECK(warrior.name == "Ada");
    CHECK(warrior.className == "Guerrier");
    CHECK(warrior.maxHp == 125);
    CHECK(warrior.baseDefense == 7);
    CHECK(warrior.weapon.name == "Lame de Feu");
    CHECK(warrior.armor.name == "Manteau de Feu");
    CHECK(warrior.inventory.size() == 2);

    const Player trickster = createHero("Pile", 10, Element::Ombre);
    CHECK(trickster.className == "Pile Face");
    CHECK(trickster.maxHp == 100);
    CHECK(trickster.baseDefense == 3);
    CHECK(trickster.chance == 35);
}

void sharedCombatRulesRetainElementAndEquipmentDamageMath() {
    const Player hero = createHero("Ada", 1, Element::Feu);
    const Enemy plantEnemy{"Épine", "Bête", 40, 40, 12, 3, 0, 0, 0, Element::Plante, false};
    CHECK(CombatRules::playerDamage(hero, plantEnemy, false, -3, false) == 28);
    CHECK(CombatRules::playerDamage(hero, plantEnemy, true, 2, false) == 64);

    const Enemy waterEnemy{"Noyé", "Spectre", 30, 30, 20, 1, 0, 0, 0, Element::Eau, false};
    CHECK(CombatRules::enemyDamage(waterEnemy, hero, 4, false) == 17);
}

void anEnemyThatKillsTheLastHeroBecomesAPersistentNemesis() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 99));
    run.team[0].hp = 1;
    Encounter encounter{{"Briseur", "Berserker", 80, 80, 200, 0, 0, 0, 0, Element::Ombre, false}, false, ""};
    std::mt19937 rng(42);

    const CombatTurnResult result = resolveCombatAction(run, encounter, 0, CombatAction::Potion, rng);
    CHECK(result.heroDied);
    CHECK(result.teamDefeated);
    CHECK(!run.active);
    CHECK(run.nemeses.size() == 1);
    CHECK(run.nemeses[0].enemy.attack == 202);
    CHECK(run.nemeses[0].enemy.defense == 2);
    CHECK(run.nemeses[0].stolenEquipment.size() == 2);
}

void fourClearedRoomsLeadToTheBossAndBankTheBossReward() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 777));

    for (int room = 0; room < 4; ++room) {
        Encounter encounter = nextEncounter(run);
        CHECK(!encounter.boss);
        encounter.enemy.hp = 0;
        CHECK(resolveEncounterVictory(profile, run, encounter));
    }
    CHECK(run.completedRooms == 4);
    CHECK(profile.fragments == 4);

    Encounter boss = nextEncounter(run);
    CHECK(boss.boss);
    boss.enemy.hp = 0;
    CHECK(resolveEncounterVictory(profile, run, boss));
    CHECK(!run.active);
    CHECK(run.won);
    CHECK(profile.fragments == 8);
}

void aClearedRoomKeepsItsRewardPendingUntilThePlayerChooses() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 101));
    run.team[0].hp = 50;
    Encounter encounter = nextEncounter(run);
    encounter.enemy.xp = 0;
    encounter.enemy.gold = 0;
    encounter.enemy.hp = 0;

    CHECK(resolveEncounterVictory(profile, run, encounter));
    CHECK(run.pendingReward);
    CHECK(claimRoomReward(run, 0, RoomReward::Rest));
    CHECK(!run.pendingReward);
    CHECK(run.team[0].hp == 75);
}

void aDefeatedNemesisReturnsItsStolenEquipmentToTheSurvivor() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 202));
    const Enemy original{"Garde", "Guerrier", 50, 50, 10, 2, 5, 0, 0, Element::Feu, false};
    Player victim = run.team[0];
    run.nemeses.push_back(createNemesisFromKill(original, victim, "rival-revanche"));
    Encounter encounter{run.nemeses[0].enemy, false, "rival-revanche"};
    encounter.enemy.hp = 0;

    CHECK(resolveEncounterVictory(profile, run, encounter));
    CHECK(run.nemeses.empty());
    const std::vector<Item>& inventory = run.team[0].inventory;
    CHECK(std::any_of(inventory.begin(), inventory.end(), [](const Item& item) { return item.name == "Lame de Feu"; }));
    CHECK(std::any_of(inventory.begin(), inventory.end(), [](const Item& item) { return item.name == "Manteau de Feu"; }));
}

void theSameRunSeedAndRoomChooseTheSameEncounter() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun firstRun;
    RogueliteRun secondRun;
    CHECK(startRogueliteRun(profile, firstRun, 314159));
    CHECK(startRogueliteRun(profile, secondRun, 314159));
    firstRun.completedRooms = 2;
    secondRun.completedRooms = 2;

    const Encounter first = nextEncounter(firstRun);
    const Encounter second = nextEncounter(secondRun);
    CHECK(first.enemy.name == second.enemy.name);
    CHECK(first.enemy.hp == second.enemy.hp);
    CHECK(first.enemy.attack == second.enemy.attack);
}

void openingEncounterIsAccessibleAndEachRoomRaisesThePowerBand() {
    RogueliteRun run;
    run.seed = 2090735165U;
    Encounter opening = nextEncounter(run);
    CHECK(!opening.boss);
    CHECK(opening.enemy.maxHp <= 170);
    CHECK(opening.enemy.attack <= 25);
    CHECK(opening.enemy.defense <= 12);

    int previousPower = 0;
    for (int room = 0; room < 4; ++room) {
        run.completedRooms = room;
        const Encounter encounter = nextEncounter(run);
        const Enemy& enemy = encounter.enemy;
        const int power = enemy.maxHp + enemy.attack * 3 + enemy.defense * 2;
        CHECK(power > previousPower);
        previousPower = power;
    }
}

void rogueliteSavesRestoreMetaAndRunWhileKeepingCampaignV3Readable() {
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "chroniques_roguelite_tests";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    const std::string profilePath = (folder / "profile.save").string();
    const std::string runPath = (folder / "run.save").string();

    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    profile.fragments = 10;
    CHECK(buyPermanentUpgrade(profile, 0, PermanentStat::MaxHp));
    CHECK(RogueliteSaveSystem::saveProfile(profile, profilePath));
    RogueliteProfile loadedProfile;
    CHECK(RogueliteSaveSystem::loadProfile(loadedProfile, profilePath));
    CHECK(loadedProfile.fragments == 6);
    CHECK(loadedProfile.roster[0].maxHp == 135);
    CHECK(loadedProfile.upgradeRanks[0].maxHp == 1);

    RogueliteRun run;
    CHECK(startRogueliteRun(loadedProfile, run, 4321));
    run.completedRooms = 2;
    run.pendingReward = true;
    Player victim = run.team[0];
    run.nemeses.push_back(createNemesisFromKill(
        {"Garde", "Guerrier", 50, 50, 10, 2, 5, 20, 10, Element::Feu, false}, victim, "rival-test"));
    CHECK(RogueliteSaveSystem::saveRun(run, runPath));
    RogueliteRun loadedRun;
    CHECK(RogueliteSaveSystem::loadRun(loadedRun, runPath));
    CHECK(loadedRun.seed == 4321);
    CHECK(loadedRun.completedRooms == 2);
    CHECK(loadedRun.pendingReward);
    CHECK(loadedRun.nemeses.size() == 1);
    CHECK(loadedRun.nemeses[0].id == "rival-test");
    CHECK(loadedRun.nemeses[0].stolenEquipment.size() == 2);

    const std::string corruptPath = (folder / "corrupt.save").string();
    {
        std::ofstream corrupt(corruptPath);
        corrupt << "NOT_A_ROGUELITE_SAVE\n";
    }
    loadedProfile.fragments = 19;
    CHECK(!RogueliteSaveSystem::loadProfile(loadedProfile, corruptPath));
    CHECK(loadedProfile.fragments == 19);

    std::vector<Player> campaignTeam;
    Progress campaignProgress;
    CHECK(SaveSystem::load(campaignTeam, campaignProgress, "sauvegarde/savegame.txt"));
    CHECK(campaignTeam.size() == 3);
    std::filesystem::remove_all(folder);
}
}

int main() {
    permanentUpgradesCostMoreAndPersistInTheNextRun();
    completedRoomsBankFragmentsEvenWhenTheRunLaterFails();
    aNemesisStealsAndAccumulatesTheEquipmentAndStatsOfItsVictims();
    heroFactoryPreservesTheExistingClassAndElementDefaults();
    sharedCombatRulesRetainElementAndEquipmentDamageMath();
    anEnemyThatKillsTheLastHeroBecomesAPersistentNemesis();
    fourClearedRoomsLeadToTheBossAndBankTheBossReward();
    aClearedRoomKeepsItsRewardPendingUntilThePlayerChooses();
    aDefeatedNemesisReturnsItsStolenEquipmentToTheSurvivor();
    theSameRunSeedAndRoomChooseTheSameEncounter();
    openingEncounterIsAccessibleAndEachRoomRaisesThePowerBand();
    rogueliteSavesRestoreMetaAndRunWhileKeepingCampaignV3Readable();
    if (failures != 0) {
        std::cerr << failures << " vérification(s) en échec.\n";
        return 1;
    }
    std::cout << "Tous les tests roguelite sont passés.\n";
    return 0;
}
