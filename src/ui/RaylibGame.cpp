#include "../../include/ui/RaylibGame.hpp"
#include "../../include/core/HeroFactory.hpp"
#include "../../include/systems/Game.hpp"
#include "../../include/systems/RogueliteSaveSystem.hpp"
#include "../../include/systems/RogueliteSystem.hpp"
#include <raylib.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace {
constexpr int windowWidth = 1280;
constexpr int windowHeight = 760;
constexpr Color ink{12, 16, 25, 255};
constexpr Color panel{22, 29, 42, 245};
constexpr Color panelLight{31, 41, 58, 255};
constexpr Color gold{220, 177, 94, 255};
constexpr Color pale{233, 229, 216, 255};
constexpr Color muted{149, 160, 178, 255};
constexpr Color teal{75, 181, 169, 255};
constexpr Color danger{193, 84, 83, 255};
constexpr Color violet{127, 105, 180, 255};

void centeredText(const std::string& text, int x, int y, int width, int size, Color color) {
    const int textWidth = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), x + std::max(0, (width - textWidth) / 2), y, size, color);
}

void panelBox(Rectangle area, Color border = {55, 69, 89, 255}) {
    DrawRectangleRounded(area, 0.08f, 8, panel);
    DrawRectangleRoundedLinesEx(area, 0.08f, 8, 1.0f, border);
}

void progressBar(float x, float y, float width, float height, int value, int maximum, Color fill) {
    DrawRectangleRounded({x, y, width, height}, 0.5f, 8, {45, 53, 66, 255});
    if (maximum <= 0) return;
    const float fraction = std::clamp(static_cast<float>(value) / static_cast<float>(maximum), 0.0f, 1.0f);
    if (fraction > 0.0f) DrawRectangleRounded({x, y, width * fraction, height}, 0.5f, 8, fill);
}

bool gameButton(float x, float y, float width, float height, const std::string& text,
                bool enabled = true, bool emphasized = false) {
    const Rectangle bounds{x, y, width, height};
    const bool hovered = enabled && CheckCollisionPointRec(GetMousePosition(), bounds);
    const Color base = !enabled ? Color{36, 42, 52, 255} : emphasized ? Color{95, 70, 43, 255} : panelLight;
    DrawRectangleRounded(bounds, 0.16f, 8, hovered ? Color{48, 61, 82, 255} : base);
    DrawRectangleRoundedLinesEx(bounds, 0.16f, 8, hovered ? 2.0f : 1.0f, hovered ? gold : Color{65, 78, 98, 255});
    const int fontSize = text.size() > 20 ? 15 : 20;
    centeredText(text, static_cast<int>(x), static_cast<int>(y + (height - fontSize) / 2), static_cast<int>(width), fontSize,
                 enabled ? pale : muted);
    return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void drawHeroPortrait(float x, float y, const Player& hero, bool selected = false) {
    DrawCircle(static_cast<int>(x + 30), static_cast<int>(y + 30), 26.0f, selected ? gold : Color{74, 92, 119, 255});
    DrawCircle(static_cast<int>(x + 30), static_cast<int>(y + 30), 21.0f, Color{31, 39, 55, 255});
    const std::string initial = hero.name.empty() ? "?" : hero.name.substr(0, 1);
    centeredText(initial, static_cast<int>(x + 8), static_cast<int>(y + 19), 44, 22, pale);
}

class RaylibApplication {
public:
    int run() {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        InitWindow(windowWidth, windowHeight, "Chroniques de la Faille — Roguelite");
        if (!IsWindowReady()) return 1;
        SetWindowMinSize(960, 760);
        SetTargetFPS(60);
        SetExitKey(KEY_NULL);
        loadState();

        while (!WindowShouldClose() && !quit_) {
            if (IsKeyPressed(KEY_ESCAPE)) handleEscape();
            BeginDrawing();
            drawBackdrop();
            drawScene();
            EndDrawing();
            if (launchTerminal_) {
                persistCheckpoint();
                CloseWindow();
                Game classicGame;
                classicGame.run();
                return 0;
            }
        }

        persistCheckpoint();
        CloseWindow();
        return 0;
    }

private:
    enum class Scene { Menu, Roster, Workshop, Map, Combat, Reward, Summary };

    Scene scene_ = Scene::Menu;
    RogueliteProfile profile_;
    RogueliteRun run_;
    RogueliteRun roomCheckpoint_;
    Encounter encounter_{};
    std::mt19937 rng_{std::random_device{}()};
    bool hasProfile_ = false;
    bool hasRun_ = false;
    bool quit_ = false;
    bool launchTerminal_ = false;
    bool confirmNewRun_ = false;
    int selectedHero_ = 0;
    int editingName_ = -1;
    std::array<std::string, 3> draftNames_{{"Aube", "Brume", "Cendre"}};
    std::array<int, 3> draftClass_{{1, 4, 7}};
    std::array<int, 3> draftElement_{{0, 1, 3}};
    std::string statusMessage_;
    std::string lastCombatMessage_;

    int screenW() const { return GetScreenWidth(); }
    int screenH() const { return GetScreenHeight(); }

    void loadState() {
        hasProfile_ = RogueliteSaveSystem::loadProfile(profile_);
        if (!hasProfile_) profile_ = RogueliteProfile{};
        hasRun_ = RogueliteSaveSystem::loadRun(run_) && run_.active;
        if (!hasRun_) run_ = RogueliteRun{};
        else selectedHero_ = firstLivingHero();
    }

    void persistCheckpoint() {
        if (!profile_.roster.empty()) {
            if (!RogueliteSaveSystem::saveProfile(profile_)) statusMessage_ = "Impossible d’enregistrer le profil.";
        }
        if (run_.active && scene_ != Scene::Combat) {
            if (!RogueliteSaveSystem::saveRun(run_)) statusMessage_ = "Impossible d’enregistrer la run.";
        }
    }

    void drawBackdrop() const {
        DrawRectangleGradientV(0, 0, screenW(), screenH(), Color{19, 25, 37, 255}, ink);
        for (int i = 0; i < 7; ++i) {
            const float x = static_cast<float>((i * 211 + 120) % std::max(1, screenW()));
            const float y = static_cast<float>((i * 137 + 80) % std::max(1, screenH()));
            DrawCircle(static_cast<int>(x), static_cast<int>(y), 1.5f, Color{114, 127, 150, 85});
        }
        DrawRectangle(0, 0, screenW(), 5, gold);
    }

    void drawHeader(const std::string& title, const std::string& subtitle = {}) const {
        DrawText(title.c_str(), 58, 33, 31, pale);
        if (!subtitle.empty()) DrawText(subtitle.c_str(), 60, 72, 16, muted);
        DrawLine(58, 103, screenW() - 58, 103, Color{63, 76, 96, 255});
    }

    void drawScene() {
        switch (scene_) {
            case Scene::Menu: drawMenu(); break;
            case Scene::Roster: drawRoster(); break;
            case Scene::Workshop: drawWorkshop(); break;
            case Scene::Map: drawMap(); break;
            case Scene::Combat: drawCombat(); break;
            case Scene::Reward: drawReward(); break;
            case Scene::Summary: drawSummary(); break;
        }
    }

    void drawMenu() {
        const int w = screenW();
        DrawCircle(w / 2, 188, 65.0f, Color{50, 42, 58, 255});
        DrawCircleLines(w / 2, 188, 67.0f, gold);
        DrawTriangle({static_cast<float>(w / 2), 143.0f}, {static_cast<float>(w / 2 - 25), 221.0f},
                     {static_cast<float>(w / 2 + 25), 221.0f}, Color{143, 88, 86, 255});
        centeredText("CHRONIQUES DE LA FAILLE", 0, 275, w, 36, pale);
        centeredText("LES RUNS FAÇONNENT LES HÉROS. LES MORTS FAÇONNENT LES RIVAUX.", 0, 321, w, 15, muted);

        const float x = static_cast<float>(w / 2 - 190);
        const float buttonW = 380.0f;
        if (gameButton(x, 385, buttonW, 54, "NOUVELLE EXPÉDITION", true, true)) {
            confirmNewRun_ = run_.active;
            if (!confirmNewRun_) beginNewRunSetup();
        }
        if (hasRun_ && gameButton(x, 451, buttonW, 50, "CONTINUER LA RUN")) {
            scene_ = run_.pendingReward ? Scene::Reward : Scene::Map;
            selectedHero_ = firstLivingHero();
        }
        if (gameButton(x, hasRun_ ? 513 : 451, buttonW, 48, "AVENTURE CLASSIQUE — TERMINAL")) launchTerminal_ = true;
        if (gameButton(x, hasRun_ ? 575 : 513, buttonW, 42, "QUITTER")) quit_ = true;

        if (confirmNewRun_) {
            panelBox({static_cast<float>(w / 2 - 250), 625, 500, 86}, gold);
            centeredText("Abandonner la run en cours et repartir ?", w / 2 - 240, 637, 480, 17, pale);
            if (gameButton(static_cast<float>(w / 2 - 225), 669, 205, 34, "ABANDONNER", true, true)) {
                confirmNewRun_ = false;
                beginNewRunSetup();
            }
            if (gameButton(static_cast<float>(w / 2 + 20), 669, 205, 34, "ANNULER")) confirmNewRun_ = false;
        }

        if (hasProfile_) {
            const std::string bank = "Fragments : " + std::to_string(profile_.fragments) + "   •   Roster : " +
                                     std::to_string(profile_.roster.size()) + " héros";
            centeredText(bank, 0, screenH() - 36, w, 16, gold);
        }
        if (!statusMessage_.empty()) centeredText(statusMessage_, 0, screenH() - 64, w, 15, danger);
    }

    void beginNewRunSetup() {
        confirmNewRun_ = false;
        if (profile_.roster.empty()) {
            scene_ = Scene::Roster;
            return;
        }
        scene_ = Scene::Workshop;
    }

    void drawRoster() {
        drawHeader("Fonder votre roster", "Choisissez les héros qui reviendront entre les runs.");
        const float left = static_cast<float>(screenW() / 2 - 430);
        for (int i = 0; i < 3; ++i) {
            const float y = 130.0f + i * 160.0f;
            panelBox({left, y, 860, 142}, selectedHero_ == i ? gold : Color{55, 69, 89, 255});
            DrawText(TextFormat("HÉROS %d", i + 1), static_cast<int>(left + 22), static_cast<int>(y + 13), 15, gold);
            const Rectangle nameField{left + 22, y + 42, 245, 38};
            DrawRectangleRounded(nameField, 0.12f, 8, editingName_ == i ? Color{47, 57, 75, 255} : Color{28, 36, 51, 255});
            DrawRectangleRoundedLinesEx(nameField, 0.12f, 8, 1.0f, editingName_ == i ? teal : Color{68, 81, 102, 255});
            DrawText(draftNames_[static_cast<std::size_t>(i)].c_str(), static_cast<int>(left + 34), static_cast<int>(y + 52), 18, pale);
            if (CheckCollisionPointRec(GetMousePosition(), nameField) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                editingName_ = i;
                selectedHero_ = i;
            }

            const int classId = draftClass_[static_cast<std::size_t>(i)];
            const std::string classLabel = makeCharacterClass(classId)->name();
            DrawText("CLASSE", static_cast<int>(left + 312), static_cast<int>(y + 38), 13, muted);
            if (gameButton(left + 310, y + 61, 40, 40, "<")) draftClass_[static_cast<std::size_t>(i)] = classId == 1 ? 10 : classId - 1;
            panelBox({left + 356, y + 61, 180, 40});
            centeredText(classLabel, static_cast<int>(left + 359), static_cast<int>(y + 72), 174, 16, pale);
            if (gameButton(left + 542, y + 61, 40, 40, ">")) draftClass_[static_cast<std::size_t>(i)] = classId == 10 ? 1 : classId + 1;

            const int element = draftElement_[static_cast<std::size_t>(i)];
            DrawText("ÉLÉMENT", static_cast<int>(left + 620), static_cast<int>(y + 38), 13, muted);
            if (gameButton(left + 618, y + 61, 40, 40, "<")) draftElement_[static_cast<std::size_t>(i)] = (element + 3) % 4;
            panelBox({left + 664, y + 61, 142, 40});
            centeredText(elementName(static_cast<Element>(element)), static_cast<int>(left + 667), static_cast<int>(y + 72), 136, 16, teal);
            if (gameButton(left + 812, y + 61, 40, 40, ">")) draftElement_[static_cast<std::size_t>(i)] = (element + 1) % 4;

            if (editingName_ == i) {
                if (IsKeyPressed(KEY_BACKSPACE) && !draftNames_[static_cast<std::size_t>(i)].empty()) draftNames_[static_cast<std::size_t>(i)].pop_back();
                int codepoint = GetCharPressed();
                while (codepoint > 0) {
                    if (codepoint >= 32 && codepoint <= 126 && draftNames_[static_cast<std::size_t>(i)].size() < 18) {
                        draftNames_[static_cast<std::size_t>(i)].push_back(static_cast<char>(codepoint));
                    }
                    codepoint = GetCharPressed();
                }
            }
        }
        if (gameButton(80, screenH() - 72, 180, 46, "RETOUR")) scene_ = Scene::Menu;
        if (gameButton(screenW() - 310, screenH() - 72, 230, 46, "CRÉER LE ROSTER", true, true)) {
            std::vector<Player> roster;
            roster.reserve(3);
            for (std::size_t i = 0; i < draftNames_.size(); ++i) {
                roster.push_back(createHero(draftNames_[i], draftClass_[i], static_cast<Element>(draftElement_[i])));
            }
            initializeRogueliteProfile(profile_, std::move(roster));
            hasProfile_ = RogueliteSaveSystem::saveProfile(profile_);
            selectedHero_ = 0;
            scene_ = Scene::Workshop;
        }
    }

    int upgradeRank(std::size_t heroIndex, PermanentStat stat) const {
        if (heroIndex >= profile_.upgradeRanks.size()) return 0;
        const UpgradeRanks& ranks = profile_.upgradeRanks[heroIndex];
        switch (stat) {
            case PermanentStat::MaxHp: return ranks.maxHp;
            case PermanentStat::Attack: return ranks.attack;
            case PermanentStat::Defense: return ranks.defense;
        }
        return 0;
    }

    int upgradeCost(std::size_t heroIndex, PermanentStat stat) const {
        return (stat == PermanentStat::MaxHp ? 4 : 6) + upgradeRank(heroIndex, stat) * 2;
    }

    int upgradeLimit(PermanentStat stat) const {
        return stat == PermanentStat::MaxHp ? 5 : 8;
    }

    void drawWorkshop() {
        drawHeader("Atelier de la Faille", "Dépensez les fragments gagnés ; ces bonus suivent vos héros dans les runs suivantes.");
        DrawText(TextFormat("FRAGMENTS DISPONIBLES : %d", profile_.fragments), screenW() - 360, 48, 17, gold);
        const float left = 70.0f;
        for (std::size_t i = 0; i < profile_.roster.size(); ++i) {
            const float y = 128.0f + static_cast<float>(i) * 155.0f;
            const Player& hero = profile_.roster[i];
            panelBox({left, y, static_cast<float>(screenW() - 140), 136}, static_cast<int>(i) == selectedHero_ ? gold : Color{55, 69, 89, 255});
            drawHeroPortrait(left + 18, y + 27, hero, static_cast<int>(i) == selectedHero_);
            DrawText(hero.name.c_str(), static_cast<int>(left + 88), static_cast<int>(y + 22), 22, pale);
            DrawText((hero.className + " • " + elementName(hero.element)).c_str(), static_cast<int>(left + 88), static_cast<int>(y + 53), 15, muted);
            DrawText(TextFormat("PV %d  |  ATQ %d  |  DEF %d", hero.maxHp, hero.baseAttack, hero.baseDefense),
                     static_cast<int>(left + 88), static_cast<int>(y + 86), 15, teal);
            selectedHero_ = selectedHero_ < 0 ? 0 : selectedHero_;
            const float buttonsX = static_cast<float>(screenW() - 690);
            const std::array<PermanentStat, 3> stats{{PermanentStat::MaxHp, PermanentStat::Attack, PermanentStat::Defense}};
            const std::array<std::string, 3> labels{{"+10 PV max", "+1 ATQ", "+1 DEF"}};
            for (std::size_t j = 0; j < stats.size(); ++j) {
                const PermanentStat stat = stats[j];
                const int rank = upgradeRank(i, stat);
                const int cost = upgradeCost(i, stat);
                const std::string caption = labels[j] + " — " + std::to_string(cost) + " fragments";
                const float bx = buttonsX + static_cast<float>(j) * 195.0f;
                const bool enabled = rank < upgradeLimit(stat) && profile_.fragments >= cost;
                if (gameButton(bx, y + 46, 180, 46, rank >= upgradeLimit(stat) ? "PLAFOND ATTEINT" : caption,
                               enabled, j == 0)) {
                    if (buyPermanentUpgrade(profile_, i, stat)) {
                        statusMessage_ = hero.name + " progresse durablement.";
                        RogueliteSaveSystem::saveProfile(profile_);
                    }
                }
                if (rank > 0) DrawText(TextFormat("Rang %d", rank), static_cast<int>(bx + 4), static_cast<int>(y + 100), 13, muted);
            }
            const Rectangle card{left, y, static_cast<float>(screenW() - 140), 136};
            if (CheckCollisionPointRec(GetMousePosition(), card) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) selectedHero_ = static_cast<int>(i);
        }
        if (gameButton(75, screenH() - 73, 180, 46, "MENU")) scene_ = Scene::Menu;
        if (gameButton(screenW() - 315, screenH() - 73, 240, 46, "LANCER LA RUN", true, true)) beginRun();
    }

    void beginRun() {
        const std::uint32_t seed = rng_();
        if (!startRogueliteRun(profile_, run_, seed)) {
            statusMessage_ = "Impossible de démarrer la run avec ce roster.";
            return;
        }
        rng_.seed(seed);
        hasRun_ = true;
        selectedHero_ = 0;
        lastCombatMessage_.clear();
        RogueliteSaveSystem::saveProfile(profile_);
        RogueliteSaveSystem::saveRun(run_);
        roomCheckpoint_ = run_;
        scene_ = Scene::Map;
    }

    int firstLivingHero() const {
        for (std::size_t i = 0; i < run_.team.size(); ++i) if (run_.team[i].alive()) return static_cast<int>(i);
        return 0;
    }

    void drawPartyPanel(float x, float y, float width, bool clickable, bool compact = false) {
        for (std::size_t i = 0; i < run_.team.size(); ++i) {
            const Player& hero = run_.team[i];
            const float cardHeight = compact ? 74.0f : 100.0f;
            const float rowHeight = compact ? 82.0f : 113.0f;
            const float cy = y + static_cast<float>(i) * rowHeight;
            const bool selected = static_cast<int>(i) == selectedHero_;
            panelBox({x, cy, width, cardHeight}, selected ? gold : Color{55, 69, 89, 255});
            drawHeroPortrait(x + (compact ? 8.0f : 13.0f), cy + 9, hero, selected);
            const float textX = x + (compact ? 68.0f : 82.0f);
            if (compact) {
                DrawText(TextFormat("%zu. %s", i + 1, hero.name.c_str()), static_cast<int>(textX), static_cast<int>(cy + 6), 15, hero.alive() ? pale : danger);
                DrawText(hero.className.c_str(), static_cast<int>(textX), static_cast<int>(cy + 25), 11, muted);
                DrawText(TextFormat("%d / %d PV", std::max(0, hero.hp), hero.maxHp), static_cast<int>(textX), static_cast<int>(cy + 42), 12, pale);
                progressBar(textX, cy + 61, width - (textX - x) - 15, 7, std::max(0, hero.hp), hero.maxHp, hero.alive() ? teal : danger);
            } else {
                DrawText(TextFormat("%zu. %s", i + 1, hero.name.c_str()), static_cast<int>(textX), static_cast<int>(cy + 14), 17, hero.alive() ? pale : danger);
                DrawText(hero.className.c_str(), static_cast<int>(textX), static_cast<int>(cy + 38), 13, muted);
                DrawText(TextFormat("%d / %d PV", std::max(0, hero.hp), hero.maxHp), static_cast<int>(textX), static_cast<int>(cy + 59), 13, pale);
                progressBar(textX, cy + 80, width - (textX - x) - 18, 8, std::max(0, hero.hp), hero.maxHp, hero.alive() ? teal : danger);
            }
            const Rectangle card{x, cy, width, cardHeight};
            if (clickable && hero.alive() && CheckCollisionPointRec(GetMousePosition(), card) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                selectedHero_ = static_cast<int>(i);
            }
        }
    }

    void drawMap() {
        const int roomNumber = std::min(run_.completedRooms + 1, 5);
        drawHeader("La descente", TextFormat("RUN • SALLE %d / 5 • GRAINE %u", roomNumber, run_.seed));
        const float left = 60.0f;
        const float right = static_cast<float>(screenW() - 60);
        panelBox({left, 132, right - left, 174});
        DrawText("CHEMIN DE LA FAILLE", 87, 154, 16, gold);
        const float routeY = 231.0f;
        const float routeStart = left + 105.0f;
        const float routeEnd = right - 105.0f;
        DrawLineEx({routeStart, routeY}, {routeEnd, routeY}, 4.0f, Color{66, 78, 96, 255});
        for (int i = 0; i < 5; ++i) {
            const float x = routeStart + (routeEnd - routeStart) * static_cast<float>(i) / 4.0f;
            const bool cleared = i < run_.completedRooms;
            const bool next = i == run_.completedRooms;
            DrawCircle(static_cast<int>(x), static_cast<int>(routeY), next ? 19.0f : 15.0f,
                       cleared ? teal : next ? gold : Color{49, 58, 74, 255});
            DrawCircleLines(static_cast<int>(x), static_cast<int>(routeY), next ? 21.0f : 16.0f, Color{151, 164, 183, 255});
            const std::string label = i == 4 ? "BOSS" : "SALLE " + std::to_string(i + 1);
            centeredText(label, static_cast<int>(x - 70), 260, 140, 13, cleared ? teal : next ? pale : muted);
        }

        panelBox({left, 333, 385, 342});
        DrawText("ROSTER", 85, 354, 16, gold);
        drawPartyPanel(80, 385, 345, false, true);
        const float rightX = 475.0f;
        panelBox({rightX, 333, right - rightX, 342});
        DrawText("PROGRESSION DE RUN", static_cast<int>(rightX + 28), 354, 16, gold);
        DrawText(TextFormat("Fragments en banque : %d", profile_.fragments), static_cast<int>(rightX + 30), 399, 19, pale);
        DrawText(TextFormat("Rivaux Némésis : %zu", run_.nemeses.size()), static_cast<int>(rightX + 30), 435, 17, violet);
        DrawText("Les survivants gagnent des soins au camp.", static_cast<int>(rightX + 30), 475, 15, muted);
        DrawText("Un rival qui tue repart avec l’arme et l’armure.", static_cast<int>(rightX + 30), 502, 15, muted);
        DrawText("4 salles précèdent le boss de la Faille.", static_cast<int>(rightX + 30), 529, 15, muted);
        if (gameButton(screenW() - 330, screenH() - 79, 250, 50, "ENTRER DANS LA SALLE", true, true)) enterRoom();
        if (gameButton(80, screenH() - 79, 180, 50, "SAUVEGARDER / MENU")) {
            persistCheckpoint();
            scene_ = Scene::Menu;
        }
        if (!statusMessage_.empty()) DrawText(statusMessage_.c_str(), 280, screenH() - 68, 15, gold);
    }

    void enterRoom() {
        if (run_.pendingReward) {
            scene_ = Scene::Reward;
            return;
        }
        encounter_ = nextEncounter(run_);
        const std::uint32_t roomSeed = run_.seed ^
            (static_cast<std::uint32_t>(run_.completedRooms + 1) * 2654435761U);
        rng_.seed(roomSeed);
        roomCheckpoint_ = run_;
        RogueliteSaveSystem::saveProfile(profile_);
        RogueliteSaveSystem::saveRun(run_);
        selectedHero_ = firstLivingHero();
        lastCombatMessage_.clear();
        scene_ = Scene::Combat;
    }

    void performAction(CombatAction action) {
        const CombatTurnResult result = resolveCombatAction(run_, encounter_, static_cast<std::size_t>(selectedHero_), action, rng_);
        if (!result.actionApplied) {
            lastCombatMessage_ = action == CombatAction::Potion ? "Aucune potion disponible." : "Action impossible.";
            return;
        }
        if (result.playerDamage > 0) {
            lastCombatMessage_ = run_.team[static_cast<std::size_t>(selectedHero_)].name + " inflige " +
                                 std::to_string(result.playerDamage) + " dégâts.";
            if (result.playerCritical) lastCombatMessage_ += " Coup critique !";
        } else if (result.healing > 0) {
            lastCombatMessage_ = "Soin : +" + std::to_string(result.healing) + " PV.";
        } else {
            lastCombatMessage_ = "La potion ne restaure pas de PV supplémentaires.";
        }
        if (result.enemyDamage > 0 && result.targetIndex >= 0) {
            const Player& target = run_.team[static_cast<std::size_t>(result.targetIndex)];
            lastCombatMessage_ += " " + encounter_.enemy.name + " frappe " + target.name +
                                  " (-" + std::to_string(result.enemyDamage) + " PV).";
            if (result.enemyCritical) lastCombatMessage_ += " Critique !";
        }
        if (result.heroDied) lastCombatMessage_ = "NÉMÉSIS : " + encounter_.enemy.name +
            " vole l’équipement de " + run_.team[static_cast<std::size_t>(result.targetIndex)].name +
            " et gagne en puissance !";
        if (result.heroDied && !result.teamDefeated) selectedHero_ = firstLivingHero();
        if (result.teamDefeated) {
            lastCombatMessage_ = "Toute l’équipe est tombée. La run est terminée.";
            saveAfterCombatEnd();
            scene_ = Scene::Summary;
            return;
        }
        if (result.enemyDefeated) {
            if (resolveEncounterVictory(profile_, run_, encounter_)) {
                if (run_.won) {
                    lastCombatMessage_ = "Le boss est vaincu. La Faille se referme.";
                    scene_ = Scene::Summary;
                    saveAfterCombatEnd();
                } else {
                    lastCombatMessage_ = "Salle purifiée. Choisissez votre récompense.";
                    scene_ = Scene::Reward;
                    RogueliteSaveSystem::saveProfile(profile_);
                    RogueliteSaveSystem::saveRun(run_);
                    roomCheckpoint_ = run_;
                }
            }
        }
    }

    void saveAfterCombatEnd() {
        RogueliteSaveSystem::saveProfile(profile_);
        RogueliteSaveSystem::saveRun(run_);
        hasRun_ = false;
    }

    void drawCombat() {
        if (run_.team.size() > 0 && run_.team[0].alive() && IsKeyPressed(KEY_ONE)) selectedHero_ = 0;
        if (run_.team.size() > 1 && run_.team[1].alive() && IsKeyPressed(KEY_TWO)) selectedHero_ = 1;
        if (run_.team.size() > 2 && run_.team[2].alive() && IsKeyPressed(KEY_THREE)) selectedHero_ = 2;
        const std::string type = encounter_.boss ? "BOSS DE LA FAILLE" :
            encounter_.nemesisId.empty() ? "RENCONTRE" : "RIVAL NÉMÉSIS";
        drawHeader(type, "Choisissez un héros vivant puis une action ; les tours des ennemis se résolvent automatiquement.");
        const float left = 55.0f;
        panelBox({left, 132, 385, 460}, gold);
        DrawText("ÉQUIPE", 80, 154, 16, gold);
        drawPartyPanel(70, 187, 355, true);
        const float enemyX = 480.0f;
        const float enemyW = static_cast<float>(screenW() - 535);
        panelBox({enemyX, 132, enemyW, 460}, encounter_.boss ? danger : violet);
        DrawCircle(static_cast<int>(enemyX + enemyW / 2), 252, 78.0f, Color{47, 39, 52, 255});
        DrawCircleLines(static_cast<int>(enemyX + enemyW / 2), 252, 80.0f, encounter_.boss ? danger : gold);
        DrawTriangle({enemyX + enemyW / 2, 192}, {enemyX + enemyW / 2 - 36, 296},
                     {enemyX + enemyW / 2 + 36, 296}, encounter_.boss ? Color{159, 75, 77, 255} : Color{109, 100, 153, 255});
        centeredText(encounter_.enemy.name, static_cast<int>(enemyX + 20), 349, static_cast<int>(enemyW - 40), 27, pale);
        centeredText((encounter_.enemy.className + " • " + elementName(encounter_.enemy.element)).c_str(),
                     static_cast<int>(enemyX + 20), 389, static_cast<int>(enemyW - 40), 15, muted);
        progressBar(enemyX + 50, 428, enemyW - 100, 17, encounter_.enemy.hp, encounter_.enemy.maxHp, danger);
        centeredText(TextFormat("PV %d / %d     ATQ %d     DEF %d     CRIT %d%%", std::max(0, encounter_.enemy.hp),
                     encounter_.enemy.maxHp, encounter_.enemy.attack, encounter_.enemy.defense, encounter_.enemy.chance),
                     static_cast<int>(enemyX + 20), 454, static_cast<int>(enemyW - 40), 15, pale);
        if (!encounter_.nemesisId.empty()) {
            const auto rival = std::find_if(run_.nemeses.begin(), run_.nemeses.end(), [this](const Nemesis& nemesis) {
                return nemesis.id == encounter_.nemesisId;
            });
            std::string stolenItems = "butin à récupérer";
            if (rival != run_.nemeses.end() && !rival->stolenEquipment.empty()) {
                stolenItems.clear();
                for (const Item& item : rival->stolenEquipment) {
                    if (!stolenItems.empty()) stolenItems += ", ";
                    stolenItems += item.name;
                }
            }
            centeredText("ÉQUIPEMENT VOLÉ : " + stolenItems, static_cast<int>(enemyX + 20), 484,
                         static_cast<int>(enemyW - 40), 14, gold);
        }

        const float actionY = static_cast<float>(screenH() - 147);
        const float actionX = static_cast<float>(screenW() - 785);
        const std::string selected = run_.team.empty() ? "Héros" : run_.team[static_cast<std::size_t>(selectedHero_)].name;
        DrawText(("ACTION DE " + selected).c_str(), static_cast<int>(actionX), static_cast<int>(actionY - 31), 15, gold);
        if (gameButton(actionX, actionY, 210, 48, "ATTAQUER  [A]", true, true) || IsKeyPressed(KEY_A)) performAction(CombatAction::Attack);
        if (gameButton(actionX + 226, actionY, 210, 48, "TECHNIQUE  [S]") || IsKeyPressed(KEY_S)) performAction(CombatAction::Skill);
        if (gameButton(actionX + 452, actionY, 210, 48, "POTION  [P]") || IsKeyPressed(KEY_P)) performAction(CombatAction::Potion);
        panelBox({65, static_cast<float>(screenH() - 79), static_cast<float>(screenW() - 130), 54});
        DrawText(lastCombatMessage_.empty() ? "Le silence de la Faille vous observe." : lastCombatMessage_.c_str(),
                 88, screenH() - 62, 15, lastCombatMessage_.find("NÉMÉSIS") != std::string::npos ? gold : pale);
        DrawText("ÉCHAP : quitter le combat et reprendre au début de la salle", 75, screenH() - 104, 13, muted);
    }

    void drawReward() {
        drawHeader("Récompense de salle", "Choisissez un seul avantage avant de poursuivre. Les héros tombés ne peuvent pas être réanimés.");
        panelBox({80, 135, static_cast<float>(screenW() - 160), 306}, gold);
        DrawText("LES SURVIVANTS", 108, 158, 16, gold);
        drawPartyPanel(100, 188, static_cast<float>(screenW() - 200), true, true);
        const float cardY = 455.0f;
        const float cardWidth = static_cast<float>((screenW() - 190) / 2);
        panelBox({80, cardY, cardWidth, 190}, teal);
        centeredText("POTION SOLAIRE", 100, static_cast<int>(cardY + 27), static_cast<int>(cardWidth - 40), 22, pale);
        centeredText("Ajoute une potion +45 PV à " + (run_.team.empty() ? std::string("un héros") : run_.team[static_cast<std::size_t>(selectedHero_)].name),
                     100, static_cast<int>(cardY + 68), static_cast<int>(cardWidth - 40), 15, muted);
        if (gameButton(120, cardY + 125, cardWidth - 80, 43, "CHOISIR LA POTION", true, true)) {
            claimReward(RoomReward::Potion);
        }
        const float rightX = 110.0f + cardWidth;
        panelBox({rightX, cardY, cardWidth, 190}, gold);
        centeredText("REPOS DU CAMP", static_cast<int>(rightX + 20), static_cast<int>(cardY + 27), static_cast<int>(cardWidth - 40), 22, pale);
        centeredText("Rend 20 % des PV max à tous les survivants.", static_cast<int>(rightX + 20), static_cast<int>(cardY + 68), static_cast<int>(cardWidth - 40), 15, muted);
        if (gameButton(rightX + 40, cardY + 125, cardWidth - 80, 43, "CHOISIR LE REPOS")) claimReward(RoomReward::Rest);
        if (!statusMessage_.empty()) centeredText(statusMessage_, 0, screenH() - 38, screenW(), 15, danger);
    }

    void claimReward(RoomReward reward) {
        const std::size_t hero = static_cast<std::size_t>(std::clamp(selectedHero_, 0, static_cast<int>(run_.team.size()) - 1));
        if (!claimRoomReward(run_, hero, reward)) return;
        statusMessage_ = reward == RoomReward::Rest ? "Les survivants reprennent leur souffle." : "Potion ajoutée au sac.";
        RogueliteSaveSystem::saveProfile(profile_);
        if (RogueliteSaveSystem::saveRun(run_)) roomCheckpoint_ = run_;
        scene_ = Scene::Map;
    }

    void drawSummary() {
        const bool victory = run_.won;
        const Color accent = victory ? teal : danger;
        DrawCircle(screenW() / 2, 190, 73, Color{38, 47, 60, 255});
        DrawCircleLines(screenW() / 2, 190, 75, accent);
        centeredText(victory ? "V" : "X", screenW() / 2 - 40, 160, 80, 52, accent);
        centeredText(victory ? "LA FAILLE EST PURIFIÉE" : "LA RUN EST TERMINÉE", 0, 300, screenW(), 30, pale);
        centeredText(TextFormat("Salles franchies : %d / 4     •     Fragments en banque : %d", run_.completedRooms,
                     profile_.fragments), 0, 348, screenW(), 18, gold);
        if (!run_.nemeses.empty()) centeredText(TextFormat("Rivaux encore en liberté : %zu", run_.nemeses.size()), 0, 385, screenW(), 16, violet);
        centeredText(victory ? "Vos héros conserveront leurs améliorations pour la prochaine expédition." :
                     "Les fragments et entraînements permanents restent acquis.", 0, 422, screenW(), 16, muted);
        const float x = static_cast<float>(screenW() / 2 - 240);
        if (gameButton(x, 500, 220, 54, "PRÉPARER UNE NOUVELLE RUN", true, true)) {
            hasRun_ = false;
            scene_ = Scene::Workshop;
        }
        if (gameButton(x + 260, 500, 220, 54, "MENU PRINCIPAL")) scene_ = Scene::Menu;
    }

    void saveAndReturnToMenu() {
        if (scene_ == Scene::Combat) {
            run_ = roomCheckpoint_;
            hasRun_ = run_.active;
        }
        persistCheckpoint();
        scene_ = Scene::Menu;
    }

    void handleEscape() {
        if (scene_ == Scene::Menu) return;
        if (scene_ == Scene::Combat) {
            run_ = roomCheckpoint_;
            hasRun_ = run_.active;
            scene_ = Scene::Map;
            statusMessage_ = "Checkpoint chargé : le combat recommencera au début de la salle.";
            persistCheckpoint();
            return;
        }
        if (scene_ == Scene::Roster || scene_ == Scene::Workshop || scene_ == Scene::Map || scene_ == Scene::Reward) {
            saveAndReturnToMenu();
            return;
        }
        scene_ = Scene::Menu;
    }
};
}

int runRaylibGame() {
    RaylibApplication application;
    return application.run();
}
