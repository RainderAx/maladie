# Journal des améliorations

Ce journal décrit uniquement les changements apportés à cette version. Les sauvegardes de campagne V3 et le mode terminal d’origine sont conservés.

## 1. Boucle roguelite et progression des salles

- **Nom de la modification :** Boucle d’expédition à cinq étapes.
- **Type :** Ajout.
- **Fichiers concernés :** `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Ajout d’un modèle de run indépendant de l’interface, d’une sélection reproductible de rencontres tirées du bestiaire et réparties en paliers de puissance, de quatre salles normales suivies du boss, d’actions de combat, de victoires, de fragments et d’un choix de récompense entre potion et repos. Les héros morts restent indisponibles pendant la run ; une équipe sans survivant met fin à celle-ci. Le repos soigne uniquement les survivants.
- **Justification :** Le dépôt proposait une aventure RPG en terminal mais aucune boucle roguelite relançable.
- **Source :** Conception approuvée dans `docs/superpowers/specs/2026-11-13-roguelite-graphique-nemesis-design.md` ; bestiaire existant dans `src/entities/EnemyFactory.cpp` ; analyse des règles déjà utilisées par le projet.
- **Impact :** Ajoute une progression courte de run qui réutilise les classes, héros, ennemis et objets existants. La graine et l’index de salle déterminent l’adversaire sélectionné.

## 2. Rivaux Némésis et butin

- **Nom de la modification :** Promotion d’un ennemi ayant tué un héros en rival persistant.
- **Type :** Ajout.
- **Fichiers concernés :** `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `include/systems/CombatRules.hpp`, `src/systems/CombatRules.cpp`, `src/systems/CombatSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** À la mort d’un héros, l’ennemi gagne l’arme et l’armure équipées, stocke ces objets et augmente son attaque de la valeur de l’arme, sa défense de celle de l’armure, ses PV max de `10 × niveau` et sa chance critique de 2 points (plafond 60 %). Plusieurs morts par le même rival cumulent les bonus. Un rival peut réapparaître dans une salle ultérieure ; sa défaite rend tout son équipement au sac d’un survivant.
- **Justification :** Réalise la mécanique inspirée du système Némésis demandée, tout en gardant des règles métier testables sans interface graphique.
- **Source :** Barème explicitement approuvé par l’utilisateur ; modèle `Player`/`Enemy` et calcul de dégâts du combat d’origine ; fonctions C++ conçues à partir de l’analyse du dépôt.
- **Impact :** Les morts influencent les rencontres ultérieures et rendent les rivaux plus dangereux ; leurs pièces d’équipement peuvent être récupérées.

## 3. Règles de dégâts partagées

- **Nom de la modification :** Extraction du calcul de dégâts du combat terminal.
- **Type :** Refactorisation.
- **Fichiers concernés :** `include/systems/CombatRules.hpp`, `src/systems/CombatRules.cpp`, `src/systems/CombatSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Les dégâts normaux, techniques, élémentaires, défenses et coups critiques sont calculés par des fonctions communes sans entrée/sortie. Le combat terminal appelle maintenant ces fonctions au lieu de garder sa formule intégrée dans l’affichage.
- **Justification :** Le nouveau combat visuel doit respecter les formules existantes sans dupliquer leur logique, et ces règles doivent pouvoir être testées sans ouvrir de fenêtre.
- **Source :** Formules observées dans `src/systems/CombatSystem.cpp` avant extraction ; règles élémentaires et principes de séparation modèle/présentation.
- **Impact :** Réduit la duplication et rend les dégâts accessibles à l’interface Raylib et à la suite de tests, en conservant les multiplicateurs et le minimum de dégâts existants.

## 4. Améliorations permanentes payées en fragments

- **Nom de la modification :** Atelier de progression entre les runs.
- **Type :** Ajout.
- **Fichiers concernés :** `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `src/ui/RaylibGame.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Les fragments achetés en run financent `+10 PV max` (coût initial 4), `+1 ATQ` ou `+1 DEF` (coût initial 6). Le prix d’une même statistique augmente de 2 à chaque achat et les caps par héros sont respectivement `+50 PV`, `+8 ATQ` et `+8 DEF`. Ces bonus sont repris au début des runs suivantes ; niveaux, PV courants, inventaire et équipement de run sont réinitialisés.
- **Justification :** Ajoute la méta-progression demandée, afin que les fragments donnent un motif de relance sans retirer le risque de mort de chaque expédition.
- **Source :** Réglages proposés puis approuvés dans l’échange de conception ; règles métier validées par tests.
- **Impact :** Le roster progresse durablement ; les coûts croissants et plafonds bornent l’économie.

## 5. Sauvegardes roguelite séparées

- **Nom de la modification :** Sauvegarde versionnée du profil et des checkpoints de run.
- **Type :** Ajout.
- **Fichiers concernés :** `include/systems/RogueliteSaveSystem.hpp`, `src/systems/RogueliteSaveSystem.cpp`, `src/systems/RogueliteSystem.cpp`, `.gitignore`, `tests/roguelite_tests.cpp`.
- **Description :** Ajout de formats texte distincts `ROGUELITE_PROFILE_V1` et `ROGUELITE_RUN_V1`. Le profil conserve roster, fragments et rangs ; la run conserve graine, équipe, salle, état de récompense en attente, rivaux et objets volés. Un chargement invalide ne remplace pas le profil/run déjà en mémoire. Les fichiers roguelite générés sont ignorés par Git.
- **Justification :** Éviter toute conversion ou écrasement de `sauvegarde/savegame.txt`, et permettre la reprise d’un checkpoint.
- **Source :** Format V3 de campagne observé dans `src/systems/SaveSystem.cpp` et fichier fourni `sauvegarde/savegame.txt` ; bibliothèque standard C++17 (`<fstream>`, `<filesystem>`, `<iomanip>`).
- **Impact :** Le mode roguelite ne modifie pas la sauvegarde de campagne. Quitter pendant un combat reprend au début de la salle sauvegardée.

## 6. Fabrique de héros réutilisable

- **Nom de la modification :** Centralisation des héros de départ.
- **Type :** Refactorisation.
- **Fichiers concernés :** `include/core/HeroFactory.hpp`, `src/core/HeroFactory.cpp`, `src/systems/Game.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Extraction de la construction du nom, de la classe, de l’élément, des statistiques, des armes, armures et potions dans `createHero`. Le terminal délègue à la même fabrique que le roster graphique.
- **Justification :** Empêcher la divergence entre héros créés en terminal et héros de la nouvelle interface, sans modifier le choix de classes ni les équipements existants.
- **Source :** Valeurs et choix de départ précédemment définis dans `src/systems/Game.cpp`, réutilisés et couverts par des tests.
- **Impact :** Une seule configuration de départ est maintenue ; la logique de l’ancienne campagne reste disponible.

## 7. Interface de jeu Raylib

- **Nom de la modification :** Interface de bureau pour l’expédition roguelite.
- **Type :** Ajout.
- **Fichiers concernés :** `include/ui/RaylibGame.hpp`, `src/ui/RaylibGame.cpp`, `main.cpp`, `src/terminal_main.cpp`.
- **Description :** Le lancement principal ouvre par défaut une fenêtre Raylib : menu, création du roster, atelier, carte des salles, combat, récompenses et résumé. Les contrôles comprennent la souris, les touches `1`–`3` pour le héros actif et `A`/`S`/`P` pour attaque/technique/potion. `Échap` rétablit le checkpoint de la salle. Un lanceur terminal séparé conserve l’aventure existante.
- **Justification :** Répond à la demande de remplacer l’expérience principale en terminal par une interface visuelle sans supprimer l’ancien mode.
- **Source :** Choix utilisateur de Raylib native C++ ; API Raylib 5.5 vérifiée dans le package utilisé pour le build ; modèle métier séparé de l’affichage.
- **Impact :** Le jeu roguelite se joue dans une fenêtre 2D redimensionnable. Le mode terminal demeure compilable sans Raylib via sa cible autonome. La cible graphique requiert Raylib 5.5.

## 8. Build, tests et documentation du projet

- **Nom de la modification :** Cibles CMake/Make et guide de lancement actualisé.
- **Type :** Ajout et refactorisation.
- **Fichiers concernés :** `CMakeLists.txt`, `Makefile`, `readme.md`, `tests/roguelite_tests.cpp`.
- **Description :** Ajout des cibles `chroniques_de_la_faille`, `chroniques_de_la_faille_terminal` et `roguelite_tests`, ainsi que de l’intégration CTest. `make`, `make run`, `make terminal`, `make run-terminal` et `make test` sont documentés. Les tests couvrent achats, reset de run, morts, bonus et récupération Némésis, boss, récompenses, sauvegardes, graine, progression de difficulté et lecture de la sauvegarde V3 existante.
- **Justification :** Le README mentionnait déjà CMake alors qu’aucun `CMakeLists.txt` n’existait ; il fallait des commandes vérifiables pour les deux modes.
- **Source :** Configuration CMake/GNU Make du dépôt ; tests C++17 sans framework externe.
- **Impact :** Le modèle et ses tests se construisent sans Raylib avec `BUILD_GRAPHICAL_GAME=OFF` ; l’exécutable graphique demande Raylib 5.5. Le README décrit les commandes, la dépendance, les contrôles et les chemins de sauvegarde.

## 9. Conception approuvée et traçabilité

- **Nom de la modification :** Spécification de conception et journal des changements.
- **Type :** Ajout.
- **Fichiers concernés :** `docs/superpowers/specs/2026-11-13-roguelite-graphique-nemesis-design.md`, `AMELIORATIONS.md`.
- **Description :** Ajout de la conception approuvée avant implémentation, puis mise à jour de sa description d’interface pour refléter les écrans, dimensions et contrôles effectivement livrés. Ce fichier journal consigne les changements du présent lot.
- **Justification :** Conserver les choix approuvés et rendre vérifiables la portée et les motivations des modifications.
- **Source :** Décisions de conception validées par l’utilisateur et état réel du dépôt après implémentation.
- **Impact :** Fournit une référence de conception et une trace des fichiers et effets concernés.

## 10. Équilibrage progressif des rencontres

- **Nom de la modification :** Paliers de puissance pour éviter un premier combat de fin d’arène.
- **Type :** Correction.
- **Fichiers concernés :** `src/systems/RogueliteSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Le premier prototype tirait uniformément dans les 20 ennemis d’arène, ce qui pouvait sélectionner dès l’ouverture un adversaire de rang élevé. Les ennemis de base sont maintenant triés par puissance puis répartis en quatre tranches de cinq ; chaque salle tire de façon reproductible dans sa tranche, de la plus faible à la plus forte.
- **Justification :** Le smoke test Xvfb a montré une première rencontre à 218 PV max, 28 ATQ et 16 DEF, qui a éliminé le roster avant la première récompense. Le défaut venait du tirage global, pas d’une erreur d’interaction.
- **Source :** `src/entities/EnemyFactory.cpp`, `src/systems/ArenaSystem.cpp` et les valeurs d’arène documentées dans `docs/BESTIAIRE.md` ; test de balance observé en échec avant le correctif, puis réussi après.
- **Impact :** La première salle utilise le palier d’entrée ; la puissance attendue progresse ensuite par salles tout en gardant un choix aléatoire reproductible.

## Vérifications exécutées

- `make test` : réussi ; CTest indique **1/1** test passé.
- Build CMake Release des cibles graphique (`chroniques_de_la_faille`), terminale (`chroniques_de_la_faille_terminal`) et `roguelite_tests` : réussi avec Raylib 5.5 installée dans l’environnement de travail.
- CTest après build graphique et terminal : **100 % des tests passés**.
- Smoke test GUI sous Xvfb/xdotool : fenêtre affichée, roster et run sauvegardés, entrée en combat visible dans une capture, attaque et interface des héros/actions confirmées.
- Smoke test de combat après correction d’équilibrage : deux actions techniques ont suffi à vaincre l’ennemi de la première salle ; le checkpoint avec récompense en attente a été retrouvé dans le fichier de run.
- La campagne V3 fournie a été chargée avec succès par le test. Une partie complète jusqu’au boss n’a pas été exécutée dans l’interface.
