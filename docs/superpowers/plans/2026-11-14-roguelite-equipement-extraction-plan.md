# Plan d'implémentation — Équipement, économie et extraction

Conception de référence : [`2026-11-13-roguelite-equipement-extraction-design.md`](../specs/2026-11-13-roguelite-equipement-extraction-design.md). Référence Git : `868cc3c2561a66717ec0e7e159a01c3e6a3fedde`. Travail test-first : écrire un test comportemental, vérifier son échec attendu, implémenter le changement minimal, puis rejouer le test et toute la suite pertinente.

## 0. Établir la baseline

- Installer CMake dans le sandbox (l'appel `make` initial échoue car la commande est absente).
- Configurer avec `BUILD_GRAPHICAL_GAME=OFF`, compiler terminal + tests, puis exécuter CTest avant tout changement de code.
- Conserver le résultat initial et ne prétendre à aucune validation graphique tant que Raylib et une fenêtre ne sont pas disponibles.

## 1. Corriger les invariants de vie et d'équipement (RED → GREEN)

1. Ajouter des tests qui échouent sur un héros à PV négatifs après dégâts; borner les PV à zéro et valider les états sérialisables.
2. Tester que l'XP et les montées de niveau ne réaniment pas un héros mort; distribuer l'XP seulement aux héros vivants dans roguelite et campagne.
3. Tester les bonus d'un emplacement vide; retirer les bonus élémentaires et critiques quand aucune pièce n'est équipée.
4. Tester plusieurs promotions d'un même rival; conserver ses PV courants tout en appliquant la croissance de stats.
5. Tester l'équipement remplacé retourné au stockage/sac plutôt que détruit; réutiliser le même chemin de permutation pour le camp et les inventaires qui équipent des objets.

## 2. Fiabiliser les sauvegardes roguelite

1. Ajouter les cas RED de sauvegarde/rechargement d'une run avec un héros à 0 PV, et d'entrée invalide refusée sans toucher au fichier valide existant.
2. Ajouter l'armurerie, les rangs d'amélioration et les champs de run nécessaires; lire V1 avec des valeurs par défaut puis écrire les nouvelles données en V2.
3. Valider l'intégralité des profils, runs, inventaires et objets avant ouverture; écrire dans un temporaire et remplacer la destination seulement après succès.
4. Tester le round-trip V2 du profil, de la réserve et de la run, ainsi que la migration V1 et les chemins d'échec.

## 3. Construire les fonctions métier d'économie, forge et extraction

1. Tester les gains exacts de fragments (normal, Némésis, boss, avec et sans mort) et l'état de compteur par salle; implémenter le barème spécifié.
2. Tester l'achat d'armes/armures, le refus si les fragments sont insuffisants, l'équipement depuis l'armurerie et la restitution de l'ancienne pièce.
3. Tester les améliorations d'armes/armures, leurs coûts et plafonds, ainsi que le refus si le héros manque d'or ou est mort.
4. Tester l'extraction d'une pièce non équipée, une seule fois par checkpoint, son retrait du sac et sa persistance après défaite; tester qu'un butin non extrait n'est pas crédité.
5. Tester la persistance des pièces équipées après victoire de la run et le reset des seules données temporaires à la nouvelle expédition.

## 4. Intégrer les nouveaux choix à Raylib

- Ajouter le catalogue et les contrôles d'armurerie à l'atelier sans déplacer le mode terminal ni casser le roster.
- Ajouter à la récompense de salle l'extraction d'un objet et l'option de forge, sans supprimer le choix Potion/Repos.
- Afficher solde, coûts, rangs, limites et résultat des opérations; vérifier les retours de sauvegarde et présenter les erreurs dans un écran encore visible.
- Compiler la cible graphique si Raylib 5.5 est disponible; sinon documenter exactement cette limite.

## 5. Corriger les défauts reproductibles de la campagne terminale

1. Tester l'EOF sur `Terminal::ask`; lever/propager une fin d'entrée contrôlée et quitter proprement au lieu de boucler.
2. Tester une quête de victoire en combat multi-tours; ne progresser qu'une seule fois après victoire.
3. Corriger Pile Face selon le bonus de classe documenté et tester le taux effectif.
4. Tester et imposer un roster de trois héros lors du chargement de campagne.
5. Tester qu'une fuite de boss ne déclenche pas une attaque et n'affecte pas la vie de l'équipe.
6. Borner les ennemis de l'arène selon la vague et vérifier la montée attendue.
7. Ne pas modifier les fichiers/ressources `legacy/` ni supprimer la sauvegarde V3 suivie par Git.

## 6. Build, documentation et intégration finale

- Rendre les cibles terminal/tests accessibles sans Raylib; garder la cible graphique activable et expliquer son prérequis dans le README.
- Utiliser une fixture dédiée pour le test de rétrocompatibilité V3; le test ne doit pas lire la sauvegarde personnelle suivie par Git.
- Mettre à jour `AMELIORATIONS.md` avec une entrée par changement réellement réalisé et la liste réelle des commandes exécutées.
- Exécuter le build complet possible, CTest, un test sous EOF avec timeout, contrôler `git diff --check`, inspecter les changements, puis commit descriptif et push vers `main` via le connecteur GitHub autorisé.

## Points de contrôle

- Aucun code de production sans test d'abord observé en échec pour le comportement visé.
- Après chaque groupe GREEN : suite complète des tests avant le groupe suivant.
- Toute contrainte d'environnement, échec préexistant ou test non exécuté est reporté explicitement.
- Réexécuter les vérifications après toute correction, et relire les sauvegardes générées dans des chemins temporaires propres.
