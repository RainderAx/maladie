# Audit de `RainderAx/maladie` — « Chroniques de la Faille »

> **Référence auditée :** https://github.com/RainderAx/maladie — branche `main`, commit `f966f1d509ba366cc3bb59ccfe268d9e6d044bc6` (« dude »), 2 commits au total. Audit réalisé le 13/11/2026 sur un clone propre, `git status` vide avant et après build.
> **Méthode :** lecture intégrale des 19 fichiers `src/` et des 22 headers `include/`, lecture de `readme.md`, `docs/README_CLASSES.md`, `docs/BESTIAIRE.md` (début) et `legacy.md` (début) — `guide.md` non lu, build avec le `Makefile` du dépôt, quatre tests dynamiques reproductibles (voir plus bas), un build AddressSanitizer/UBSan en copie temporaire.
> **Limites :** Linux uniquement (g++ 13.3.0, GNU Make). Pas testé : clang, MSVC/MinGW, macOS, affichage visuel du terminal (alignement des cadres). Le dossier `legacy/` n'a été examiné qu'en surface (README : « non compilé »). Aucun correctif appliqué : l'audit était seul demandé, le dépôt distant n'est pas modifié.

---

## Synthèse exécutive

Petit RPG console en C++17 (~1 000 lignes utiles), **bien découpé** (hiérarchie de classes + factory, systèmes séparés) et qui **compile sans aucun avertissement** avec `-Wall -Wextra -pedantic`. La base est saine pour un projet de cette taille.

Les problèmes se concentrent sur quatre points :

1. **Un blocage réel : entrée standard fermée (EOF) ⇒ boucle infinie** qui a produit 205 Mo de texte en 3 secondes (`Terminal.hpp:9`).
2. **Des erreurs de logique de jeu confirmées par test** : la quête « Premier éclat » se termine au premier *coup* et non à la première victoire ; Pile Face dépasse le plafond de chance critique dès le niveau 1 ; un héros K.O. est « ressuscité » par une montée de niveau mais jamais par le repos.
3. **Un accès hors bornes** (détecté par ASan) quand une sauvegarde contient moins de 3 héros, alors que le chargeur l'autorise.
4. **Une documentation qui ne correspond plus au code** : CMake est documenté mais `CMakeLists.txt` n'existe pas ; les docs se contredisent sur l'élément Ombre ; `legacy.md` décrit une architecture disparue.

**Priorités :** (1) gérer l'EOF dans `Terminal::ask`, (2) corriger quête / chance critique / résurrection, (3) rendre cohérents `load()` et les `ask(1,3)`, (4) réaligner la documentation (ou ajouter le `CMakeLists.txt`), (5) sortir `savegame.txt` et les binaires `legacy/` du suivi Git.

---

## Objectif, fonctionnement et fonctionnalités observées

Jeu de rôle au tour par tour dans le terminal : équipe de **3 héros** (nom, classe parmi 10, élément parmi 4), hub, **4 donjons** (2 salles + boss, le donjon 3 enchaîne un second boss ; le donjon 4 est optionnel), **arène infinie** par vagues, boutique, inventaire/équipement, une quête, sauvegarde automatique (format texte `CHRONIQUES_SAVE_V3`).

Fonctionnalités annoncées et **retrouvées dans le code** : éléments (×1,5 / ×0,75), bonus d'équipement assorti (+4 ATK/DEF, +5 % critique), +15 % PV ennemis / +30 % boss (`CombatSystem.cpp:38`), sauvegarde V3, bestiaire (les valeurs du `docs/BESTIAIRE.md` correspondent à `EnemyFactory.cpp`).

Annoncées mais **introuvables dans ce dépôt** : `CMakeLists.txt`, `include/characters/CharacterClasses.hpp`, avantage d'Ombre sur Eau (voir constats), tout test automatisé.

---

## Architecture actuelle

```
main.cpp                  → instancie Game
include/{core,characters,entities,systems}/*.hpp
src/{characters,entities,systems}/*.cpp
docs/  sauvegarde/  legacy/
```

| Zone | Rôle | Évaluation |
|---|---|---|
| `characters/` | Chaîne d'héritage `CharacterClass → Guerrier → Berserker → Paladin`, etc. + `CharacterFactory` | Propre, polymorphe, `shared_ptr` sans `new` brut |
| `core/` | `Player`, `Types`, `Terminal` | `Player` mélange état et règles de stats (acceptable) |
| `entities/EnemyFactory` | Données ennemis | Contient aussi des fonctions déclarées dans `Types.hpp` (voir constat) |
| `systems/` | Combat, donjon, arène, sauvegarde, `Game` | `Game.cpp` en lignes très denses |

Cohérence avec la taille du projet : bonne. Pas de sur-ingénierie.

**Forces observées :** compilation sans warning sous options strictes ; `enum class Element` ; sérialisation avec `std::quoted` (noms avec espaces/guillemets sûrs) ; en-tête de version dans la sauvegarde ; validation des entrées numériques dans `ask()` (hors EOF) ; documentation abondante (bestiaire, classes, éléments) ; aucune dépendance externe.

---

## État de build et vérifications

- **Commandes exécutées :**
  - `make` → exit 0, binaire `chroniques_de_la_faille` (≈173 Ko), aucune ligne `warning`/`error`.
  - `make -B | grep -E "warning|error"` → vide.
  - T1 : `./chroniques_de_la_faille < /dev/null` (avec `timeout 3`).
  - T2 : partie scriptée (3 héros Guerrier/Eau, un seul coup puis fuite, puis menu Quêtes).
  - T3 : mini-programme lié aux sources pour `criticalChance()` et `gainXp()`.
  - T4 : build `-fsanitize=address,undefined` + sauvegarde éditée à 1 héros, menu Sac, héros n°3.
- **Résultats :**
  - T1 → exit 124 (timeout), 205 338 015 octets produits (« Choix invalide. Recommencez : » en boucle).
  - T2 → « A inflige 41 dégâts. » puis « Quête accomplie : Premier éclat » alors que l'ennemi est vivant ; après la fuite, la quête affiche `[TERMINÉE] 1/1`.
  - T3 → `PileFace lvl1: chance=35 luckBonus=35 crit effective=60` ; `Guerrier: crit=15` ; héros à PV −5 : `alive=0` avant, `alive=1 level=5 hp=172` après `gainXp(1000)`.
  - T4 → `heap-buffer-overflow` dans `Game::inventory()` (`Game.cpp:63`).
- **Outillage :** g++ 13.3.0 (Ubuntu 24.04), GNU Make 4.x, `-std=c++17 -Wall -Wextra -pedantic -O2`.
- **Limites d'exécution :** les tests T1/T2/T4 ont été lancés dans une **copie** du dépôt (`/tmp/work`) pour ne pas modifier `sauvegarde/savegame.txt`, qui est suivi par Git.

---

## Constats priorisés

### Structure et architecture

> **[Moyen]** `include/core/Types.hpp:6-9` / `src/entities/EnemyFactory.cpp:20-28` — `elementName`, `elementFromId`, `elementId`, `elementMultiplier` sont déclarées dans `Types.hpp` mais **définies dans `EnemyFactory.cpp`**. **Effet :** une règle centrale (éléments) est cachée dans un fichier de contenu ; un projet de test qui ne lierait pas `EnemyFactory.cpp` ne se lierait plus. **Recommandation :** créer `src/core/Types.cpp` et y déplacer ces 4 fonctions (aucun changement de comportement).

> **[Moyen]** `CombatSystem.cpp:16`, `Game.cpp:38-39,76-78`, `SaveSystem.cpp:36` — les quêtes sont copiées sur chaque héros mais seul `team.front().quests` fait foi. **Effet :** état dupliqué à resynchroniser à la main (`Game.cpp:39,78`) ; fragile si l'ordre de l'équipe change. **Recommandation :** introduire une structure `GameState { team; quests; Progress; }` possédée par `Game`, passée aux systèmes.

> **[Faible]** `Game.cpp:63-66,83-102`, `EnemyFactory.cpp:5-16` — lignes très denses (plusieurs instructions par ligne, tableaux d'ennemis sur une seule ligne). **Effet :** relecture et diff difficiles. **Recommandation :** un `clang-format` + sortir les statistiques ennemis dans une table lisible (ou fichier JSON/CSV local).

### Code, logique et erreurs

> **[Élevé]** `include/core/Terminal.hpp:9` — `ask()` boucle indéfiniment si `std::cin` atteint EOF (le `clear()` + `ignore()` ne change rien à un flux terminé). **Effet :** confirmé par T1, 205 Mo de sortie en 3 s ; un `Ctrl-D` ou une entrée redirigée gèle le programme et sature le terminal. **Recommandation :** après l'échec de lecture, tester `std::cin.eof()` et quitter proprement (lever une exception rattrapée dans `main`, ou sauvegarder puis `std::exit`). Même traitement pour `std::getline` dans `Game.cpp:14`.

> **[Moyen]** `src/systems/CombatSystem.cpp:87` (et `:105`) — `updateQuestProgress(team,"vaincre")` est appelé **à chaque attaque** et encore une fois à la victoire. **Effet :** confirmé par T2 : « Premier éclat » (objectif « Vaincre votre premier adversaire ») se valide au premier coup, même si le joueur fuit ensuite ; avec un `goal` > 1 la progression serait comptée plusieurs fois par combat. **Recommandation :** supprimer l'appel de la ligne 87 et ne garder que celui de la victoire (`:105`).

> **[Moyen]** `src/systems/Game.cpp:28` + `src/characters/PileFace.cpp:10` + `include/core/Player.hpp:27` — Pile Face reçoit `chance=35` à la création **et** `luckBonus()=35` ; `criticalChance()` plafonne à 60 %. **Effet :** confirmé par T3, 70 → 60 % dès le niveau 1 : les `++chance` des montées de niveau n'ont plus aucun effet pour cette classe, le bonus de classe est en partie perdu. **Recommandation :** garder la chance de base à 10 (supprimer `hero.chance=35` ligne 28) ou retirer `luckBonus()`, et documenter la valeur voulue.

> **[Moyen]** `include/core/Player.hpp:30`, `src/systems/Game.cpp:102`, `CombatSystem.cpp:104`, `ArenaSystem.cpp:19` — `gainXp()` remet `hp = maxHp` à chaque niveau gagné **sans tester `alive()`**, tandis que « Repos » ne soigne que les héros vivants et que les récompenses (XP) sont distribuées aussi aux K.O. **Effet :** confirmé par T3 : un héros à PV négatifs redevient vivant en montant de niveau, mais ne peut sinon jamais être relevé ; les PV négatifs sont affichés. **Recommandation :** trancher la règle (mort permanente, ou repos/potion de réanimation) ; ne donner l'XP qu'aux héros vivants ou bien appliquer le soin de niveau uniquement aux vivants ; borner `hp` à 0.

> **[Moyen]** `src/systems/SaveSystem.cpp:53` vs `CombatSystem.cpp:30-32`, `Game.cpp:58-60,85-86` — `load()` accepte 1 à 3 héros, mais les menus demandent toujours `ask(1,3)` et indexent `team[choice-1]`. **Effet :** confirmé par T4 : `heap-buffer-overflow` (comportement indéfini) avec une sauvegarde éditée à 1 ou 2 héros. Non atteignable en jeu normal (création toujours 3 héros). **Recommandation :** soit exiger `teamSize == 3` dans `load()`, soit utiliser `static_cast<int>(team.size())` dans les bornes.

> **[Moyen]** `src/systems/Game.cpp:66` — équiper un objet **supprime** l'équipement remplacé (`erase` sans remettre l'ancien dans l'inventaire). **Effet :** perte définitive d'armes/armures (dont celles achetées 120-180 or). Détection arme/armure par `item.attack>0` : un équipement à ATK=0 serait toujours classé « armure ». **Recommandation :** `std::swap` entre l'emplacement et l'objet de l'inventaire, et un champ explicite `slot` dans `Item`.

> **[Moyen]** `src/systems/Game.cpp:100,102` + `SaveSystem` — après un « game over », `explore()` sauvegarde une équipe entièrement K.O. (`Game.cpp:95`) ; à la reprise, `hub()` sort immédiatement car personne n'est vivant. **Effet :** la partie est inutilisable (il faut choisir « Nouvelle équipe »). Déduit à la lecture, non rejoué. **Recommandation :** soit réanimer l'équipe à la place noire avec pénalité, soit afficher un écran de défaite et proposer explicitement de recommencer.

> **[Faible]** `src/systems/CombatSystem.cpp:57,70-73` — « 4. Fuir » contre un boss n'est pas géré : le choix 4 tombe dans la branche d'attaque et lance une **attaque de base** sans message. **Effet :** option trompeuse. **Recommandation :** afficher « On ne fuit pas un boss » et `continue`.

> **[Faible]** `src/systems/CombatSystem.cpp:71-73` — « Technique » n'a ni coût ni délai de recharge et inflige `attack + skillDamage + 0..4` contre `attack - 3..6` pour la frappe, plus un soin éventuel. **Effet :** Technique domine strictement la frappe ; le choix 1 n'est jamais utile (profondeur tactique nulle). **Recommandation :** ajouter un coût (énergie/mana ou recharge) — en supposant que ce soit l'intention de design.

> **[Faible]** `src/systems/ArenaSystem.cpp:16` — l'ennemi est tiré au hasard parmi **les 20** du roster quelle que soit la vague. **Effet :** « Avatar de la Faille » (300 PV, ATK 50) peut apparaître à la vague 1. **Recommandation :** filtrer par palier de vague (ex. fenêtre glissante sur l'index trié par puissance).

> **[Faible]** `src/systems/Game.cpp:87` — l'achat n'affiche aucune confirmation ; l'`else` et `autoSave()` sont sur la même ligne, ce qui prête à confusion. **Recommandation :** message de confirmation et mise en forme multi-lignes.

### Sécurité et fiabilité

> **[Moyen]** `src/systems/SaveSystem.cpp:22-24` et `Game.cpp:10` — `save()` ouvre le fichier final en écriture (troncature immédiate) puis écrit ; la valeur de retour est ignorée par `autoSave()`. **Effet :** un crash ou un disque plein pendant l'écriture détruit la progression sans avertir. **Recommandation :** écrire dans `savegame.txt.tmp` puis `std::filesystem::rename`, et signaler l'échec à l'écran.

> **[Faible]** `SaveSystem.cpp:57-66,79`, `CharacterFactory.cpp:27` — pas de validation des valeurs lues (niveau, PV, quantité d'objets) ; un `classId` inconnu devient silencieusement « Guerrier » ; `className` est stocké alors que `characterClass->name()` le fournit déjà. **Effet :** une sauvegarde corrompue ou éditée peut produire des états incohérents. **Recommandation :** valider les bornes, rejeter les `classId` hors 1-11, ne plus sérialiser `className` (recalculé au chargement).

Pas de surface d'attaque réseau ni de traitement de données externes : le risque sécurité est proportionnellement faible (la seule entrée non fiable est le fichier de sauvegarde local).

### Performance

Aucun point préoccupant : `arenaRoster()` reconstruit 20 ennemis à chaque vague (`ArenaSystem.cpp:15`), coût négligeable ; deux générateurs `std::mt19937` distincts (`CombatSystem.cpp:6`, `ArenaSystem.cpp:13`) sans conséquence fonctionnelle. À ne pas optimiser.

### UI/UX et accessibilité

> **[Faible]** `include/core/Terminal.hpp:8` — `title()` calcule l'alignement avec `text.size()` en **octets** UTF-8 (« ARÈNE », « É »…) et peut passer une largeur négative à `setw` pour un titre de plus de 60 caractères. **Effet :** cadres potentiellement mal alignés (non vérifié visuellement). **Recommandation :** compter les caractères Unicode ou centrer sans cadre dynamique ; prévoir une option sans codes ANSI (`NO_COLOR`).

### Documentation, configuration et livraison

> **[Moyen]** `readme.md` (section « Compilation »), `docs/README_CLASSES.md` (arborescence et section « Compilation ») — documentent `cmake -S . -B build`, mais **aucun `CMakeLists.txt`** n'existe (`ls CMakeLists.txt` : introuvable). **Effet :** la commande documentée échoue pour tout nouveau lecteur. **Recommandation :** soit ajouter un `CMakeLists.txt` minimal (voir « Ajouts »), soit supprimer ces paragraphes.

> **[Moyen]** `readme.md` (section « Équipe et éléments ») et `docs/README_CLASSES.md` (tableau des éléments : `Ombre | Eau | —`) — annoncent que **Ombre est avantageux contre Eau**. Le code (`EnemyFactory.cpp:24-28`) et la dernière section de `README_CLASSES.md` / `BESTIAIRE.md` disent qu'Ombre est **neutre**. **Effet :** trois documents sur quatre se contredisent ; le code fait foi. **Recommandation :** retirer la ligne Ombre du tableau et la phrase du README.

> **[Moyen]** `docs/README_CLASSES.md` — cite `include/characters/CharacterClasses.hpp`, `src/characters/CharacterClasses.cpp` et un dossier racine `rpg-terminal/` qui n'existent pas ; `legacy.md` décrit un `main.cpp` « contenant les systèmes », des voies « Sentinelle/Arcaniste/Nocturne » et `savegame.txt` à la racine. **Effet :** documentation d'une version antérieure. **Recommandation :** corriger l'arborescence, et fusionner/archiver `legacy.md` (voir plus bas). Remarque : `guide.md` (11 Ko) n'a pas été utilisé pour établir les constats.

> **[Moyen]** `.gitignore:1-2`, `sauvegarde/savegame.txt` — le fichier de sauvegarde (noms de héros réels comme « Maladie », « Rachid », partie à 235 or) est **suivi par Git**, alors que le README le décrit comme « généré automatiquement » ; chaque partie le modifie. **Effet :** pollution du dépôt et fuite de données personnelles de jeu. **Recommandation :** `git rm --cached sauvegarde/savegame.txt` et ajouter `sauvegarde/*.txt` au `.gitignore` (garder `.gitkeep`).

> **[Faible]** `legacy/HelloWorld` (39 Ko) et `legacy/jeu` (70 Ko) — binaires ELF compilés, committés. **Effet :** poids inutile, non reproductibles. **Recommandation :** les retirer du suivi et ajouter les noms au `.gitignore`.

> **[Faible]** Historique Git : 2 commits (« first commit », « dude ») ; pas de fichier `LICENSE`. **Recommandation :** messages explicites, choisir une licence si le dépôt est destiné à être partagé.

### Tests et expérience développeur

> **[Moyen]** *Aucun test* (`test_path_candidates` vide). Aucune graine injectable : `std::random_device` est utilisé directement (`CombatSystem.cpp:6`, `ArenaSystem.cpp:13`), donc les combats ne sont pas reproductibles. **Effet :** les régressions comme celles ci-dessus ne sont pas détectables automatiquement. **Recommandation :** (1) tests unitaires sur `elementMultiplier`, `Player::criticalChance`, `Player::gainXp`, aller-retour `SaveSystem::save/load` (exécutable simple avec `assert`, sans dépendance) ; (2) un RNG partagé initialisable par graine ; (3) cible `make test`.

> **[Faible]** `Makefile:8` — `$(TARGET): $(SOURCES)` ne liste pas les headers : modifier un `.hpp` seul ne déclenche pas de recompilation (piège en cas de build incrémental). Pas de `.o` intermédiaires : tout est recompilé à chaque fois. Fichier en fins de ligne CRLF (59 des 69 fichiers suivis le sont, sans `.gitattributes`). **Recommandation :** objets séparés avec `-MMD -MP`, `.gitattributes` (`* text=auto eol=lf`).

> **[Faible]** `.vscode/tasks.json` — la tâche par défaut compile `${file}` seul : elle échoue dès qu'on l'applique à un `.cpp` du projet (includes relatifs, plusieurs unités). **Recommandation :** faire de « Build and run C++ project » (`make`) la tâche par défaut. Par ailleurs `src/*` mélange `#include "CharacterClass.hpp"` (dépend des `-I` du Makefile) et `#include "../../include/…"` : harmoniser sur une seule convention (`-Iinclude` + `#include "core/Player.hpp"`).

### Données / IA

Sans objet : aucun modèle ni jeu de données (les « Technique » sont du polymorphisme C++, pas de l'IA).

---

## Éléments à supprimer, fusionner, simplifier ou déplacer

| Élément | Rôle observé | Décision recommandée | Condition |
|---|---|---|---|
| `legacy/HelloWorld`, `legacy/jeu` | binaires ELF de l'ancien prototype | Retirer du suivi Git | Aucune dépendance dans le Makefile (vérifié) |
| `sauvegarde/savegame.txt` | sauvegarde de partie personnelle | Retirer du suivi, ignorer | Garder `sauvegarde/.gitkeep` |
| `legacy/*.cpp`, `legacy/*.hpp`, `legacy.md` | prototype d'origine, « non compilé » (`legacy/main.legacy.cpp` échoue d'ailleurs à `g++ -fsyntax-only`) | Archiver dans un tag Git (`legacy-v0`) puis supprimer du dossier **seulement si vous n'en avez plus besoin** | Votre décision : rôle de référence personnelle inconnu |
| `guide.md` | rôle non établi (non exploité dans cet audit) | Relire ; fusionner dans `readme.md` si redondant | À confirmer par l'auteur |
| `Terminal::title` + éléments dans `EnemyFactory.cpp` | voir constats | Déplacer (pas supprimer) | — |

## Scripts et outils

| Élément | Rôle observé | Évaluation | Recommandation |
|---|---|---|---|
| `Makefile` | build, `run`, `clean` | Fonctionne ; pas de dépendances headers ; CRLF | Ajouter `-MMD`, cible `test` |
| `.vscode/tasks.json` | 2 tâches (`${file}` et `make`) | La première est inadaptée au projet | Rendre `make` par défaut |
| `.vscode/settings.json` | associations de fichiers C++ | Générée par l'éditeur, sans enjeu | Peut être ignorée par Git si souhaité |
| CI / tests / linter | **introuvables dans ce dépôt** | — | Voir « Ajouts » |

## Refactorings recommandés

| Problème actuel | Structure actuelle → cible | Bénéfice | Difficulté | Nécessité |
|---|---|---|---|---|
| Fonctions d'éléments dans `EnemyFactory.cpp` | `EnemyFactory.cpp` → `src/core/Types.cpp` | Cohérence, testabilité | Faible | Recommandée |
| Quêtes dupliquées sur 3 héros | `Player::quests` ×3 → `GameState::quests` | Un seul état de vérité | Moyenne | Recommandée |
| Menus avec `ask(1,3)` en dur | constante 3 → `team.size()` | Supprime l'accès hors bornes | Faible | Essentielle |
| Données ennemis dans le code | tableaux inline → table/fichier de données | Équilibrage sans recompiler | Moyenne | Optionnelle |
| Lignes denses `Game.cpp` | one-liners → fonctions courtes + `clang-format` | Lisibilité | Faible | Recommandée |

## Ajouts potentiels

- **Essentiel :** gestion de l'EOF ; tests unitaires minimaux (éléments, critique, XP, aller-retour de sauvegarde).
- **Recommandé :** `CMakeLists.txt` minimal (ou retrait de la doc CMake) ; `.gitattributes` ; écran de défaite explicite ; message d'erreur si la sauvegarde échoue.
- **Optionnel :** RNG à graine (`--seed N`) pour rejouer un combat ; `clang-format`/`clang-tidy` locaux ; GitHub Actions gratuit pour `make && make test`.
- **Expérimental :** coût de compétences/ressources ; difficulté de l'arène par paliers.

## Solutions et dépendances

Tout peut rester **local, gratuit et sans dépendance** : `assert` (ou un exécutable de test maison) pour les tests, GNU Make ou CMake (open source), `clang-format`/`clang-tidy` (open source), `-fsanitize=address,undefined` (déjà disponible dans g++). Une bibliothèque de test (Catch2/doctest, header-only, open source) est possible mais pas nécessaire à cette taille.

## Dette technique

- **À corriger maintenant :** EOF, quête, Pile Face, accès hors bornes, incohérence de résurrection, documentation CMake/Ombre.
- **Peut attendre :** `GameState`, déplacement des fonctions d'éléments, sauvegarde atomique, validation du fichier.
- **Risque futur :** ajouter une 4ᵉ ou 5ᵉ classe / un 4ᵉ héros avec les bornes `1..3` codées en dur ; `Game.cpp` qui grossit en one-liners.
- **Peut rester simple :** pas de moteur de plugins, pas de GUI, `shared_ptr` pour les classes (un `unique_ptr` conviendrait mais l'impact est nul).

---

## Roadmap

### Phase 1 — Corrections rapides
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Gérer l'EOF dans `ask()` et `getline` | Élevée | Faible | Supprime un gel/une boucle infinie | `Terminal.hpp`, `Game.cpp:14` |
| Retirer `updateQuestProgress` du coup (ligne 87) | Moyenne | Faible | Quête correcte | `CombatSystem.cpp` |
| Corriger la chance de Pile Face | Moyenne | Faible | Équilibrage | `Game.cpp:28`, `PileFace.cpp` |
| Remplacer `ask(1,3)` par `team.size()` ou exiger 3 héros au chargement | Moyenne | Faible | Supprime l'UB | `CombatSystem.cpp`, `Game.cpp`, `SaveSystem.cpp` |
| Message « on ne fuit pas un boss » | Faible | Faible | UX | `CombatSystem.cpp` |

### Phase 2 — Nettoyage
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Retirer du suivi `savegame.txt` et binaires `legacy/` | Moyenne | Faible | Dépôt propre | `.gitignore`, `sauvegarde/`, `legacy/` |
| Corriger README/docs (CMake, Ombre, chemins) | Moyenne | Faible | Documentation fiable | `readme.md`, `docs/README_CLASSES.md`, `legacy.md` |
| `.gitattributes` + normaliser les fins de ligne | Faible | Faible | Diffs propres | racine |

### Phase 3 — Refactoring
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Règle unique de résurrection/mort | Moyenne | Moyenne | Cohérence de jeu | `Player.hpp`, `Game.cpp`, `CombatSystem.cpp` |
| Équipement : échange au lieu de suppression | Moyenne | Faible | Aucune perte d'objet | `Game.cpp:66`, `Types.hpp` |
| Déplacer les fonctions d'éléments | Faible | Faible | Cohérence | `Types.hpp`, `EnemyFactory.cpp` |
| `GameState` pour quêtes/progress | Faible | Moyenne | État unique | `Game`, `SaveSystem` |

### Phase 4 — Améliorations
| Tâche | Priorité | Difficulté | Impact | Fichiers |
|---|---|---|---|---|
| Tests unitaires + `make test` | Moyenne | Moyenne | Anti-régression | nouveau `tests/`, `Makefile` |
| Sauvegarde atomique + erreur visible | Moyenne | Faible | Fiabilité | `SaveSystem.cpp`, `Game.cpp:10` |
| Écran de défaite / reprise | Moyenne | Faible | Pas de partie bloquée | `Game.cpp` |
| Coût des techniques, arène par paliers | Faible | Moyenne | Équilibrage | `CombatSystem.cpp`, `ArenaSystem.cpp` |

### Phase 5 — Optimisation
Aucune optimisation justifiée. Éventuellement : `Makefile` avec objets séparés pour des rebuilds plus rapides (utile seulement si le projet grossit).

---

## Architecture recommandée

L'architecture actuelle est **adaptée** ; la cible est une évolution légère, pas une refonte :

```
include/core/    Types.hpp, Player.hpp, Terminal.hpp, GameState.hpp (nouveau)
src/core/        Types.cpp (nouveau : fonctions d'éléments)
src/characters/  inchangé
src/entities/    EnemyFactory.cpp (données seulement)
src/systems/     inchangé (Game.cpp reformaté)
tests/           tests unitaires (nouveau)
data/ (optionnel) ennemis en fichier de données
docs/            alignés sur le code
```

## Résumé décisionnel

- **À conserver :** hiérarchie de classes + factory, découpage en systèmes, sérialisation `std::quoted`, bestiaire, build sans warning.
- **À améliorer :** gestion des entrées (EOF), validation de sauvegarde, messages utilisateur.
- **À refactoriser :** fonctions d'éléments, état des quêtes, règle de mort/résurrection, équipement.
- **À supprimer/remplacer :** `savegame.txt` et binaires `legacy/` du suivi Git ; docs obsolètes (`legacy.md`, passages CMake/Ombre).
- **À ajouter :** tests minimaux, `CMakeLists.txt` (ou retrait de la doc), `.gitattributes`.
- **À surveiller :** bornes à 3 héros codées en dur, croissance de `Game.cpp`.
- **Priorités absolues :** (1) EOF, (2) quête/Pile Face/résurrection, (3) bornes de `load()`, (4) documentation de build.

## Questions ouvertes / faits indéterminables

- Mort permanente ou réanimation : intention de design non documentée.
- La technique sans coût est-elle voulue ? Le bonus Pile Face (35 + 35) est-il voulu ?
- `guide.md` (11 Ko) et `legacy/*` : rôle exact non établi par cet audit.
- Comportement sous Windows/macOS/clang et rendu visuel des cadres Unicode : non testés.
- Les noms (« Maladie », « Rachid ») dans `savegame.txt` sont-ils des données volontairement publiées ? À confirmer par l'auteur.
