# Bestiaire — Pokédex de la Faille

Ce document recense les ennemis connus. Les valeurs de l’arène correspondent à la vague 1 ; à chaque vague, les statistiques augmentent automatiquement.

## Boss et ennemis des donjons

| Zone | Nom | Classe | PV | ATK | DEF | Chance | Élément |
|---|---|---|---:|---:|---:|---:|---|
| Cathédrale des Cendres | Rat d’obsidienne | Bête | 45 | 9 | 2 | 8 % | Feu |
| Cathédrale des Cendres | Gardien des Cendres | Guerrier | 180 | 22 | 8 | 15 % | Feu |
| Jardins Engloutis | Spectre noyé | Spectre | 82 | 16 | 5 | 12 % | Eau |
| Jardins Engloutis | Oracle des Marées | Oracle | 270 | 30 | 12 | 20 % | Eau |
| Faille du Roi | Chevalier de la Faille | Chevalier | 125 | 23 | 9 | 18 % | Plante |
| Faille du Roi | Roi Sans-Visage | Seigneur Nocturne | 390 | 38 | 16 | 25 % | Ombre |
| Faille du Roi | Reine aux Milles-Visages | BossDeFin | 520 | 46 | 20 | 35 % | Ombre |
| Sanctuaire inversé | Mimiqueur de Cristal | Paladin | 170 | 29 | 12 | 22 % | Eau |
| Sanctuaire inversé | Archonte du Néant | Templier | 620 | 52 | 24 | 32 % | Ombre |

## Roster de l’arène

| # | Nom | Classe | PV | ATK | DEF | Chance | Élément |
|---:|---|---|---:|---:|---:|---:|---|
| 1 | Garde de Fer | Guerrier | 90 | 18 | 6 | 10 % | Feu |
| 2 | Brisecrâne | Berserker | 115 | 24 | 5 | 13 % | Ombre |
| 3 | Chevalier Prismatique | Paladin | 145 | 22 | 12 | 11 % | Eau |
| 4 | Pénitent Cendré | Prêtre | 100 | 17 | 7 | 15 % | Feu |
| 5 | Voyante du Fil | Oracle | 110 | 20 | 8 | 18 % | Eau |
| 6 | Sceau Vivant | Templier | 155 | 25 | 14 | 14 % | Plante |
| 7 | Néophyte Sanguin | Vampire | 105 | 23 | 5 | 16 % | Ombre |
| 8 | Dévoreur Rouge | Sanguinaire | 135 | 28 | 7 | 19 % | Feu |
| 9 | Duc de la Nuit | Seigneur Nocturne | 180 | 32 | 11 | 22 % | Ombre |
| 10 | Lame du Quartier Noir | Guerrier | 125 | 27 | 8 | 12 % | Feu |
| 11 | Furie Fendue | Berserker | 150 | 35 | 6 | 20 % | Feu |
| 12 | Héraut d’Azur | Paladin | 190 | 28 | 16 | 15 % | Eau |
| 13 | Sœur des Braises | Prêtre | 135 | 24 | 9 | 21 % | Feu |
| 14 | Oracle Aveugle | Oracle | 160 | 31 | 10 | 24 % | Eau |
| 15 | Templier de Bronze | Templier | 210 | 34 | 18 | 16 % | Plante |
| 16 | Comte des Veines | Vampire | 175 | 36 | 9 | 23 % | Ombre |
| 17 | Sanguinaire Royal | Sanguinaire | 220 | 42 | 10 | 25 % | Feu |
| 18 | Marquis Sans-Lune | Seigneur Nocturne | 250 | 45 | 14 | 27 % | Ombre |
| 19 | Paladin du Miroir | Paladin | 240 | 37 | 20 | 18 % | Plante |
| 20 | Avatar de la Faille | Seigneur Nocturne | 300 | 50 | 17 | 30 % | Ombre |

## Règles de l’arène

Le roster est sélectionné aléatoirement à chaque vague. Pour une vague `n`, les PV et l’ATK gagnent un bonus proportionnel à `n - 1`, la DEF augmente également, la chance critique progresse sans dépasser 60 %, et les récompenses en XP et en or augmentent avec la vague. Le meilleur score est enregistré dans `sauvegarde/savegame.txt`.

## Équilibrage contre une équipe

Les statistiques de PV du tableau sont les valeurs de référence du bestiaire. En jeu, les ennemis ordinaires reçoivent automatiquement **+15 % de PV** et les boss **+30 % de PV** afin d’équilibrer les combats contre trois héros. L’arène conserve en plus sa progression de difficulté par vague.

Les éléments affichés dans ce bestiaire sont utilisés par le système de combat : ils déterminent les multiplicateurs de dégâts contre les héros et l’équipe.

## Correction de l’élément Ombre

L’élément **Ombre** est désormais totalement neutre : il n’est efficace contre aucun élément et ne résiste à aucun élément. Toute attaque Ombre ou toute attaque dirigée contre Ombre utilise un multiplicateur de **1,0×**.
