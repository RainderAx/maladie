#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "core/HeroFactory.hpp"
#include "core/Terminal.hpp"
#include "systems/ArenaSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/CombatRules.hpp"
#include "systems/Game.hpp"
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
    CHECK(trickster.chance == 10);
    CHECK(trickster.criticalChance() == 50);
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
    CHECK(profile.fragments >= 4);

    const int fragmentsBeforeBoss = profile.fragments;
    Encounter boss = nextEncounter(run);
    CHECK(boss.boss);
    boss.enemy.hp = 0;
    CHECK(resolveEncounterVictory(profile, run, boss));
    CHECK(!run.active);
    CHECK(run.won);
    CHECK(profile.fragments > fragmentsBeforeBoss);
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
    CHECK(SaveSystem::load(campaignTeam, campaignProgress, "tests/fixtures/campaign_v3.txt"));
    CHECK(campaignTeam.size() == 3);
    if (!campaignTeam.empty()) CHECK(campaignTeam[0].name == "Ada");
    std::filesystem::remove_all(folder);
}

void deadHeroesStayAtZeroHpAndCannotBeRevivedByExperience() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 808));
    run.team[0].hp = 1;
    run.team[0].inventory.push_back({"Potion", "Soin de test", 0, 0, 45, 0, false, Element::Eau});
    Encounter encounter{{"Briseur", "Berserker", 1000, 1000, 500, 0, 0, 0, 0, Element::Ombre, false}, false, {}};
    std::mt19937 rng(42);

    const CombatTurnResult result = resolveCombatAction(run, encounter, 0, CombatAction::Potion, rng);
    CHECK(result.heroDied);
    CHECK(run.team[0].hp == 0);
    run.team[0].gainXp(1000);
    CHECK(run.team[0].hp == 0);
    CHECK(run.team[0].level == 1);
}

void emptyEquipmentDoesNotGrantElementalBonuses() {
    Player empty = createHero("Ada", 1, Element::Feu);
    empty.weapon = {"", "", 0, 0, 0, 0, false, Element::Feu};
    empty.armor = {"", "", 0, 0, 0, 0, false, Element::Feu};
    Player nonmatching = empty;
    nonmatching.weapon.element = Element::Ombre;
    nonmatching.armor.element = Element::Ombre;

    CHECK(empty.matchingEquipmentBonus() == 0);
    CHECK(empty.attack() == nonmatching.attack());
    CHECK(empty.defense() == nonmatching.defense());
    CHECK(empty.criticalChance() == nonmatching.criticalChance());
}

void nemesisPromotionPreservesItsCurrentEncounterHp() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 809));
    Player previousVictim = createHero("Brume", 1, Element::Eau);
    run.nemeses.push_back(createNemesisFromKill(
        {"Rival", "Guerrier", 100, 100, 500, 0, 0, 0, 0, Element::Ombre, false}, previousVictim, "rival-hp"));
    run.team[0].hp = 1;
    run.team[0].inventory.push_back({"Potion", "Soin de test", 0, 0, 45, 0, false, Element::Eau});
    Encounter encounter{run.nemeses[0].enemy, false, "rival-hp"};
    encounter.enemy.hp = 20;
    std::mt19937 rng(43);

    const CombatTurnResult result = resolveCombatAction(run, encounter, 0, CombatAction::Potion, rng);
    CHECK(result.heroDied);
    CHECK(encounter.enemy.hp == 20);
    CHECK(run.nemeses[0].enemy.hp == 20);
}

void saveRunRejectsNegativeHeroHpWithoutTruncatingTheExistingFile() {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "chroniques_invalid_run.save";
    const std::filesystem::path profilePath = std::filesystem::temp_directory_path() / "chroniques_invalid_profile.save";
    {
        std::ofstream existing(path);
        existing << "existing checkpoint\n";
    }
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 810));
    run.team[0].hp = -1;

    CHECK(!RogueliteSaveSystem::saveRun(run, path.string()));
    std::ifstream saved(path);
    std::string marker;
    std::getline(saved, marker);
    CHECK(marker == "existing checkpoint");
    std::filesystem::remove(path);

    {
        std::ofstream existing(profilePath);
        existing << "existing profile\n";
    }
    RogueliteProfile invalidProfile;
    initializeRogueliteProfile(invalidProfile, {createHero("Ada", 1, Element::Feu)});
    invalidProfile.upgradeRanks[0].attack = 9;
    CHECK(!RogueliteSaveSystem::saveProfile(invalidProfile, profilePath.string()));
    std::ifstream savedProfile(profilePath);
    std::getline(savedProfile, marker);
    CHECK(marker == "existing profile");
    std::filesystem::remove(profilePath);
}

void fragmentRewardScalesWithEnemyDangerAndFlawlessVictory() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 811));
    Encounter encounter{{"Garde", "Guerrier", 100, 100, 10, 1, 0, 250, 0, Element::Ombre, false}, false, {}};
    encounter.enemy.hp = 0;

    CHECK(resolveEncounterVictory(profile, run, encounter));
    CHECK(profile.fragments == 4);
}

void fragmentRewardsIncludeNemesisBossAndCasualtyRules() {
    RogueliteProfile woundedProfile;
    initializeRogueliteProfile(woundedProfile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun woundedRun;
    CHECK(startRogueliteRun(woundedProfile, woundedRun, 812));
    woundedRun.roomDeaths = 1;
    Encounter wounded{{"Garde", "Guerrier", 100, 100, 10, 1, 0, 100, 0, Element::Ombre, false}, false, {}};
    wounded.enemy.hp = 0;
    CHECK(resolveEncounterVictory(woundedProfile, woundedRun, wounded));
    CHECK(woundedProfile.fragments == 2);

    RogueliteProfile rivalProfile;
    initializeRogueliteProfile(rivalProfile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun rivalRun;
    CHECK(startRogueliteRun(rivalProfile, rivalRun, 813));
    Encounter rivalEncounter{{"Rival", "Guerrier", 100, 100, 10, 1, 0, 100, 0, Element::Ombre, false}, false, "score-rival"};
    rivalEncounter.enemy.hp = 0;
    rivalRun.nemeses.push_back({"score-rival", rivalEncounter.enemy, {}});
    CHECK(resolveEncounterVictory(rivalProfile, rivalRun, rivalEncounter));
    CHECK(rivalProfile.fragments == 4);

    RogueliteProfile bossProfile;
    initializeRogueliteProfile(bossProfile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun bossRun;
    CHECK(startRogueliteRun(bossProfile, bossRun, 814));
    bossRun.completedRooms = 4;
    Encounter boss{{"Boss", "Boss", 600, 600, 20, 5, 10, 600, 0, Element::Ombre, true}, true, {}};
    boss.enemy.hp = 0;
    CHECK(resolveEncounterVictory(bossProfile, bossRun, boss));
    CHECK(bossProfile.fragments == 7);
}

void purchasedEquipmentCanBeAssignedWithoutDestroyingTheDisplacedWeapon() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    profile.fragments = 6;

    CHECK(buyRogueliteEquipment(profile, 0));
    CHECK(profile.fragments == 0);
    CHECK(profile.armory.size() == 1);
    CHECK(equipFromArmory(profile, 0, 0, EquipmentSlot::Weapon));
    CHECK(profile.roster[0].weapon.name == "Épée turquoise");
    CHECK(profile.armory.size() == 1);
    CHECK(profile.armory[0].name == "Lame de Feu");

    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 815));
    CHECK(run.team[0].weapon.name == "Épée turquoise");
}

void inRunForgeAndSingleItemExtractionRetainTheReplacedGear() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 816));
    run.team[0].gold = 300;
    run.team[0].inventory.push_back({"Lame de butin", "ATQ +5", 5, 0, 0, 0, true, Element::Eau});
    Encounter encounter{{"Garde", "Guerrier", 10, 10, 1, 0, 0, 0, 0, Element::Ombre, false}, false, {}};
    encounter.enemy.hp = 0;
    CHECK(resolveEncounterVictory(profile, run, encounter));
    CHECK(equipRunItem(run, 0, 2));
    CHECK(run.team[0].weapon.name == "Lame de butin");
    CHECK(std::any_of(run.team[0].inventory.begin(), run.team[0].inventory.end(),
        [](const Item& item) { return item.name == "Lame de Feu"; }));

    const int initialAttack = run.team[0].weapon.attack;
    CHECK(upgradeRunEquipment(run, 0, EquipmentSlot::Weapon));
    CHECK(run.team[0].weapon.attack == initialAttack + 2);
    CHECK(run.team[0].weapon.upgradeLevel == 1);
    CHECK(run.team[0].gold == 255);

    const auto loot = std::find_if(run.team[0].inventory.begin(), run.team[0].inventory.end(),
        [](const Item& item) { return item.name == "Lame de Feu"; });
    CHECK(loot != run.team[0].inventory.end());
    const std::size_t lootIndex = static_cast<std::size_t>(loot - run.team[0].inventory.begin());
    CHECK(extractRunLoot(profile, run, 0, lootIndex));
    CHECK(profile.armory.size() == 1);
    CHECK(profile.armory[0].name == "Lame de Feu");
    CHECK(!extractRunLoot(profile, run, 0, 0));
}

void aBossVictoryKeepsTheRunEquipmentAsTheNextRosterLoadout() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 817));
    run.team[0].weapon.attack += 4;
    Encounter boss{{"Boss", "Boss", 10, 10, 1, 0, 0, 0, 0, Element::Ombre, true}, true, {}};
    boss.enemy.hp = 0;

    CHECK(resolveEncounterVictory(profile, run, boss));
    CHECK(profile.roster[0].weapon.attack == run.team[0].weapon.attack);
}

void rogueliteV2RoundTripsArmoryUpgradesAndExtractionState() {
    const auto folder = std::filesystem::temp_directory_path() / "chroniques_roguelite_v2";
    std::filesystem::create_directories(folder);
    const auto profilePath = folder / "profile.save";
    const auto runPath = folder / "run.save";
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    profile.fragments = 17;
    profile.roster[0].weapon.attack += 4;
    profile.roster[0].weapon.upgradeLevel = 2;
    Item stored{"Lame récupérée", "ATK +5", 5, 0, 0, 0, true, Element::Eau};
    stored.upgradeLevel = 1;
    profile.armory.push_back(stored);

    CHECK(RogueliteSaveSystem::saveProfile(profile, profilePath.string()));
    RogueliteProfile loadedProfile;
    CHECK(RogueliteSaveSystem::loadProfile(loadedProfile, profilePath.string()));
    CHECK(loadedProfile.fragments == 17);
    CHECK(loadedProfile.armory.size() == 1);
    if (!loadedProfile.armory.empty()) {
        CHECK(loadedProfile.armory[0].name == "Lame récupérée");
        CHECK(loadedProfile.armory[0].upgradeLevel == 1);
    }
    CHECK(loadedProfile.roster[0].weapon.upgradeLevel == 2);

    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 818));
    run.pendingReward = true;
    run.roomLootExtracted = true;
    run.team[0].weapon.upgradeLevel = 2;
    CHECK(RogueliteSaveSystem::saveRun(run, runPath.string()));
    RogueliteRun loadedRun;
    CHECK(RogueliteSaveSystem::loadRun(loadedRun, runPath.string()));
    CHECK(loadedRun.team[0].weapon.upgradeLevel == 2);
    CHECK(loadedRun.roomLootExtracted);

    std::ifstream profileFile(profilePath);
    std::string marker;
    std::getline(profileFile, marker);
    CHECK(marker == "ROGUELITE_PROFILE_V2");
    std::ifstream runFile(runPath);
    std::getline(runFile, marker);
    CHECK(marker == "ROGUELITE_RUN_V2");
    std::filesystem::remove_all(folder);
}

void rogueliteV1ProfileAndRunFixturesStillLoadWithDefaultNewFields() {
    const auto folder = std::filesystem::temp_directory_path() / "chroniques_roguelite_v1_fixture";
    std::filesystem::create_directories(folder);
    const auto profilePath = folder / "profile.save";
    const auto runPath = folder / "run.save";
    const std::string legacyPlayer =
        "1\n\"Ada\" \"Guerrier\" 1 1\n1 0 80 125 125 8 7 0\n"
        "\"Lame de Feu\" \"Arme accordée\" 2 0 0 0 1 1\n"
        "\"Manteau de Feu\" \"Armure accordée\" 0 2 0 0 1 1\n0\n";
    {
        std::ofstream profileFile(profilePath);
        profileFile << "ROGUELITE_PROFILE_V1\n" << legacyPlayer << "0 0 0\n12\n";
        std::ofstream runFile(runPath);
        runFile << "ROGUELITE_RUN_V1\n" << legacyPlayer << "123 1 1 0 0\n0\n";
    }

    RogueliteProfile profile;
    CHECK(RogueliteSaveSystem::loadProfile(profile, profilePath.string()));
    CHECK(profile.fragments == 12);
    CHECK(profile.armory.empty());
    CHECK(profile.roster[0].weapon.upgradeLevel == 0);
    RogueliteRun run;
    CHECK(RogueliteSaveSystem::loadRun(run, runPath.string()));
    CHECK(run.completedRooms == 1);
    CHECK(run.roomDeaths == 0);
    CHECK(!run.roomLootExtracted);
    CHECK(run.team[0].weapon.upgradeLevel == 0);
    std::filesystem::remove_all(folder);
}

void extractionQuotaResetsAtTheNextRoomCheckpoint() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 819));
    run.team[0].inventory.push_back({"Butin 1", "ATK +3", 3, 0, 0, 0, true, Element::Eau});
    run.team[0].inventory.push_back({"Butin 2", "DEF +3", 0, 3, 0, 0, true, Element::Plante});
    Encounter first{{"Garde", "Guerrier", 10, 10, 1, 0, 0, 0, 0, Element::Ombre, false}, false, {}};
    first.enemy.hp = 0;
    CHECK(resolveEncounterVictory(profile, run, first));
    CHECK(extractRunLoot(profile, run, 0, 2));
    CHECK(run.roomLootExtracted);
    CHECK(claimRoomReward(run, 0, RoomReward::Rest));

    Encounter second{{"Sentinelle", "Paladin", 10, 10, 1, 0, 0, 0, 0, Element::Ombre, false}, false, {}};
    second.enemy.hp = 0;
    CHECK(resolveEncounterVictory(profile, run, second));
    CHECK(!run.roomLootExtracted);
    const auto loot = std::find_if(run.team[0].inventory.begin(), run.team[0].inventory.end(),
        [](const Item& item) { return item.name == "Butin 2"; });
    CHECK(loot != run.team[0].inventory.end());
    if (loot != run.team[0].inventory.end()) {
        const std::size_t index = static_cast<std::size_t>(loot - run.team[0].inventory.begin());
        CHECK(extractRunLoot(profile, run, 0, index));
    }
    CHECK(profile.armory.size() == 2);
}

void aDeadHeroCannotEquipLootAtTheCheckpoint() {
    RogueliteProfile profile;
    initializeRogueliteProfile(profile, {createHero("Ada", 1, Element::Feu)});
    RogueliteRun run;
    CHECK(startRogueliteRun(profile, run, 820));
    run.pendingReward = true;
    run.team[0].hp = 0;
    run.team[0].inventory.push_back({"Lame", "ATK +3", 3, 0, 0, 0, true, Element::Eau});

    CHECK(!equipRunItem(run, 0, 2));
    CHECK(run.team[0].weapon.name == "Lame de Feu");
}

void terminalAskReportsEndOfInputInsteadOfRetryingForever() {
    std::istringstream emptyInput;
    std::streambuf* previousBuffer = std::cin.rdbuf();
    const auto previousExceptions = std::cin.exceptions();
    std::cin.exceptions(std::ios::goodbit);
    std::cin.rdbuf(emptyInput.rdbuf());
    std::cin.exceptions(std::ios::failbit | std::ios::eofbit);
    bool reportedEndOfInput = false;
    try {
        Terminal::ask(1, 3);
    } catch (const std::runtime_error& error) {
        reportedEndOfInput = std::string(error.what()) == "Fin de l'entrée standard.";
    } catch (...) {
    }
    std::cin.exceptions(std::ios::goodbit);
    std::cin.clear();
    std::cin.rdbuf(previousBuffer);
    std::cin.exceptions(previousExceptions);
    CHECK(reportedEndOfInput);
}

void campaignSaveAndLoadRejectTeamsShorterThanThreeHeroes() {
    const auto folder = std::filesystem::temp_directory_path() / "chroniques_short_campaign_team";
    std::filesystem::create_directories(folder);
    const auto shortSavePath = folder / "short.save";
    const auto fixturePath = folder / "valid-but-short-v3.save";
    std::vector<Player> shortTeam{
        createHero("Ada", 1, Element::Feu), createHero("Brume", 2, Element::Eau)
    };
    Progress progress;
    CHECK(!SaveSystem::save(shortTeam, progress, shortSavePath.string()));
    {
        std::ofstream fixture(fixturePath);
        fixture << "CHRONIQUES_SAVE_V3\n2\n"
                << "\"Ada\" \"Guerrier\" 1 1 1 0 80 125 125 8 7 0\n"
                << "\"Lame de Feu\" \"Arme accordée\" 2 0 0 0 1 1\n"
                << "\"Manteau de Feu\" \"Armure accordée\" 0 2 0 0 1 1\n0\n"
                << "\"Brume\" \"Berserker\" 2 0 1 0 80 145 145 11 4 0\n"
                << "\"Lame des Eaux\" \"Arme accordée\" 2 0 0 0 1 0\n"
                << "\"Manteau des Eaux\" \"Armure accordée\" 0 2 0 0 1 0\n0\n"
                << "0\n0 0 0 0 0\n";
    }
    std::vector<Player> loadedTeam{createHero("Conservé", 1, Element::Feu)};
    Progress loadedProgress;
    CHECK(!SaveSystem::load(loadedTeam, loadedProgress, fixturePath.string()));
    CHECK(loadedTeam.size() == 1);
    CHECK(loadedTeam[0].name == "Conservé");
    std::filesystem::remove_all(folder);
}

void campaignEquipmentReplacementReturnsTheOldPieceToTheBag() {
    Player hero = createHero("Ada", 1, Element::Feu);
    hero.inventory.clear();
    hero.weapon = {"Ancienne lame", "ATK +2", 2, 0, 0, 0, true, Element::Feu};
    hero.inventory.push_back({"Nouvelle lame", "ATK +9", 9, 0, 0, 0, true, Element::Eau});

    CHECK(hero.equipItemFromInventory(0));
    CHECK(hero.weapon.name == "Nouvelle lame");
    CHECK(hero.inventory.size() == 1);
    if (!hero.inventory.empty()) CHECK(hero.inventory[0].name == "Ancienne lame");
}

void campaignInventoryMenuPreservesTheReplacedWeaponAcrossSaveAndReload() {
    const auto previousDirectory = std::filesystem::current_path();
    const auto folder = std::filesystem::temp_directory_path() / "chroniques_campaign_inventory_round_trip";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "sauvegarde");
    std::filesystem::current_path(folder);

    std::vector<Player> team{
        createHero("Aube", 1, Element::Eau), createHero("Brume", 4, Element::Feu),
        createHero("Cendre", 7, Element::Ombre)
    };
    team[0].weapon = {"Ancienne lame", "ATK +2", 2, 0, 0, 0, true, Element::Eau};
    team[0].inventory.clear();
    team[0].inventory.push_back({"Nouvelle lame", "ATK +9", 9, 0, 0, 0, true, Element::Feu});
    Progress progress;
    CHECK(SaveSystem::save(team, progress, "sauvegarde/savegame.txt"));

    std::istringstream input("1\n3\n1\n1\n0\n0\n");
    std::streambuf* previousInput = std::cin.rdbuf(input.rdbuf());
    std::ostringstream output;
    std::streambuf* previousOutput = std::cout.rdbuf(output.rdbuf());
    Game game;
    game.run();
    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);

    std::vector<Player> loadedTeam;
    Progress loadedProgress;
    CHECK(SaveSystem::load(loadedTeam, loadedProgress, "sauvegarde/savegame.txt"));
    CHECK(loadedTeam.size() == 3);
    if (!loadedTeam.empty()) {
        CHECK(loadedTeam[0].weapon.name == "Nouvelle lame");
        CHECK(loadedTeam[0].inventory.size() == 1);
        if (!loadedTeam[0].inventory.empty()) CHECK(loadedTeam[0].inventory[0].name == "Ancienne lame");
    }
    std::filesystem::current_path(previousDirectory);
    std::filesystem::remove_all(folder);
}

void aWipedCampaignOffersRestartInsteadOfSilentlyReturningToTheHub() {
    const auto previousDirectory = std::filesystem::current_path();
    const auto folder = std::filesystem::temp_directory_path() / "chroniques_campaign_wipe_resume";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "sauvegarde");
    std::filesystem::current_path(folder);

    std::vector<Player> defeatedTeam{
        createHero("Aube", 1, Element::Eau), createHero("Brume", 4, Element::Feu),
        createHero("Cendre", 7, Element::Ombre)
    };
    for (Player& hero : defeatedTeam) hero.hp = 0;
    Progress progress;
    CHECK(SaveSystem::save(defeatedTeam, progress, "sauvegarde/savegame.txt"));

    std::istringstream input("1\n0\n");
    std::streambuf* previousInput = std::cin.rdbuf(input.rdbuf());
    std::ostringstream output;
    std::streambuf* previousOutput = std::cout.rdbuf(output.rdbuf());
    Game game;
    game.run();
    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);

    CHECK(output.str().find("Aucun survivant") != std::string::npos);
    std::vector<Player> loadedTeam;
    Progress loadedProgress;
    CHECK(SaveSystem::load(loadedTeam, loadedProgress, "sauvegarde/savegame.txt"));
    CHECK(loadedTeam.size() == 3);
    if (!loadedTeam.empty()) CHECK(loadedTeam[0].hp == 0);

    std::filesystem::current_path(previousDirectory);
    std::filesystem::remove_all(folder);
}

void campaignQuestProgressOccursOnceOnlyAfterVictory() {
    Player hero = createHero("Ada", 1, Element::Feu);
    hero.baseAttack = 1000;
    hero.quests.push_back({"Premier éclat", "vaincre", "Récompense", 3, 0, 0, 0, false, false});
    std::vector<Player> team{hero};
    Enemy enemy{"Cible", "Bête", 10, 10, 0, 0, 0, 0, 0, Element::Ombre, false};
    std::istringstream input("1\n1\n");
    std::streambuf* previousInput = std::cin.rdbuf(input.rdbuf());
    std::ostringstream output;
    std::streambuf* previousOutput = std::cout.rdbuf(output.rdbuf());

    const bool victory = combat(team, enemy);

    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);
    CHECK(victory);
    CHECK(team[0].quests[0].progress == 1);
    CHECK(!team[0].quests[0].completed);
}

void aNonlethalCampaignStrikeDoesNotAdvanceTheVictoryQuest() {
    Player hero = createHero("Ada", 1, Element::Feu);
    hero.quests.push_back({"Premier éclat", "vaincre", "Récompense", 1, 0, 0, 0, false, false});
    std::vector<Player> team{hero};
    Enemy enemy{"Mur", "Bête", 100000, 100000, 0, 100000, 0, 0, 0, Element::Ombre, false};
    std::istringstream input("1\n1\n");
    std::streambuf* previousInput = std::cin.rdbuf(input.rdbuf());
    std::ostringstream output;
    std::streambuf* previousOutput = std::cout.rdbuf(output.rdbuf());
    bool endedAtInput = false;
    try {
        combat(team, enemy);
    } catch (const Terminal::EndOfInput&) {
        endedAtInput = true;
    }
    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);
    CHECK(endedAtInput);
    CHECK(team[0].quests[0].progress == 0);
}

void aBossDoesNotTreatTheFleeChoiceAsAnAttack() {
    std::vector<Player> team{createHero("Ada", 1, Element::Feu)};
    Enemy boss{"Gardien", "Boss", 1000, 1000, 0, 0, 0, 0, 0, Element::Ombre, true};
    std::istringstream input("1\n4\n");
    std::streambuf* previousInput = std::cin.rdbuf(input.rdbuf());
    std::ostringstream output;
    std::streambuf* previousOutput = std::cout.rdbuf(output.rdbuf());
    try {
        combat(team, boss);
    } catch (const Terminal::EndOfInput&) {
    }
    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);
    CHECK(output.str().find(" inflige ") == std::string::npos);
}

void arenaEnemyPoolUnlocksFiveNewOpponentsPerWaveAndStopsAtTwenty() {
    CHECK(arenaRosterForWave(1).size() == 5);
    CHECK(arenaRosterForWave(2).size() == 10);
    CHECK(arenaRosterForWave(4).size() == 20);
    CHECK(arenaRosterForWave(99).size() == 20);
    CHECK(arenaRosterForWave(1)[0].name == "Garde de Fer");
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
    deadHeroesStayAtZeroHpAndCannotBeRevivedByExperience();
    emptyEquipmentDoesNotGrantElementalBonuses();
    nemesisPromotionPreservesItsCurrentEncounterHp();
    saveRunRejectsNegativeHeroHpWithoutTruncatingTheExistingFile();
    fragmentRewardScalesWithEnemyDangerAndFlawlessVictory();
    fragmentRewardsIncludeNemesisBossAndCasualtyRules();
    purchasedEquipmentCanBeAssignedWithoutDestroyingTheDisplacedWeapon();
    inRunForgeAndSingleItemExtractionRetainTheReplacedGear();
    aBossVictoryKeepsTheRunEquipmentAsTheNextRosterLoadout();
    rogueliteV2RoundTripsArmoryUpgradesAndExtractionState();
    rogueliteV1ProfileAndRunFixturesStillLoadWithDefaultNewFields();
    extractionQuotaResetsAtTheNextRoomCheckpoint();
    aDeadHeroCannotEquipLootAtTheCheckpoint();
    terminalAskReportsEndOfInputInsteadOfRetryingForever();
    campaignSaveAndLoadRejectTeamsShorterThanThreeHeroes();
    campaignEquipmentReplacementReturnsTheOldPieceToTheBag();
    campaignInventoryMenuPreservesTheReplacedWeaponAcrossSaveAndReload();
    aWipedCampaignOffersRestartInsteadOfSilentlyReturningToTheHub();
    campaignQuestProgressOccursOnceOnlyAfterVictory();
    aNonlethalCampaignStrikeDoesNotAdvanceTheVictoryQuest();
    aBossDoesNotTreatTheFleeChoiceAsAnAttack();
    arenaEnemyPoolUnlocksFiveNewOpponentsPerWaveAndStopsAtTwenty();
    if (failures != 0) {
        std::cerr << failures << " vérification(s) en échec.\n";
        return 1;
    }
    std::cout << "Tous les tests roguelite sont passés.\n";
    return 0;
}
