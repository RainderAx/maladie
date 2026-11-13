# Architecture des classes

Le projet est organisé autour de responsabilités séparées. Le programme ne dépend plus d’un fichier monolithique : `main.cpp` instancie `Game`, puis `Game` coordonne les systèmes spécialisés.

## Arborescence

```text
rpg-terminal/
├── main.cpp
├── Makefile
├── CMakeLists.txt
├── include/
│   ├── core/
│   │   ├── Player.hpp
│   │   ├── Terminal.hpp
│   │   └── Types.hpp
│   ├── characters/
│   │   └── CharacterClasses.hpp
│   ├── entities/
│   │   └── EnemyFactory.hpp
│   └── systems/
│       ├── ArenaSystem.hpp
│       ├── CombatSystem.hpp
│       ├── DungeonSystem.hpp
│       ├── Game.hpp
│       └── SaveSystem.hpp
├── src/
│   ├── characters/CharacterClasses.cpp
│   ├── entities/EnemyFactory.cpp
│   └── systems/
│       ├── ArenaSystem.cpp
│       ├── CombatSystem.cpp
│       ├── DungeonSystem.cpp
│       ├── Game.cpp
│       └── SaveSystem.cpp
├── sauvegarde/
├── docs/
└── legacy/
```

## Hiérarchie des personnages

Les héritages demandés sont conservés dans `include/characters/CharacterClasses.hpp`.

| Classe | Hérite de | Responsabilité |
|---|---|---|
| Guerrier | `CharacterClass` | Base martiale |
| Berserker | `Guerrier` | Dégâts offensifs |
| Paladin | `Berserker` | Dégâts et soin défensif |
| Prêtre | `CharacterClass` | Soin sacré |
| Oracle | `Prêtre` | Magie et soutien |
| Templier | `Oracle` | Défense sacrée |
| Vampire | `CharacterClass` | Drain de vie |
| Sanguinaire | `Vampire` | Attaque et régénération |
| Seigneur Nocturne | `Sanguinaire` | Puissance de fin de lignée |
| Pile Face | `CharacterClass` | Chance et coups critiques |
| BossDeFin | `Seigneur Nocturne` | Classe réservée aux boss |

`BossDeFin` n’est pas proposé au héros. Il est utilisé pour la Reine aux Milles-Visages. La fonction `makeCharacterClass` centralise la création polymorphe des classes à partir de leur identifiant.

## Responsabilités des modules

`Player` possède uniquement l’état du héros et ses opérations fondamentales : attaque calculée, défense, chance critique, expérience, niveau et inventaire. `EnemyFactory` contient les données de contenu des ennemis, des boss et du roster d’arène. `CombatSystem` contient la boucle de combat et les effets des compétences. `DungeonSystem` orchestre les salles et les conditions de déblocage. `ArenaSystem` gère les vagues infinies. `SaveSystem` sérialise le héros et la progression. `Game` reste le contrôleur des menus et des transitions.

## Compilation

Avec Make :

```bash
make
make run
```

Avec CMake :

```bash
cmake -S . -B build
cmake --build build
./build/chroniques_de_la_faille
```

Sur Windows, Visual Studio peut compiler tous les fichiers listés par CMake. Le dossier `sauvegarde` est créé automatiquement au lancement si nécessaire.

## Fichiers individuels des classes

Chaque classe possède maintenant son propre contrat public et son propre fichier d’implémentation. Pour ajouter une méthode à une classe, déclarer la méthode dans le `.hpp`, puis écrire son corps dans le `.cpp` correspondant.

| Classe | Header | Implémentation |
|---|---|---|
| Classe de base | `include/characters/CharacterClass.hpp` | `src/characters/CharacterClass.cpp` |
| Guerrier | `include/characters/Guerrier.hpp` | `src/characters/Guerrier.cpp` |
| Berserker | `include/characters/Berserker.hpp` | `src/characters/Berserker.cpp` |
| Paladin | `include/characters/Paladin.hpp` | `src/characters/Paladin.cpp` |
| Prêtre | `include/characters/Priest.hpp` | `src/characters/Priest.cpp` |
| Oracle | `include/characters/Oracle.hpp` | `src/characters/Oracle.cpp` |
| Templier | `include/characters/Templier.hpp` | `src/characters/Templier.cpp` |
| Vampire | `include/characters/Vampire.hpp` | `src/characters/Vampire.cpp` |
| Sanguinaire | `include/characters/Sanguinaire.hpp` | `src/characters/Sanguinaire.cpp` |
| Seigneur Nocturne | `include/characters/SeigneurNocturne.hpp` | `src/characters/SeigneurNocturne.cpp` |
| Pile Face | `include/characters/PileFace.hpp` | `src/characters/PileFace.cpp` |
| BossDeFin | `include/characters/BossDeFin.hpp` | `src/characters/BossDeFin.cpp` |
| Factory | `include/characters/CharacterFactory.hpp` | `src/characters/CharacterFactory.cpp` |

### Ajouter une méthode

Par exemple, pour ajouter une compétence au Berserker, modifier `Berserker.hpp` :

```cpp
int rageBonus() const;
```

Puis ajouter son implémentation dans `Berserker.cpp` :

```cpp
int Berserker::rageBonus() const {
    return 10;
}
```

La factory n’a besoin d’être modifiée que lorsqu’une nouvelle classe est créée, pas lorsqu’une méthode est ajoutée à une classe existante.

## Éléments et équipe

Lors de la création, chaque héros choisit un élément parmi **Eau**, **Feu**, **Plante** et **Ombre**. Les trois héros peuvent avoir des éléments différents, ce qui permet de construire une équipe complémentaire.

| Élément offensif | Avantage contre | Résistance contre |
|---|---|---|
| Feu | Plante | Eau |
| Plante | Eau | Feu |
| Eau | Feu | Plante |
| Ombre | Eau | — |

Un avantage élémentaire multiplie les dégâts par **1,5**. Une résistance les réduit à **0,75**. Les autres associations restent neutres. Les mêmes règles s’appliquent aux attaques ennemies contre les héros.

Les armes et armures possèdent également un élément. Lorsqu’un équipement possède le même élément que son porteur, le héros obtient un bonus de **+4 ATK/DEF par équipement assorti**. Une arme assortie ajoute aussi **+5 % de chance critique**. Le bonus est recalculé automatiquement lorsqu’un objet est équipé.

Le jeu crée maintenant une équipe composée de trois héros. À chaque tour, le joueur choisit le héros actif, puis son action. Les héros K.O. ne peuvent plus être sélectionnés, les ennemis peuvent cibler aléatoirement un membre vivant, et les récompenses d’XP sont distribuées à toute l’équipe.

## Compétences visibles et méthodes C++

Chaque classe possède maintenant une méthode nommée dédiée. La méthode générique `skillDamage(int level)` délègue à cette méthode, ce qui permet au système de combat de rester polymorphe tout en laissant une méthode claire à modifier.

| Classe | Compétence affichée | Méthode dans le `.hpp/.cpp` |
|---|---|---|
| Guerrier | Frappe tellurique | `frappeTellurique(int level)` |
| Berserker | Rage des braises | `rageDesBraises(int level)` |
| Paladin | Égide radieuse | `egideRadieuse(int level)` |
| Prêtre | Lumière réparatrice | `lumiereReparatrice(int level)` |
| Oracle | Présage astral | `presageAstral(int level)` |
| Templier | Sceau du jugement | `sceauDuJugement(int level)` |
| Vampire | Morsure sanguine | `morsureSanguine(int level)` |
| Sanguinaire | Marée écarlate | `mareeEcarlate(int level)` |
| Seigneur Nocturne | Éclipse prédatrice | `eclipsePredatrice(int level)` |
| Pile Face | Destin hasardeux | `destinHasardeux(int level)` |
| BossDeFin | Anéantissement de la Faille | `aneantissementDeLaFaille(int level)` |

Exemple pour le Guerrier :

```cpp
// include/characters/Guerrier.hpp
int frappeTellurique(int level) const;

// src/characters/Guerrier.cpp
int Guerrier::frappeTellurique(int level) const {
    return 16 + level * 3;
}

int Guerrier::skillDamage(int level) const {
    return frappeTellurique(level);
}
```

Pendant le combat, l’action **Technique** affiche le nom de la méthode C++ appelée. La fiche de chaque héros affiche également la méthode correspondante.

## Neutralité d’Ombre

Ombre ne possède désormais aucune force ni résistance élémentaire. Toute interaction impliquant Ombre retourne **1,0×**. Les interactions spéciales sont limitées à Feu contre Plante, Plante contre Eau et Eau contre Feu, avec leurs résistances inverses à 0,75×.
