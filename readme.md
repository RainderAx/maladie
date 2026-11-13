# Chroniques de la Faille

RPG d’équipe en C++17. Le lancement principal ouvre maintenant un jeu de bureau 2D avec Raylib ; l’aventure historique en terminal reste accessible dans un exécutable séparé.

## Contenu du mode roguelite

Une expédition se joue avec un roster persistant de trois héros. Le joueur traverse quatre salles de combat, choisit après chacune une potion ou un repos, puis affronte le boss. Les héros tués ne reviennent pas pendant la run ; une équipe entièrement vaincue met fin à l’expédition.

Quand un ennemi tue un héros, il vole son arme et son armure, puis devient un rival Némésis : il gagne l’ATQ de l’arme, la DEF de l’armure, `10 × niveau` PV max et 2 points de critique (plafond 60 %). Un rival peut réapparaître, cumuler l’équipement de plusieurs victimes et rendre tout son butin s’il est vaincu.

Les salles normales donnent chacune 1 fragment de Faille et le boss en donne 4. À l’écran d’entraînement, avant une nouvelle run, ces fragments achètent des bonus permanents : `+10 PV max` pour 4 fragments, `+1 ATQ` ou `+1 DEF` pour 6 ; le prix de chaque caractéristique augmente de 2 à chaque achat de cette caractéristique. Les caps par héros sont `+50 PV`, `+8 ATQ` et `+8 DEF`. Les niveaux, les PV courants, l’inventaire de run et les équipements de run repartent de la base au début de chaque expédition ; le roster et ses améliorations restent.

## Interface et commandes

L’interface Raylib présente le roster, le chemin des salles, les statistiques de combat, les actions, les rivaux et l’écran d’entraînement. Les boutons se contrôlent à la souris. Pendant un combat, sélectionner un portrait ou appuyer sur `1`, `2` ou `3` choisit le héros actif ; `A` attaque, `S` lance sa technique, `P` utilise une potion et `Échap` revient au checkpoint de la salle (le combat recommencera au début de cette salle).

## Compilation

La version graphique demande **Raylib 5.5** (package CMake `raylib`) et CMake 3.16 ou plus.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target chroniques_de_la_faille
./build/chroniques_de_la_faille
```

Les commandes GNU Make équivalentes sont `make` puis `make run`. Pour lancer uniquement le mode terminal sans dépendance Raylib :

```bash
make terminal
make run-terminal
```

Le mode terminal autonome peut également être construit avec `cmake -S . -B build -DBUILD_GRAPHICAL_GAME=OFF`, puis `cmake --build build --target chroniques_de_la_faille_terminal`. L’option `--terminal` est aussi acceptée par l’exécutable graphique.

Pour compiler et exécuter les tests du modèle métier sans construire l’interface graphique :

```bash
make test
```

Le test CTest utilise le fichier de sauvegarde de campagne V3 fourni dans `sauvegarde/savegame.txt` pour vérifier que l’ancienne campagne reste lisible. Pour le binaire graphique, installez Raylib 5.5 et ses dépendances de bureau selon les instructions d’installation officielles de Raylib, puis assurez-vous que `raylib-config.cmake` est visible par CMake.

## Sauvegardes

La sauvegarde de l’aventure classique demeure `sauvegarde/savegame.txt` et conserve son format V3. Le mode roguelite utilise deux fichiers texte versionnés distincts : `sauvegarde/roguelite_profile.txt` (roster, fragments et améliorations permanentes) et `sauvegarde/roguelite_run.txt` (graine, salle, héros de la run et rivaux). Les sauvegardes de run sont faites aux checkpoints ; une interruption au milieu d’un combat le relance au début de la salle. Les sauvegardes roguelite sont ignorées par Git.

## Architecture

| Dossier | Rôle |
|---|---|
| `include/core` et `src/core` | Modèles communs et fabrique des héros. |
| `include/characters` et `src/characters` | Dix classes jouables et leurs compétences. |
| `include/entities` et `src/entities` | Ennemis et bestiaire. |
| `include/systems` et `src/systems` | Combat partagé, aventure, run roguelite, Némésis et sauvegardes. |
| `include/ui` et `src/ui` | Interface de bureau Raylib, isolée des règles métier. |
| `tests` | Tests automatisés du modèle, des règles et de la persistance. |
| `sauvegarde` | Progression générée automatiquement et sauvegarde V3 d’exemple. |
| `legacy` | Ancien prototype conservé à titre de référence, non compilé. |
| `docs` | Documentation des classes, du bestiaire et de la conception du roguelite. |

La liste des classes et le bestiaire d’origine restent documentés dans [`docs/README_CLASSES.md`](docs/README_CLASSES.md) et [`docs/BESTIAIRE.md`](docs/BESTIAIRE.md). Les règles de conception de cette première version se trouvent dans [`docs/superpowers/specs/2026-11-13-roguelite-graphique-nemesis-design.md`](docs/superpowers/specs/2026-11-13-roguelite-graphique-nemesis-design.md).
