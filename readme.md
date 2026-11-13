# Chroniques de la Faille — Architecture modulaire

Le RPG est maintenant organisé en modules C++ spécialisés. Le point d’entrée `main.cpp` ne contient plus les règles du jeu : il instancie uniquement `Game` et lui délègue l’exécution.

## Documentation

Le détail des classes et des héritages se trouve dans [`docs/README_CLASSES.md`](docs/README_CLASSES.md). Le bestiaire complet et les statistiques des ennemis sont disponibles dans [`docs/BESTIAIRE.md`](docs/BESTIAIRE.md).

## Architecture

| Dossier | Rôle |
|---|---|
| `include/core` | Modèles du héros, types communs, éléments et interface terminal. |
| `include/characters` | Hiérarchie Guerrier, Prêtre, Vampire, Pile Face et BossDeFin. |
| `include/entities` | Contrats des ennemis et du générateur de contenu. |
| `include/systems` | Interfaces Combat, Donjons, Arène, Sauvegarde et Game. |
| `src/characters` | Implémentation des factories de classes. |
| `src/entities` | Statistiques des ennemis, boss et roster d’arène. |
| `src/systems` | Règles d’exécution et contrôleurs de chaque système. |
| `sauvegarde` | Fichiers de progression générés automatiquement. |
| `docs` | Documentation des classes et bestiaire/Pokédex. |
| `legacy` | Ancien prototype conservé à titre de référence, non compilé. |

## Compilation

Avec GNU Make :

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

Sous Windows, CMake peut générer un projet Visual Studio ; avec MinGW, la commande `make` reste disponible depuis MSYS2.

## Sauvegarde

La progression est enregistrée dans `sauvegarde/savegame.txt`. Le fichier contient le héros, l’identifiant de classe, le niveau, l’XP, les statistiques, l’inventaire, les quêtes, les donjons purifiés et le meilleur score d’arène. Le dossier est créé automatiquement au besoin.

Les fichiers du dossier `legacy` ne sont pas compilés. Ils permettent de retrouver l’ancien prototype sans polluer l’architecture actuelle.

## Équipe et éléments

Une nouvelle partie crée trois héros. Pour chacun, le joueur choisit un nom, une classe et un élément : Eau, Feu, Plante ou Ombre. Pendant les combats, le joueur choisit le héros actif à chaque tour.

Les éléments influencent les dégâts. Feu est avantageux contre Plante, Plante contre Eau, Eau contre Feu et Ombre contre Eau. Un avantage donne ×1,5 dégâts et une résistance donne ×0,75. Les équipements portent également un élément : un équipement assorti au porteur fournit un bonus automatique de +4 ATK/DEF ; une arme assortie ajoute aussi +5 % de chance critique.

Pour équilibrer le combat en équipe, les ennemis ordinaires ont reçu +15 % de PV et les boss +30 % de PV. La sauvegarde V3 de `sauvegarde/savegame.txt` conserve les trois héros, leurs classes, éléments, statistiques, équipements, inventaires et la progression commune.