# Audit de `RainderAx/maladie` — « Chroniques de la Faille » (v2)

> **Référence auditée :** https://github.com/RainderAx/maladie — branche `main`, commit `4b5d5fb9e509bd14a3f0122ad812970aa023c52f` (« a »), 4 commits au total. Audit du 13/11/2026 sur un clone propre ; `git status` vide avant et après les builds.
> **Audit précédent :** commit `f966f1d` (archivé dans `previous/audit_f966f1d.md`). Depuis, 2 commits ont ajouté **+2 246 lignes** : mode roguelite « Némésis », interface graphique Raylib, CMake, tests.
> **Méthode :** lecture de tous les fichiers ajoutés/modifiés (`RogueliteSystem`, `RogueliteSaveSystem`, `CombatRules`, `HeroFactory`, `RaylibGame.cpp`, `CMakeLists.txt`, `Makefile`, tests, README, `AMELIORATIONS.md`), diff avec `f966f1d`, build CMake (terminal + tests), `ctest`, et 3 programmes de test ciblés écrits pour confirmer chaque défaut cité comme « confirmé ».
> **Limites :** **le binaire graphique n'a pas été compilé ni lancé** (Raylib 5.5 absent de l'environnement, pas d'écran) : tout ce qui touche au rendu est de la lecture statique. Linux/g++ 13.3 seulement. CMake 4.4.4 installé localement pour l'audit (absent par défaut).

---

## Synthèse exécutive

Le dépôt a gagné une vraie couche de **tests (12 scénarios, tous verts)**, un **`CMakeLists.txt`**, une **séparation règles/affichage** (`CombatRules`, `RogueliteSystem` sans I/O) et un **chargeur de sauvegarde bien plus strict** que celui de la campagne. Le build terminal + tests passe **sans aucun avertissement** (`-Wall -Wextra -pedantic`).

Mais le nouveau mode introduit des défauts de logique qui touchent sa promesse centrale, et **la quasi-totalité des défauts de l'audit précédent est toujours là** :

1. **Perte de run silencieuse (confirmé)** : un héros mort a des PV négatifs ; `saveRun` l'écrit, mais `loadRun` le **refuse** (`RogueliteSaveSystem.cpp:49`). Toute run avec un héros tombé ne peut plus être reprise après redémarrage.
2. **Les héros morts ressuscitent (confirmé)** : contraire au README (« Les héros tués ne reviennent pas »), une victoire donne de l'XP aux morts (`RogueliteSystem.cpp:264-267`) et `gainXp` les remet à pleine vie.
3. **Le rival Némésis a des PV incohérents (confirmé)** : ses PV sont figés à la valeur de la 1ʳᵉ mort et réinjectés au combat à chaque mort suivante.
4. **Équipement « volé » mais bonus conservé (confirmé)** : l'objet vide garde l'élément du héros, donc +4 ATK / +4 DEF / +5 % critique persistent.
5. **Boucle infinie sur EOF toujours présente (confirmé, 139 Mo en 3 s)**.

**Priorités :** (1) borner les PV à 0 et accepter 0 au chargement, (2) ne jamais donner d'XP aux morts, (3) corriger l'EOF, (4) corriger quête / Pile Face / bornes du chargeur de la campagne (inchangés), (5) rendre le build par défaut utilisable sans Raylib.

---

## Suivi des constats de l'audit précédent (`f966f1d` → `4b5d5fb`)

| Constat précédent | État | Preuve |
|---|---|---|
| `CMakeLists.txt` documenté mais absent | **Corrigé** | `CMakeLists.txt` (47 lignes) |
| Aucun test | **Partiellement corrigé** | `tests/roguelite_tests.cpp`, `ctest` : 1/1 passé |
| Tâche VS Code compilant `${file}` | **Corrigé** | `.vscode/tasks.json` → `make` |
| Makefile sans dépendances headers / CRLF | **Remplacé** | Makefile = enveloppe CMake (voir constat build) |
| Boucle infinie sur EOF (`Terminal.hpp:9`) | **Non corrigé** | Rejoué : 138 797 445 octets en 3 s |
| Quête validée au 1ᵉʳ coup (`CombatSystem.cpp:87`) | **Non corrigé** | ligne inchangée dans le diff |
| Pile Face : chance 35 + 35, plafonnée 60 | **Non corrigé** | `HeroFactory.cpp:31` + `PileFace.cpp:10` |
| Résurrection par `gainXp` | **Non corrigé, étendu au mode roguelite** | test T6 ci-dessous |
| `load()` accepte 1-3 héros, menus `ask(1,3)` | **Non corrigé** | `SaveSystem.cpp` absent du diff |
| Équipement remplacé détruit (`Game.cpp`) | **Non corrigé** | inchangé |
| Fuite contre un boss = attaque silencieuse | **Non corrigé** | `CombatSystem.cpp:57,70` |
| Arène : tirage sur les 20 ennemis dès la vague 1 | **Non corrigé** | `ArenaSystem.cpp` inchangé |
| Contradiction Ombre dans la doc | **Partiel** : `readme.md` réécrit, mais `docs/README_CLASSES.md` garde `Ombre \| Eau` | fichier absent du diff |
| `docs/README_CLASSES.md` / `legacy.md` obsolètes | **Non corrigé** | absents du diff |
| `savegame.txt` et binaires `legacy/` suivis par Git | **Non corrigé** (voir nuance fixture) | `git ls-files sauvegarde legacy` |
| Écriture de sauvegarde non atomique | **Non corrigé, reproduit** dans les nouveaux fichiers | `RogueliteSaveSystem.cpp:91` |

---

## Objectif, fonctionnement et fonctionnalités observées

Deux modes dans un même dépôt :

- **Campagne terminal** (inchangée sauf refactor `createHero` / `CombatRules`) : 3 héros, 4 donjons, arène, boutique, quête, sauvegarde V3.
- **Roguelite graphique Raylib** (nouveau) : roster persistant de 3 héros, atelier d'améliorations payées en fragments, run de 4 salles + boss, récompense potion/repos, **rivaux Némésis** (un ennemi qui tue un héros vole arme + armure et grandit), checkpoints à chaque salle, `--terminal` pour basculer vers la campagne.

Fonctionnalités annoncées et retrouvées : barème Némésis (`+ATQ arme`, `+DEF armure`, `+10×niveau PV max`, `+2 critique plafonné 60 %` — `RogueliteSystem.cpp:67-70`), coûts d'amélioration (`4 + 2×rang` / `6 + 2×rang`, caps 5/8/8 — `:33-35,111-123`), sélection d'ennemis reproductible par graine et paliers de puissance (`:167-198`), plafonds de validation de sauvegarde.

Annoncée mais **contredite par le code** : « Les héros tués ne reviennent pas pendant la run » (voir C2).
Non vérifiable ici : tout le rendu et les interactions de l'UI.

---

## Architecture actuelle

```
main.cpp              → Raylib (ou --terminal)         src/terminal_main.cpp → campagne seule
include|src/core      Types, Player, Terminal, HeroFactory (nouveau)
include|src/characters hiérarchie de 11 classes + factory
include|src/entities  EnemyFactory (+ fonctions d'éléments, toujours mal placées)
include|src/systems   Combat, CombatRules (nouveau), Dungeon, Arena, Game, SaveSystem,
                      RogueliteSystem (nouveau), RogueliteSaveSystem (nouveau)
include|src/ui        RaylibGame (626 lignes, une classe)
tests/                roguelite_tests.cpp (329 lignes, 12 scénarios)
```

Points forts : cœur métier (`RogueliteSystem`, `CombatRules`) **sans I/O**, donc testable ; RNG **injecté** (`std::mt19937&`) dans le nouveau code ; `game_core` en bibliothèque partagée entre terminal, GUI et tests ; validation de sauvegarde exhaustive.
Point faible : `RaylibGame.cpp` mélange dessin, saisie, orchestration de scènes et appels de sauvegarde dans une seule classe.

---

## État de build et vérifications

- **Commandes exécutées :**
  - `make` (cible par défaut) → **échec** : `make: cmake: No such file or directory`, `Error 127`.
  - Après `pip install cmake` : `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_GAME=OFF` puis `cmake --build build -j4` → `game_core`, `chroniques_de_la_faille_terminal`, `roguelite_tests` construits, **0 warning**.
  - `ctest --test-dir build --output-on-failure` → `100% tests passed out of 1`.
  - T1 : `chroniques_de_la_faille_terminal < /dev/null` (timeout 3 s) → 138 797 445 octets.
  - T5/T6 : programmes de test liés à `game_core` (voir C1-C4).
- **Non exécuté :** cible `chroniques_de_la_faille` (Raylib 5.5 requis par `CMakeLists.txt:33`, absent).
- **Outillage :** g++ 13.3.0 ; CMake 4.4.4 (ajouté pour l'audit) ; `-std=c++17`.

---

## Constats priorisés

### Code, logique et erreurs (nouveau mode)

> **[Élevé] C1 — Une run avec un héros mort ne peut pas être rechargée.**
> `src/systems/RogueliteSystem.cpp:250` fait `target.hp -= dégâts` sans borne ; un héros tué a donc des PV négatifs. `RogueliteSaveSystem.cpp:48-50` rejette `hero.hp < 0` au chargement alors que `saveRun` (`:134-151`) n'a **aucun** contrôle équivalent. **Confirmé (T5a)** : `hp=-17` → `saveRun=1`, `loadRun=0` ; `hp=0` → `loadRun=1`.
> **Effet :** la sauvegarde de récompense (`RaylibGame.cpp:476-477`) puis le checkpoint de la salle suivante (`:429-430`) écrivent ces PV négatifs ; au prochain lancement `loadState()` (`:123`) reçoit `false`, **la reprise disparaît sans message** et la run est perdue.
> **Correction :** `target.hp = std::max(0, target.hp - damage)` (`RogueliteSystem.cpp:250`) **et** valider `hp >= 0` à l'écriture pour que `saveRun` échoue au lieu de produire un fichier illisible. Ajouter un test « run avec un héros mort : save → load ».

> **[Moyen] C2 — Les héros morts reviennent à la vie en montant de niveau.**
> `RogueliteSystem.cpp:264-267` donne or et XP à **tous** les héros ; `Player::gainXp` (`Player.hpp:30`) remet `hp = maxHp` à chaque niveau sans tester `alive()`. **Confirmé (T6)** : un héros à −12 PV passe `alive=0` → `alive=1`, niveau 2, 143 PV après une victoire (ennemi à 117 XP). Le README et `AMELIORATIONS.md` annoncent le contraire. Même défaut qu'en campagne (audit précédent), désormais au cœur de la promesse « roguelite ».
> **Correction :** donner l'XP uniquement aux héros vivants dans `resolveEncounterVictory` (et `CombatSystem.cpp:106`, `ArenaSystem.cpp:19`), ou faire de `gainXp` un no-op si `!alive()`.

> **[Moyen] C3 — PV du rival Némésis figés puis réinjectés.**
> `promoteKiller` (`RogueliteSystem.cpp:91-102`) termine par `encounter.enemy = nemesis->enemy`. `nemesis.enemy.hp` n'est mis à jour qu'à la création (copie de l'ennemi blessé) et `stealEquippedItems` n'augmente que `maxHp`. **Confirmé (T5b)** : 1ʳᵉ mort : 126 → 112 PV ; 2ᵉ et 3ᵉ morts : le rival repart à **112** alors que le héros l'avait frappé pendant le même tour (PV max 136 → 146 → 156). Au réapparaître (`nextEncounter:174-176`) le rival reprend aussi ces PV figés.
> **Effet :** les dégâts infligés entre deux morts sont annulés ; un rival peut revenir blessé ou, selon le moment, soigné de façon arbitraire. Non documenté dans la spec.
> **Correction :** ne copier que les stats « de croissance » vers l'ennemi en combat (garder `hp`), et décider explicitement si un rival revient à pleine vie.

> **[Moyen] C4 — Équipement volé : les bonus élémentaires restent.**
> `RogueliteSystem.cpp:65-66` remplace l'arme/armure par un `Item` vide dont l'élément est `victim.element` ; `Player::matchingEquipmentBonus` (`Player.hpp:19-24`) compare les éléments et accorde +4/+4, et `criticalChance` +5. **Confirmé (T5c)** : héros Guerrier/Feu, ATK 24 → 22 seulement (−2 de l'arme, aucun malus de −4), DEF 17 → 15, critique inchangé à 15 %. **T5d** : un héros déjà dépouillé, volé de nouveau, remplit le butin de **2 objets sans nom** (reversés dans le sac d'un survivant, `:277`).
> **Correction :** représenter l'emplacement vide explicitement (`bool equipped` ou `std::optional<Item>`), ne pas voler un emplacement vide, ne pas attribuer le bonus assorti à un emplacement vide.

> **[Moyen] C5 — `Échap` en combat annule les morts (et donc la mécanique Némésis).**
> `RaylibGame.cpp:604-612` : en combat, `Échap` recharge `roomCheckpoint_` (comportement documenté dans le README). Après la mort d'un héros, le joueur annule simplement la salle ; le rival ne se forme jamais et aucune perte n'est subie. La graine de salle étant réinitialisée (`:427`), les jets sont même identiques à l'essai suivant à choix identiques. À arbitrer côté design (c'est voulu et documenté, mais cela vide la promesse « les morts comptent »).

> **[Faible] C6 — Duplications de la définition des potions et des bornes.** « Potion solaire » est redéfinie dans `HeroFactory.cpp:39-40`, `RogueliteSystem.cpp:143-144` et `:297`, et `Game.cpp` (boutique). `maxHeroCount` existe en trois exemplaires (`RogueliteSystem.cpp:9`, `RogueliteSaveSystem.cpp:9`, bornes `3` en dur du terminal). **Correction :** une fabrique `makeSolarPotion()`, une constante partagée.

### Code, logique et erreurs (campagne terminal — inchangés)

Constats toujours valides ; détails dans `previous/audit_f966f1d.md` :

| Priorité | Constat | Emplacement |
|---|---|---|
| **Élevé** | `ask()` boucle sans fin sur EOF (139 Mo / 3 s confirmé à nouveau) | `include/core/Terminal.hpp:9` |
| Moyen | Quête « Premier éclat » validée au premier coup, pas à la victoire | `CombatSystem.cpp:87` (+`:107`) |
| Moyen | Pile Face : `chance=35` + `luckBonus=35` ⇒ 60 % plafonné dès le niv. 1 | `HeroFactory.cpp:31`, `PileFace.cpp:10`, `Player.hpp:27` |
| Moyen | `load()` accepte 1-3 héros, menus indexent `team[0..2]` (ASan : heap-buffer-overflow) | `SaveSystem.cpp:53`, `CombatSystem.cpp:32`, `Game.cpp:52,79` |
| Moyen | Équiper un objet détruit l'ancien | `Game.cpp` (`inventory()`) |
| Moyen | Équipe entièrement K.O. sauvegardée ⇒ reprise inutilisable | `Game.cpp` (`explore`, `hub`) |
| Faible | « Fuir » contre un boss = attaque silencieuse | `CombatSystem.cpp:57,70-73` |
| Faible | Arène : tirage parmi les 20 ennemis quelle que soit la vague | `ArenaSystem.cpp:16` |
| Faible | Fonctions d'éléments définies dans `EnemyFactory.cpp:20-28` mais déclarées dans `Types.hpp` | — |

### Sécurité et fiabilité

> **[Moyen] F1 — Écritures non atomiques et retours ignorés (nouveaux fichiers).** `RogueliteSaveSystem.cpp:91` ouvre avec `std::ios::trunc` avant toute validation ; `saveProfile` peut ensuite retourner `false` à `:105` (rang invalide) en laissant un fichier **tronqué/partiel**. Dans l'UI, plusieurs appels ignorent le résultat (`RaylibGame.cpp:317,339-340,429-430,476-477,485-486`) et, à la fermeture, `persistCheckpoint()` écrit un message d'erreur dans `statusMessage_` (`:130-133`) juste avant `CloseWindow()` (`:90-91`) : l'erreur n'est **jamais affichée**. **Correction :** écrire dans `*.tmp` puis `std::filesystem::rename`, valider avant d'ouvrir, journaliser/afficher l'échec.

> **[Faible] F2 — Bon point : validation stricte au chargement.** `readPlayer`/`readEnemy`/`loadProfile`/`loadRun` bornent classes, éléments, niveaux, PV, plafonds et tailles (`RogueliteSaveSystem.cpp:9-12,48-50,78-79,117,157`) et ne remplacent l'état qu'après lecture complète (`:130,182`). À reprendre pour `SaveSystem.cpp` (campagne), qui n'a toujours aucune validation de valeurs.

### Build, documentation, configuration et livraison

> **[Moyen] B1 — La cible `make` par défaut exige CMake **et** Raylib 5.5.** `Makefile:3-5` (`all`) configure avec `-DBUILD_GRAPHICAL_GAME=ON`, et `CMakeLists.txt:33` utilise `find_package(raylib 5.5 CONFIG REQUIRED)` sans repli. **Constaté :** `make` échoue sur une machine sans CMake ; sans Raylib, `cmake` échouerait à la configuration. Auparavant, `make` ne demandait que `g++`. `make terminal`, `make test` passent aussi par CMake/CTest. **Correction :** détecter Raylib (`find_package(raylib 5.5 QUIET)` ou `FetchContent` optionnel) et construire le terminal si absent ; faire de `make` un alias de `make terminal` ou documenter clairement la procédure d'installation (apt/brew/vcpkg) dans le README.

> **[Moyen] B2 — Le test de rétrocompatibilité lit la sauvegarde personnelle de l'utilisateur.** `tests/roguelite_tests.cpp:304` charge `sauvegarde/savegame.txt` (répertoire de travail `CMakeLists.txt:47`), fichier **suivi par Git, modifié par chaque partie de campagne** et contenant des noms réels (« Maladie », « Rachid »). Une partie jouée depuis la racine du dépôt peut donc faire échouer ou fausser le test. **Correction :** copier la fixture dans `tests/fixtures/campaign_v3.txt`, retirer `sauvegarde/savegame.txt` du suivi (`git rm --cached`), ignorer `sauvegarde/*.txt`.

> **[Moyen] B3 — Documentation du dépôt encore en partie obsolète.** `docs/README_CLASSES.md` : tableau `Ombre | Eau | —` (contredit sa propre section « Neutralité d'Ombre » et le code), chemins `CharacterClasses.hpp/.cpp` et dossier `rpg-terminal/` inexistants ; `legacy.md` décrit l'architecture d'avant. Le `audit.md` **commité à la racine** (copie de l'audit précédent) est maintenant lui-même périmé (il affirme que `CMakeLists.txt` n'existe pas) et peut induire en erreur. **Correction :** supprimer ou déplacer vers `docs/` avec la date et le commit audités ; corriger `docs/README_CLASSES.md`.

> **[Faible] B4 — Fins de ligne mixtes.** `src/systems/CombatSystem.cpp` (99 CRLF / 11 LF) et `src/systems/Game.cpp` (91 / 15) mélangent les deux ; 56 fichiers sont en CRLF pur, 23 en LF pur. Pas de `.gitattributes`. Les diffs contiennent des lignes « modifiées » qui ne le sont que par leur terminaison. **Correction :** `.gitattributes` (`* text=auto eol=lf`) puis normalisation en un commit dédié.

> **[Faible] B5 — Hygiène Git.** Messages `dude` et `a` non descriptifs (le commit `78520dd` est correct) ; binaires ELF `legacy/HelloWorld` (39 Ko) et `legacy/jeu` (70 Ko) toujours suivis ; pas de `LICENSE` ; `roguelite_tests` compilé sans options de warning (`CMakeLists.txt:44-45`, contrairement à `game_core`).

### UI/UX et accessibilité (lecture statique, **rendu non vérifié**)

> **[Faible] U1 — Saisie des noms limitée à l'ASCII.** `RaylibGame.cpp:249` n'accepte que les codepoints 32-126 : impossible de nommer un héros « Élodie » ou « Inès » dans un jeu en français (la campagne terminal l'accepte). **Correction :** accepter les codepoints Latin-1 supplémentaires et encoder en UTF-8.

> **[Faible] U2 — Polices et glyphes à vérifier.** L'UI utilise `DrawText` avec la police par défaut de Raylib et des chaînes contenant `’`, `—`, `•`, `É` (`RaylibGame.cpp:68,130,184,198,299,310,380`). La police par défaut n'inclut pas tous ces symboles : des « ? » pourraient s'afficher. **Non vérifié** (pas d'exécution graphique). **Correction :** charger une police avec `LoadFontEx` et la plage de glyphes nécessaire, ou limiter le texte aux caractères disponibles.

> **[Faible] U3 — Fenêtre minimale = fenêtre par défaut en hauteur** (`SetWindowMinSize(960, 760)` pour `760` de haut à l'ouverture) : aucune mise en page réduite possible sur un petit écran portable. À tester sur écran 768 px.

### Performance

Rien de préoccupant ; `nextEncounter` reconstruit et trie 20 ennemis à chaque salle (`RogueliteSystem.cpp:179-189`), négligeable.

### Tests et expérience développeur

> **[Moyen] T1 — Les tests ne couvrent pas les défauts trouvés.** 12 scénarios vérifient les achats, les fragments, le butin Némésis, la reproductibilité et l'aller-retour de sauvegarde, mais aucun ne teste : un héros mort dans une sauvegarde de run (C1), l'XP des morts (C2), les PV du rival après plusieurs morts (C3), le bonus d'un emplacement vide (C4), `Player::criticalChance`, `Terminal::ask` ou l'EOF. **Correction :** ajouter ces 5-6 tests (chacun < 15 lignes) ; le harnais maison `CHECK` suffit.

> **[Faible] T2 — Exécutable de test unique.** `roguelite_tests.cpp` rassemble tout dans un `main` ; un échec masque les suivants. **Correction :** un seul fichier est acceptable à cette taille, mais faire retourner la liste complète des échecs (ou découper par thème).

---

## Éléments à supprimer, fusionner, simplifier ou déplacer

| Élément | Rôle observé | Décision recommandée | Condition |
|---|---|---|---|
| `audit.md` (racine, copie de l'audit v1) | rapport devenu obsolète | Déplacer vers `docs/audits/` avec commit audité, ou supprimer | Vous seul savez si vous voulez conserver l'historique |
| `sauvegarde/savegame.txt` | sauvegarde personnelle **et** fixture de test | Copier en `tests/fixtures/`, retirer du suivi | À faire en même temps que B2 |
| `legacy/HelloWorld`, `legacy/jeu` | binaires ELF | Retirer du suivi | Aucun usage dans CMake/Makefile |
| `legacy.md`, `legacy/*` | prototype d'origine | Archiver dans un tag, puis supprimer | Décision de l'auteur |
| `AMELIORATIONS.md` | journal des changements | Conserver (utile) ; vérifier ses affirmations, voir C2 | — |
| `docs/superpowers/specs/…` | spec de conception du roguelite | Conserver | — |

## Scripts et outils

| Élément | Rôle observé | Évaluation | Recommandation |
|---|---|---|---|
| `CMakeLists.txt` | `game_core`, terminal, GUI optionnelle, tests | Bonne structure ; Raylib sans repli | Voir B1 |
| `Makefile` | alias vers CMake (`all`, `terminal`, `test`, `clean`…) | Pratique mais dépend de CMake/CTest | Documenter/prévoir repli |
| `.vscode/tasks.json` | `make` / `make run` | Corrigé | — |
| `tests/roguelite_tests.cpp` | 12 scénarios métier | Verts ; couverture lacunaire | Voir T1 |
| CI | **introuvable dans ce dépôt** | — | GitHub Actions gratuit : `cmake` terminal + `ctest` |

## Refactorings recommandés

| Problème actuel | Structure actuelle → cible | Bénéfice | Difficulté | Nécessité |
|---|---|---|---|---|
| Emplacement d'équipement vide = objet fantôme | `Item` vide → `std::optional<Item>` / `bool equipped` | Supprime C4 et T5d | Moyenne | Recommandée |
| Règle de mort dispersée (3 endroits) | XP/soin à chaque boucle → `Player::gainXp` ignore les morts + PV bornés à 0 | Supprime C1, C2 | Faible | **Essentielle** |
| Rival : état « en combat » ≠ état « persistant » | copie complète de l'ennemi → séparation `NemesisState` / `Enemy` en combat | Supprime C3 | Moyenne | Recommandée |
| `RaylibGame.cpp` monolithique (626 lignes) | 1 classe → scènes séparées (`MenuScene`, `CombatScene`…) | Lisibilité, tests UI | Moyenne | Optionnelle |
| Potions et constantes dupliquées | littéraux répétés → `Items::solarPotion()` | Cohérence | Faible | Recommandée |
| Fonctions d'éléments dans `EnemyFactory.cpp` | → `src/core/Types.cpp` | Cohérence | Faible | Recommandée |

## Ajouts potentiels

- **Essentiel :** correction C1-C2 + EOF ; tests de non-régression correspondants.
- **Recommandé :** repli de build sans Raylib ; fixture de test dédiée ; sauvegarde atomique ; message d'erreur visible si l'enregistrement échoue ; CI.
- **Optionnel :** noms accentués et police complète dans l'UI ; un indicateur « Reprise impossible : sauvegarde illisible » plutôt que la disparition silencieuse du bouton.
- **Expérimental :** pénalité à `Échap` en combat (ex. perte d'une potion), pour préserver la tension du système Némésis.

## Solutions et dépendances

Toutes gratuites et locales : CMake, CTest, g++/clang, `-fsanitize=address,undefined`, GitHub Actions (dépôt public). **Raylib 5.5** est open source (licence zlib/libpng) ; limite : version exacte demandée et installation système requise, sauf `FetchContent` (nécessite un accès réseau à la configuration).

## Dette technique

- **À corriger maintenant :** C1, C2, EOF, quête, Pile Face, bornes du chargeur de campagne.
- **Peut attendre :** C3, C4, F1, B2-B4, U1-U2.
- **Risque futur :** `RaylibGame.cpp` qui grossit ; états dupliqués (quêtes ×3 héros en campagne) ; deux chargeurs de sauvegarde aux exigences très différentes.
- **Peut rester simple :** format texte avec `std::quoted`, un seul exécutable de test, pas de moteur de données.

---

## Roadmap

### Phase 1 — Corrections rapides
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Borner les PV à 0 (`std::max(0, …)`) ; rejeter `hp<0` aussi à l'écriture | Élevée | Faible | Fin de la perte silencieuse de run | `RogueliteSystem.cpp:250`, `RogueliteSaveSystem.cpp:134` |
| Ne donner l'XP/le soin de niveau qu'aux vivants | Élevée | Faible | Respecte le README | `Player.hpp:30`, `RogueliteSystem.cpp:264`, `CombatSystem.cpp:106`, `ArenaSystem.cpp:19` |
| Gérer l'EOF dans `ask()` / `getline` | Élevée | Faible | Plus de gel/sortie infinie | `Terminal.hpp:9`, `Game.cpp` |
| Quête : supprimer l'appel par coup | Moyenne | Faible | Quête correcte | `CombatSystem.cpp:87` |
| Pile Face : retirer la chance de base de 35 | Moyenne | Faible | Équilibrage | `HeroFactory.cpp:31` |

### Phase 2 — Nettoyage
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Fixture de test dédiée + retirer `savegame.txt` du suivi | Moyenne | Faible | Tests indépendants des parties jouées | `tests/`, `.gitignore`, `CMakeLists.txt:47` |
| `.gitattributes` + normalisation des fins de ligne | Faible | Faible | Diffs propres | racine |
| Corriger/retirer `docs/README_CLASSES.md`, `legacy.md`, `audit.md` obsolètes | Moyenne | Faible | Documentation fiable | `docs/`, racine |
| Retirer les binaires `legacy/` du suivi | Faible | Faible | Dépôt plus léger | `legacy/` |

### Phase 3 — Refactoring
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Emplacements d'équipement explicites (vide ≠ objet) | Moyenne | Moyenne | Supprime C4 | `Player.hpp`, `Types.hpp`, `RogueliteSystem.cpp` |
| Séparer l'état persistant du rival de l'ennemi en combat | Moyenne | Moyenne | Supprime C3 | `RogueliteSystem.cpp` |
| Constantes/fabriques partagées (potion, nb de héros) | Faible | Faible | Cohérence | `HeroFactory.cpp`, `RogueliteSystem.cpp`, `Game.cpp` |

### Phase 4 — Améliorations
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Tests C1-C4 + `Player::criticalChance` + EOF | Moyenne | Faible | Non-régression | `tests/` |
| Repli de build sans Raylib / documentation d'installation | Moyenne | Moyenne | Premier build réussi | `CMakeLists.txt`, `Makefile`, `readme.md` |
| Sauvegarde atomique + erreur visible | Moyenne | Faible | Fiabilité | `RogueliteSaveSystem.cpp`, `RaylibGame.cpp` |
| Noms accentués, police complète | Faible | Moyenne | UX | `RaylibGame.cpp` |
| CI GitHub Actions (build terminal + ctest) | Faible | Faible | Détection rapide | `.github/workflows/` |

### Phase 5 — Optimisation
Aucune optimisation justifiée.

---

## Architecture recommandée

L'architecture actuelle est **bien proportionnée** ; pas de refonte. Cible : (1) `game_core` reste sans I/O ni dépendance graphique ; (2) `ui/` ne décide d'aucune règle de jeu (déjà largement vrai) ; (3) `tests/fixtures/` pour les données de test ; (4) une règle de vie/mort unique dans `Player`.

## Résumé décisionnel

- **À conserver :** séparation `core` / `ui`, `CombatRules`, RNG injecté, validation de sauvegarde roguelite, hiérarchie de classes, build sans warning.
- **À améliorer :** gestion des PV négatifs, build par défaut, documentation, tests ciblés.
- **À refactoriser :** règle de mort/XP, équipement vide, état du rival.
- **À supprimer/remplacer :** `audit.md` périmé à la racine, binaires `legacy/`, `savegame.txt` comme fixture.
- **À ajouter :** tests de non-régression, CI, repli sans Raylib.
- **À surveiller :** taille de `RaylibGame.cpp`, cohérence README ↔ comportement (« les morts ne reviennent pas »).
- **Priorités absolues :** C1 (perte de run), C2 (résurrection), EOF.

## Questions ouvertes / faits indéterminables

- Rendu graphique, ergonomie souris/clavier, polices : **non testés** (pas de Raylib ni d'écran).
- Le retour à l'état « checkpoint » via `Échap` est-il voulu comme garde-fou ou comme faille acceptée ?
- Un rival doit-il revenir blessé, soigné ou à PV de départ ?
- La campagne terminal reste-t-elle maintenue à terme (le roguelite est maintenant le mode principal) ?
- Comportement sous Windows/macOS/clang : non testé.
