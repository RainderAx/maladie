# Chroniques de la Faille

**Chroniques de la Faille** est un RPG narratif en C++17 jouable depuis un terminal. Le prototype initial, réparti entre plusieurs classes de personnages et une simulation de combat isolée, a été restructuré autour d’une boucle de jeu complète : hub central, exploration de donjons, combats au tour par tour, progression par expérience, quêtes, inventaire, équipement et boutique.

## Architecture

Le projet est volontairement simple à compiler et à transmettre. Le fichier `main.cpp` contient les systèmes du jeu afin de permettre une prise en main immédiate sans dépendance externe. Le `Makefile` encapsule la compilation et le lancement. Les anciens fichiers du prototype sont conservés dans le dépôt pour référence, notamment `main.legacy.cpp`, `Personnage.*`, `guerrier.*`, `priest.*` et `vampire.*`.

| Système | Fonctionnement |
|---|---|
| Héros | Trois voies : Sentinelle, Arcaniste et Nocturne, avec statistiques de départ différentes. |
| Progression | XP gagnée en combat et en quête, barre d’expérience, montée de niveau, amélioration des PV, de l’attaque et de la défense. |
| Donjons | Trois donjons reliés par une progression narrative : Cathédrale des Cendres, Jardins Engloutis et Faille du Roi. |
| Boss | Un gardien termine chaque donjon ; la Faille du Roi ajoute ensuite la Reine aux Milles-Visages comme second boss final. |
| Combat | Attaque normale, technique, potions, coups critiques et possibilité de fuite contre les ennemis ordinaires. |
| Quêtes | Quêtes de victoire et de boss, avec progression, XP et or à réclamer depuis le hub. |
| Objets | Potions de soin, armes et armures. Les équipements modifient directement les statistiques du héros. |
| Hub | Fiche de personnage, sacoche, quêtes, échoppe et repos entre les expéditions. |
| Classes | Dix classes sélectionnables : trois chaînes d’héritage, plus Pile Face. `BossDeFin` reste une classe de boss et n’est pas proposée au héros. |
| Compétences | Chaque classe possède une compétence spéciale avec dégâts, bonus ou soin propres, utilisable avec l’action « Technique ». |
| Sauvegarde | Écriture automatique dans `savegame.txt`, chargement proposé au démarrage et reprise des statistiques, classe, compétence, objets, quêtes et donjons purifiés. |

## Lancement local

Depuis un terminal Linux ou macOS :

```bash
cd rpg-terminal
make
./chroniques_de_la_faille
```

Ou en une seule commande après compilation :

```bash
make run
```

La compilation directe est également possible :

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp -o chroniques_de_la_faille
```

Le jeu utilise uniquement la bibliothèque standard C++ et les couleurs ANSI du terminal. Aucun téléchargement de dépendance ni service externe n’est nécessaire. La sauvegarde utilise `savegame.txt`, un fichier texte local sans dépendance JSON externe. Il est créé au premier lancement dans le même dossier que l’exécutable. Le jeu l’actualise après les actions importantes et propose automatiquement de reprendre la partie au prochain démarrage. Pour recommencer une aventure, choisissez « Nouvelle partie » au démarrage ou supprimez manuellement `savegame.txt`.

## Parcours recommandé

Commencez par choisir un nom et une voie. Depuis la Place Noire, consultez vos statistiques, équipez les objets trouvés, achetez des potions et reposez-vous avant une expédition. Les donjons sont déverrouillés dans l’ordre. Chaque zone comprend deux combats ordinaires, puis un combat de boss. Le deuxième donjon s’ouvre après la purification du premier, et le troisième après celle du deuxième. La victoire contre le troisième boss déclenche la conclusion narrative.

## Notes de conception

La classe sélectionnée est sérialisée par son identifiant dans `savegame.txt`, ce qui permet de restaurer le bon type polymorphe au prochain lancement.

La direction artistique est transposée dans le terminal par des titres encadrés, une palette ANSI nocturne, des messages de progression et un vocabulaire de monde vivant : faille, mana, gardiens, purification et éveil. Le code sépare les concepts principaux en structures et fonctions spécialisées pour permettre une extension ultérieure vers des fichiers `player.cpp`, `combat.cpp`, `dungeon.cpp`, `quest.cpp` et `item.cpp` sans changer les règles de jeu.

## Éléments et statistiques des ennemis

Les ennemis et les boss disposent maintenant des champs **PV**, **ATK**, **DEF**, **chance de coup critique**, **élément**, **XP**, **or** et **classe de combat**. Les éléments actuellement utilisés sont `Eau`, `Feu`, `Plante` et `Ombre`.

| Ennemi ou boss | PV | ATK | DEF | Chance | Élément | Classe |
|---|---:|---:|---:|---:|---|---|
| Rat d’obsidienne | 45 | 9 | 2 | 8 % | Feu | Bête |
| Gardien des Cendres | 180 | 22 | 8 | 15 % | Feu | Guerrier |
| Spectre noyé | 82 | 16 | 5 | 12 % | Eau | Spectre |
| Oracle des Marées | 270 | 30 | 12 | 20 % | Eau | Oracle |
| Chevalier de la Faille | 125 | 23 | 9 | 18 % | Plante | Chevalier |
| Roi Sans-Visage | 390 | 38 | 16 | 25 % | Ombre | Seigneur Nocturne |
| Reine aux Milles-Visages | 520 | 46 | 20 | 35 % | Ombre | BossDeFin |

Après la victoire contre le **Roi Sans-Visage**, le joueur affronte immédiatement la **Reine aux Milles-Visages**. La Reine utilise la classe technique `BossDeFin`, mais cette classe n’est plus proposée dans le choix du héros.

## Classe Pile Face

`PileFace` hérite directement de `Classe` et devient la dixième classe sélectionnable par le héros. Son orientation repose sur la chance : elle possède une chance critique de base élevée, un bonus de chance propre et la compétence `Destin hasardeux`. La chance du héros est affichée dans sa fiche et sauvegardée dans `savegame.txt`. Les ennemis peuvent également réaliser des coups critiques selon leur propre statistique de chance.

## Extension : Sanctuaire inversé et Arène infinie

Le **Sanctuaire inversé** est un quatrième donjon optionnel accessible après la victoire sur les trois donjons principaux. Il contient deux rencontres contre des Mimiqueurs de Cristal, puis un boss nommé **Archonte du Néant**, de classe Templier. Sa purification ne remplace pas la fin principale et est enregistrée séparément dans la sauvegarde.

L’**Arène infinie** est accessible depuis la Place Noire. Chaque vague sélectionne aléatoirement un monstre parmi le roster ci-dessous. Après chaque victoire, le niveau de la vague augmente les PV, l’ATK, la DEF, la chance critique, l’XP et l’or de l’adversaire. Le joueur peut récupérer une partie de ses PV entre les vagues, continuer indéfiniment ou quitter l’arène. Le meilleur numéro de vague est sauvegardé.

| # | Monstre | Classe | Élément |
|---:|---|---|---|
| 1 | Garde de Fer | Guerrier | Feu |
| 2 | Brisecrâne | Berserker | Ombre |
| 3 | Chevalier Prismatique | Paladin | Eau |
| 4 | Pénitent Cendré | Prêtre | Feu |
| 5 | Voyante du Fil | Oracle | Eau |
| 6 | Sceau Vivant | Templier | Plante |
| 7 | Néophyte Sanguin | Vampire | Ombre |
| 8 | Dévoreur Rouge | Sanguinaire | Feu |
| 9 | Duc de la Nuit | Seigneur Nocturne | Ombre |
| 10 | Lame du Quartier Noir | Guerrier | Feu |
| 11 | Furie Fendue | Berserker | Feu |
| 12 | Héraut d’Azur | Paladin | Eau |
| 13 | Sœur des Braises | Prêtre | Feu |
| 14 | Oracle Aveugle | Oracle | Eau |
| 15 | Templier de Bronze | Templier | Plante |
| 16 | Comte des Veines | Vampire | Ombre |
| 17 | Sanguinaire Royal | Sanguinaire | Feu |
| 18 | Marquis Sans-Lune | Seigneur Nocturne | Ombre |
| 19 | Paladin du Miroir | Paladin | Plante |
| 20 | Avatar de la Faille | Seigneur Nocturne | Ombre |

Les monstres de l’arène utilisent les mêmes classes de combat que les personnages et les boss. Ils sont donc affichés avec leur classe, leur élément, leurs PV, leur ATK, leur DEF et leur chance critique pendant le combat.