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
- **Description :** Les formats courants sont `ROGUELITE_PROFILE_V2` et `ROGUELITE_RUN_V2` ; les formats V1 restent lisibles avec les nouveaux champs initialisés par défaut. Le profil conserve roster, fragments, rangs et armurerie ; la run conserve graine, équipe, salle, état de récompense en attente, morts de salle, extractions, rivaux et objets volés. Un chargement invalide ne remplace pas le profil/run déjà en mémoire. Les fichiers roguelite générés sont ignorés par Git.
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
- **Description :** Ajout des cibles `chroniques_de_la_faille`, `chroniques_de_la_faille_terminal` et `roguelite_tests`, ainsi que de l’intégration CTest. Raylib est facultative à la configuration : sans elle, `make` construit le mode terminal et les tests. Les tests couvrent achats, équipement, forge, extraction, morts, bonus et récupération Némésis, boss, récompenses, sauvegardes, graine, progression de difficulté et une fixture V3 dédiée (`tests/fixtures/campaign_v3.txt`), indépendante de la sauvegarde personnelle.
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

## 11. Économie roguelite, armurerie et extraction

- **Nom de la modification :** Barème de fragments lié au danger et à la maîtrise.
- **Type :** Correction et équilibrage.
- **Fichiers concernés :** `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Les fragments dépendent maintenant de l’XP de l’adversaire : rencontre normale `1 + min(2, XP/100)`, bonus de `+1` pour une Némésis, boss `4 + min(2, XP/250)`. Une victoire sans mort dans la salle rapporte `+1` fragment de maîtrise; le compteur est réinitialisé à chaque nouvelle salle.
- **Justification :** L’ancien gain fixe ne récompensait ni la difficulté réelle ni une victoire propre.
- **Source :** Formules proposées et approuvées dans `docs/superpowers/specs/2026-11-13-roguelite-equipement-extraction-design.md`, puis vérifiées par tests métier.
- **Impact :** La récompense rend les choix de combat plus significatifs sans rendre l’économie illimitée; fragments déjà acquis conservés même après défaite.

- **Nom de la modification :** Armurerie persistante et achat d’équipement au camp.
- **Type :** Ajout.
- **Fichiers concernés :** `include/core/Types.hpp`, `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `src/systems/RogueliteSaveSystem.cpp`, `src/ui/RaylibGame.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Ajout d’un catalogue d’armes/armures achetables contre 6 ou 9 fragments, d’une armurerie de profil et de l’affectation d’une pièce à un héros. La pièce remplacée retourne à l’armurerie. Les onglets Améliorations et Armurerie restent tous deux cliquables; les profils/runs V2 sérialisent les nouvelles pièces et continuent de charger les V1.
- **Justification :** L’atelier ne proposait auparavant que des augmentations de statistiques, et les décisions d’équipement pouvaient détruire une pièce.
- **Source :** Catalogue de l’échoppe de campagne dans `src/systems/Game.cpp`; règles approuvées dans la spécification roguelite et tests de permutation/sérialisation.
- **Impact :** Le joueur choisit son chargement de départ entre les expéditions; les objets restent récupérables et les anciens profils restent compatibles.

- **Nom de la modification :** Forge de run et extraction d’une pièce de butin par checkpoint sûr.
- **Type :** Ajout.
- **Fichiers concernés :** `include/core/Types.hpp`, `include/systems/RogueliteSystem.hpp`, `src/systems/RogueliteSystem.cpp`, `src/systems/RogueliteSaveSystem.cpp`, `src/ui/RaylibGame.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** À la récompense d’une salle normale, un héros vivant peut améliorer son arme ou son armure de `+2` par rang, jusqu’à trois rangs, pour `45 + 30 × rang actuel` or. Le joueur peut aussi extraire au plus une pièce non équipée d’un sac vers l’armurerie persistante. L’équipement remplacé en run demeure dans le sac; les améliorations du run ne deviennent permanentes qu’après victoire finale.
- **Justification :** Répondre au besoin de conserver une deuxième arme et ajouter un choix de progression distinct des fragments et de la récompense Potion/Repos.
- **Source :** Règles approuvées dans la spécification; implémentation et limites couvertes par les tests roguelite.
- **Impact :** Les objets extraits et fragments survivent à une défaite; le butin non extrait reste exposé au risque. Le choix de récompense de salle est conservé.

## 12. Corrections de bugs et fiabilité

- **Nom de la modification :** Cohérence des morts, de l’expérience et des bonus d’équipement.
- **Type :** Correction.
- **Fichiers concernés :** `include/core/Player.hpp`, `include/core/Types.hpp`, `src/systems/RogueliteSystem.cpp`, `src/systems/RogueliteSaveSystem.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Les PV d’un héros sont bornés à zéro; une run contenant un héros mort reste sérialisable; un héros mort ne reçoit plus d’XP et ne ressuscite pas par montée de niveau. La Némésis conserve ses PV réellement restants après croissance/réapparition. Un emplacement d’équipement vide ne fournit plus de bonus élémentaire ou critique, et un emplacement vide ne génère pas de butin fantôme.
- **Justification :** Éliminer la perte silencieuse de sauvegarde et les divergences entre mort, progression et équipement.
- **Source :** Constats C1–C4 de `audit(1).md`; tests de régression ajoutés au harnais existant.
- **Impact :** État de combat et de sauvegarde cohérents; les morts restent définitives pendant une run et les bonus correspondent aux objets réellement équipés.

- **Nom de la modification :** Correctifs ciblés de la campagne terminale.
- **Type :** Correction.
- **Fichiers concernés :** `include/core/Terminal.hpp`, `include/core/Player.hpp`, `src/core/HeroFactory.cpp`, `src/systems/ArenaSystem.cpp`, `include/systems/ArenaSystem.hpp`, `src/systems/CombatSystem.cpp`, `src/systems/Game.cpp`, `src/systems/SaveSystem.cpp`, `main.cpp`, `src/terminal_main.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** EOF termine proprement la saisie; les quêtes progressent à la victoire; fuir un boss renvoie un résultat de fuite distinct; le bonus initial excessif de Pile Face est retiré; le chargeur refuse une équipe de moins de trois héros sans remplacer l’état en mémoire. L’arène ouvre progressivement les ennemis par vagues. En campagne, l’équipement remplacé revient dans le sac; au chargement d’une équipe entièrement K.O., le jeu annonce la fin de campagne et demande explicitement de créer une nouvelle équipe ou de quitter.
- **Justification :** Corriger les comportements confirmés par l’audit sans supprimer la campagne ni ses sauvegardes V3.
- **Source :** Constats de `audit(1).md`, reproductions et tests de non-régression dans `tests/roguelite_tests.cpp`.
- **Impact :** Saisie et progression plus prévisibles; pas de perte d’arme lors d’une permutation; une sauvegarde de campagne terminée ne mène plus silencieusement à un hub inutilisable.

- **Nom de la modification :** Sauvegardes roguelite atomiques et remontée des erreurs UI.
- **Type :** Correction et fiabilisation.
- **Fichiers concernés :** `src/systems/RogueliteSaveSystem.cpp`, `src/ui/RaylibGame.cpp`, `tests/roguelite_tests.cpp`.
- **Description :** Le profil/run est validé avant écriture, écrit dans un fichier temporaire puis remplacé; un échec ne tronque pas le fichier valide. Tous les appels UI passent par des wrappers qui affichent l’erreur dans l’interface et l’écrivent dans `stderr`, y compris à la fermeture où le message ne pourrait plus être dessiné.
- **Justification :** L’audit reproduisait la troncature possible et constatait que les retours de sauvegarde étaient ignorés ou affichés après la fermeture de la fenêtre.
- **Source :** Constat F1 de `audit(1).md`; tests d’échec d’écriture; comportement `std::filesystem`/flux C++17.
- **Impact :** Une panne d’écriture devient visible et n’endommage pas la dernière sauvegarde valide.

- **Nom de la modification :** Build terminal sans Raylib et fixture de campagne autonome.
- **Type :** Correction et amélioration de l’outillage.
- **Fichiers concernés :** `CMakeLists.txt`, `Makefile`, `tests/fixtures/campaign_v3.txt`, `tests/roguelite_tests.cpp`.
- **Description :** La configuration construit le jeu terminal et les tests quand Raylib est absente; le jeu graphique reste disponible lorsque Raylib 5.5 est installée. Le test V3 charge une fixture dédiée plutôt que `sauvegarde/savegame.txt` suivi par Git.
- **Justification :** Éviter l’échec de configuration sur une machine sans dépendance graphique et le couplage des tests à une sauvegarde personnelle.
- **Source :** Constats B1/B2 de `audit(1).md`; options CMake/GNU Make du projet.
- **Impact :** Build de base plus accessible et tests déterministes qui ne lisent ni n’écrasent la sauvegarde personnelle.

- **Nom de la modification :** Conception et plan d’implémentation de la boucle d’équipement.
- **Type :** Ajout documentaire.
- **Fichiers concernés :** `docs/superpowers/specs/2026-11-13-roguelite-equipement-extraction-design.md`, `docs/superpowers/plans/2026-11-14-roguelite-equipement-extraction-plan.md`, `AMELIORATIONS.md`.
- **Description :** Conservation des choix approuvés (barème de points, catalogue, forge, extraction, compatibilité des sauvegardes), des réponses aux questions ouvertes de l’audit et du plan de tests/implémentation.
- **Justification :** Assurer la traçabilité des décisions sans inventer de nouvelle mécanique hors du périmètre approuvé.
- **Source :** Approbation explicite de l’utilisateur et audit joint.
- **Impact :** Les règles proposées et leurs conséquences sont documentées et vérifiables par rapport au code livré.

## Vérifications exécutées

- `cmake --build build -j2` et `ctest --test-dir build --output-on-failure` : réussis après les derniers changements; **1/1** test CTest passé.
- `cmake --build build-raylib -j2` et `ctest --test-dir build-raylib --output-on-failure` : réussis avec Raylib 5.5; cible graphique, terminale et tests compilés; **1/1** test CTest passé.
- Smoke test Raylib sous Xvfb/xdotool : onglet Armurerie ouvert, achat à 6 fragments, permutation de l’arme et conservation de l’ancienne pièce confirmés par relecture du profil V2.
- Smoke test d’échec d’écriture : un chemin de sauvegarde rendu volontairement impossible a produit le message d’erreur à l’écran et dans `stderr`.
- Tests d’intégration campagne : permutation conservée après sauvegarde/rechargement; équipe entièrement K.O. propose de recommencer ou quitter; EOF se termine proprement; tests de quête, fuite de boss, roster, arène et Pile Face passent.
- Fixture V3 dédiée chargée par test. Une expédition roguelite complète jusqu’au boss, et les plateformes Windows/macOS, n’ont pas été testées.
