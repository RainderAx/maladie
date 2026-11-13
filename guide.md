# Guide : ajouter de nouveaux éléments au système de combat

Ce guide explique comment étendre le RPG avec de nouveaux éléments, par exemple **Lumière** ou **Chaos**, sans casser les classes, les équipements, les ennemis, les combats ni la sauvegarde.

> **Principe général :** un élément est une valeur de `enum class Element`. Il doit ensuite être reconnu par l’affichage, la conversion utilisée par la sauvegarde et la fonction qui calcule les multiplicateurs de dégâts.

## 1. Architecture actuelle

Les éléments sont centralisés dans les fichiers suivants :

| Fichier | Responsabilité |
|---|---|
| `include/core/Types.hpp` | Déclare `Element`, `Item`, `Enemy` et les fonctions élémentaires. |
| `src/entities/EnemyFactory.cpp` | Implémente les noms, identifiants et multiplicateurs élémentaires. |
| `src/systems/CombatSystem.cpp` | Applique le multiplicateur aux attaques des héros et des ennemis. |
| `src/systems/Game.cpp` | Affiche le choix d’élément et crée les équipements de départ. |
| `src/systems/SaveSystem.cpp` | Sauvegarde l’élément des héros et des équipements. |
| `docs/BESTIAIRE.md` | Documente les éléments des ennemis et boss. |

Le calcul actuel est appelé dans les deux directions :

```cpp
// Attaque du héros vers l’ennemi
int damage = std::max(
    1,
    static_cast<int>(rawDamage * elementMultiplier(hero.element, enemy.element))
        - enemy.defense
);

// Attaque de l’ennemi vers le héros
int taken = std::max(
    1,
    static_cast<int>(enemy.attack * elementMultiplier(enemy.element, target.element))
        - target.defense()
);
```

## 2. Ajouter Lumière et Chaos à l’énumération

Ouvrir `include/core/Types.hpp` et ajouter les deux valeurs :

```cpp
enum class Element {
    Eau,
    Feu,
    Plante,
    Ombre,
    Lumiere,
    Chaos
};
```

Il est conseillé de conserver l’ordre des valeurs existantes si des sauvegardes utilisent déjà leurs identifiants numériques. Les nouvelles valeurs sont ajoutées à la fin afin de ne pas modifier la signification des anciennes sauvegardes.

## 3. Ajouter la conversion élément → nom affiché

Dans `src/entities/EnemyFactory.cpp`, la fonction `elementName` doit reconnaître les nouvelles valeurs :

```cpp
std::string elementName(Element element) {
    switch (element) {
        case Element::Eau: return "Eau";
        case Element::Feu: return "Feu";
        case Element::Plante: return "Plante";
        case Element::Ombre: return "Ombre";
        case Element::Lumiere: return "Lumière";
        case Element::Chaos: return "Chaos";
    }

    return "Inconnu";
}
```

Même si le `default` permettrait de compiler, il est préférable d’avoir un `case` explicite pour chaque élément. Cela évite d’afficher accidentellement `Ombre` lorsqu’un nouvel élément est oublié.

## 4. Ajouter la conversion identifiant → élément

La sauvegarde utilise un entier pour écrire l’élément. Les fonctions suivantes doivent donc être mises à jour :

```cpp
Element elementFromId(int id) {
    switch (id) {
        case 0: return Element::Eau;
        case 1: return Element::Feu;
        case 2: return Element::Plante;
        case 3: return Element::Ombre;
        case 4: return Element::Lumiere;
        case 5: return Element::Chaos;
        default: return Element::Ombre;
    }
}

int elementId(Element element) {
    return static_cast<int>(element);
}
```

La valeur de secours doit rester un élément valide. Ici, une donnée inconnue est convertie en `Ombre`, qui est l’élément neutre actuel.

> Si la numérotation des éléments est modifiée après la publication de sauvegardes, il faut créer une migration de sauvegarde. Ne pas réordonner les quatre premiers éléments sans adapter les anciens fichiers.

## 5. Définir les forces et résistances

L’élément Ombre doit rester neutre dans la version actuelle : il n’est efficace contre rien et ne résiste à rien. Pour ajouter Lumière et Chaos, il faut étendre `elementMultiplier`.

Une table lisible est préférable à une longue condition `if`. Exemple :

```cpp
struct ElementInteraction {
    Element attack;
    Element defense;
    double multiplier;
};

static const std::vector<ElementInteraction> interactions = {
    {Element::Feu,    Element::Plante, 1.50},
    {Element::Plante, Element::Eau,     1.50},
    {Element::Eau,    Element::Feu,     1.50},

    {Element::Plante, Element::Feu,     0.75},
    {Element::Eau,    Element::Plante,  0.75},
    {Element::Feu,    Element::Eau,      0.75},

    {Element::Lumiere, Element::Chaos,  1.50},
    {Element::Chaos,   Element::Lumiere, 0.75}
};
```

La fonction peut ensuite parcourir la table :

```cpp
double elementMultiplier(Element attackElement, Element defenseElement) {
    for (const auto& interaction : interactions) {
        if (interaction.attack == attackElement &&
            interaction.defense == defenseElement) {
            return interaction.multiplier;
        }
    }

    return 1.0;
}
```

Avec cette version, toute interaction non listée reste neutre. Ombre, Lumière ou Chaos ne deviennent pas automatiquement forts contre les autres éléments : seules les relations écrites dans la table sont actives.

## 6. Choisir une règle pour Lumière et Chaos

Il faut décider si Lumière et Chaos forment un duel réciproque ou s’ils interagissent avec Ombre. Deux modèles sont possibles.

### Modèle A : duel Lumière / Chaos

Ce modèle conserve Ombre comme élément neutre :

| Attaque | Défense | Résultat |
|---|---|---:|
| Lumière | Chaos | ×1,5 |
| Chaos | Lumière | ×0,75 |
| Ombre | Tous | ×1,0 |
| Tous | Ombre | ×1,0 |

Ce modèle est recommandé si Ombre doit rester complètement neutre.

### Modèle B : cycle Lumière / Ombre / Chaos

Ce modèle donne à Ombre un rôle actif, mais il contredit la règle actuelle d’Ombre neutre :

| Attaque | Défense | Résultat |
|---|---|---:|
| Lumière | Chaos | ×1,5 |
| Chaos | Ombre | ×1,5 |
| Ombre | Lumière | ×1,5 |

Ne pas utiliser ce modèle si Ombre doit rester à **1,0× partout**.

## 7. Proposer Lumière et Chaos à la création du héros

Dans `src/systems/Game.cpp`, le menu de création doit afficher les nouveaux choix :

```cpp
std::cout << "1 Eau\n"
          << "2 Feu\n"
          << "3 Plante\n"
          << "4 Ombre\n"
          << "5 Lumière\n"
          << "6 Chaos\n> ";

hero.element = elementFromId(Terminal::ask(1, 6) - 1);
```

Les équipements de départ doivent utiliser l’élément choisi :

```cpp
hero.weapon = {
    "Lame de " + elementName(hero.element),
    "Une arme accordée à l’élément du héros.",
    2, 0, 0, 0, true, hero.element
};

hero.armor = {
    "Manteau de " + elementName(hero.element),
    "Une armure accordée à l’élément du héros.",
    0, 2, 0, 0, true, hero.element
};
```

## 8. Ajouter Lumière et Chaos aux ennemis

Un ennemi reçoit son élément dans `src/entities/EnemyFactory.cpp` :

```cpp
Enemy makeEnemy(int zone, bool boss) {
    if (zone == 5) {
        return Enemy{
            "Séraphin Fracturé",
            "Oracle",
            420, 420,
            44, 18, 28,
            700, 500,
            Element::Lumiere,
            true
        };
    }

    return Enemy{
        "Bête du Chaos",
        "Berserker",
        260, 260,
        38, 10, 24,
        300, 150,
        Element::Chaos,
        false
    };
}
```

Le bestiaire doit être mis à jour dans `docs/BESTIAIRE.md` afin d’indiquer le nouvel élément et les interactions utilisées par chaque ennemi.

## 9. Équipement élémentaire

La structure `Item` contient déjà un champ `Element element`. Pour ajouter un équipement Lumière ou Chaos à la boutique :

```cpp
{"Lame solaire", "ATK +14 — Lumière", 14, 0, 0, 220, true, Element::Lumiere},
{"Marteau chaotique", "ATK +16 — Chaos", 16, 0, 0, 260, true, Element::Chaos}
```

Le bonus d’équipement assorti est calculé dans `Player.hpp` :

```cpp
int matchingEquipmentBonus() const {
    int bonus = 0;
    if (weapon.element == element) bonus += 4;
    if (armor.element == element) bonus += 4;
    return bonus;
}
```

Aucune modification supplémentaire n’est nécessaire pour le calcul du bonus : dès que `Element::Lumiere` ou `Element::Chaos` existe dans l’énumération, la comparaison fonctionne automatiquement.

## 10. Sauvegarde et compatibilité

`SaveSystem.cpp` écrit l’identifiant numérique de l’élément du héros et de chaque objet :

```cpp
out << elementId(hero.element);
```

Pour les nouveaux éléments, il suffit d’étendre `elementFromId`. La version actuelle du fichier de sauvegarde est `CHRONIQUES_SAVE_V3`. Après une modification importante du format, il est recommandé de passer à `CHRONIQUES_SAVE_V4` et d’ajouter une migration explicite plutôt que de charger silencieusement des données ambiguës.

Exemple de migration simple :

```cpp
if (marker == "CHRONIQUES_SAVE_V3") {
    // Charger les anciennes valeurs Eau, Feu, Plante et Ombre.
}
else if (marker == "CHRONIQUES_SAVE_V4") {
    // Charger les valeurs incluant Lumière et Chaos.
}
else {
    return false;
}
```

## 11. Tests recommandés

Avant de considérer l’extension terminée, vérifier les cas suivants :

```cpp
assert(elementMultiplier(Element::Ombre, Element::Lumiere) == 1.0);
assert(elementMultiplier(Element::Lumiere, Element::Ombre) == 1.0);
assert(elementMultiplier(Element::Lumiere, Element::Chaos) == 1.5);
assert(elementMultiplier(Element::Chaos, Element::Lumiere) == 0.75);
assert(elementFromId(elementId(Element::Lumiere)) == Element::Lumiere);
assert(elementFromId(elementId(Element::Chaos)) == Element::Chaos);
```

Tester également en jeu :

1. créer trois héros et sélectionner Lumière ou Chaos ;
2. vérifier que l’élément apparaît dans la fiche de l’équipe ;
3. acheter un équipement du même élément ;
4. vérifier le bonus d’équipement assorti ;
5. lancer une attaque contre une cible avantageuse, résistante et neutre ;
6. quitter le jeu puis vérifier la reprise de l’élément depuis la sauvegarde ;
7. vérifier que les ennemis affichent correctement Lumière ou Chaos dans le bestiaire et le combat.

## 12. Checklist d’extension

| Étape | Fichier principal | Terminé quand… |
|---:|---|---|
| 1 | `Types.hpp` | Les valeurs `Lumiere` et `Chaos` existent. |
| 2 | `EnemyFactory.cpp` | Nom, identifiant et multiplicateurs sont gérés. |
| 3 | `Game.cpp` | Les héros peuvent choisir les nouveaux éléments. |
| 4 | `EnemyFactory.cpp` | Des ennemis peuvent recevoir les nouveaux éléments. |
| 5 | `Game.cpp` | La boutique propose des équipements correspondants. |
| 6 | `SaveSystem.cpp` | Les éléments sont sauvegardés et rechargés. |
| 7 | `BESTIAIRE.md` | Les nouveaux ennemis et règles sont documentés. |
| 8 | Build | `make clean && make` réussit sans erreur. |
| 9 | Tests | Les multiplicateurs et la reprise de sauvegarde sont vérifiés. |

## Résumé

Pour ajouter un élément, il ne suffit pas de modifier l’énumération. Il faut également compléter son nom, son identifiant de sauvegarde, ses interactions, les menus de création, les équipements disponibles, les ennemis concernés et la documentation. La fonction de combat est déjà générique : elle appelle `elementMultiplier`, ce qui permet d’ajouter des éléments sans réécrire toute la boucle de combat.