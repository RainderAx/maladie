# Conception — Roguelite graphique et rivaux Némésis

**Date :** 2026-11-13<br>
**Dépôt de référence :** `RainderAx/maladie`, branche `main`, commit `f966f1d`<br>
**État de référence vérifié :** `make clean && make` réussit avec GCC et C++17. Aucun test automatisé ni `CMakeLists.txt` n’est présent, bien que le README décrive une commande CMake. Le jeu actuel est piloté par `std::cin`/`std::cout` ; la logique de combat et la présentation sont étroitement mêlées.

## Intention

Faire de **Chroniques de la Faille** un premier jeu roguelite jouable dans une fenêtre 2D, en conservant son univers, ses dix classes, ses quatre éléments, son équipe de trois héros, ses équipements et son aventure existante. La première version doit fournir une boucle de run courte et relançable, un adversaire persistant qui s’enrichit de l’équipement et de la puissance des héros qu’il tue, ainsi qu’une progression permanente payée entre les runs.

## Objectifs

- Démarrer par défaut dans une interface graphique native en C++ avec Raylib, utilisable à la souris et au clavier.
- Garder l’aventure actuelle en mode terminal et ne pas remplacer ses fonctionnalités ou sa sauvegarde.
- Permettre de créer/reprendre une run d’équipe, d’enchaîner des salles de difficulté croissante, de choisir quelques récompenses et d’affronter un boss final.
- Rendre les morts importantes : un héros tué est indisponible jusqu’à la fin de la run ; si l’équipe entière tombe, la run se termine.
- Conserver un roster de trois héros entre les runs et permettre d’y acheter des améliorations permanentes à l’écran de préparation, grâce aux fragments gagnés pendant les runs.
- Enregistrer l’état de run séparément de la sauvegarde de campagne, pour ne pas la convertir, l’écraser ou la rendre incompatible.
- Séparer les règles de jeu des appels Raylib et de l’affichage terminal pour pouvoir tester les décisions de combat et de progression sans ouvrir de fenêtre.
- Ajouter `AMELIORATIONS.md` à la racine et y tracer précisément les seuls changements effectivement réalisés.

## Hors périmètre de cette première version

- Refonte complète des donjons/campagne existants, remplacement des classes ou rééquilibrage général du jeu.
- Multijoueur, réseau, génération de cartes sans fin, animation avancée, cinématiques, son ou dépendance d’assets graphiques externes.
- Arbre de compétences, déblocages permanents de classes/objets et améliorations permanentes autres que les bonus de statistiques prévus ici.
- Suppression de l’exécutable ou du mode terminal existant.

## Parcours de jeu proposé

1. Le menu graphique propose **Nouvelle run**, **Continuer** lorsqu’une sauvegarde de run valide existe, **Aventure classique (terminal)** et **Quitter**. Au premier lancement, le joueur crée son roster persistant de trois héros à partir des classes et éléments déjà disponibles.
2. Avant chaque nouvelle run, un écran d’entraînement laisse dépenser les fragments bancarisés sur n’importe lequel des trois héros. Les noms, classes, éléments et bonus permanents du roster sont conservés d’une run à l’autre. Pour la nouvelle run, les niveaux, PV courants et équipements repartent de leur base ; les bonus achetés s’ajoutent aux statistiques de départ. La run reçoit une graine aléatoire enregistrée, afin que son état soit reproductible.
3. La run propose une courte suite de rencontres tirées du bestiaire existant, avec difficulté croissante et un boss de fin. Après les rencontres prévues, le joueur choisit une récompense simple, réutilisant les objets et équipements existants ; les salles de repos soignent uniquement les survivants.
4. Le joueur choisit le héros actif et une action de combat. Un héros à 0 PV est retiré des choix jusqu’à la fin de la run. Une équipe sans survivant entraîne la défaite et le résumé de run.
5. À chaque mort causée par un ennemi, cet ennemi individuel devient un **rival Némésis** nommé et persistant dans la run. Il vole les deux pièces équipées par la victime (arme et armure), stocke les objets et reçoit un bonus visible : ATQ augmentée de la valeur d’attaque de l’arme, DEF augmentée de la défense de l’armure, PV max augmentés de `10 × niveau du héros`, chance critique augmentée de 2 points (plafond 60 %). Ces valeurs ont été approuvées par l’utilisateur. Un rival ayant tué plusieurs héros cumule les objets volés et les bonus associés ; plusieurs ennemis distincts peuvent devenir des rivaux dans une même run. La victoire ultérieure contre un rival restitue tout son équipement volé dans le butin de run et retire ce rival des rencontres futures. Un rival survivant peut remplacer une rencontre ultérieure appropriée.
6. Chaque salle non-boss terminée rapporte 1 fragment de Faille ; le boss final en rapporte 4. Les fragments sont crédités et conservés même si l’équipe perd ensuite la run.
7. La sauvegarde roguelite est indépendante de `sauvegarde/savegame.txt` : le profil permanent conserve roster, fragments et achats ; la sauvegarde de run conserve équipe, salle, graine, état du parcours et rivaux. Les sauvegardes se font aux checkpoints de salle ; quitter pendant un combat reprend au début de cette salle, sans stocker l’état de chaque tour. Une nouvelle run remplace uniquement la progression de run après confirmation dans l’interface. Les anciennes sauvegardes V3 de la campagne restent intactes et leur format ne change pas.

### Progression permanente proposée

Les bonus s’achètent uniquement avant une nouvelle run, après avoir obtenu les fragments lors de runs précédentes. Le joueur peut acheter, sur chaque héros, `+10 PV max`, `+1 ATQ` ou `+1 DEF`. Le premier achat dans une statistique coûte 4 fragments pour les PV ou 6 pour l’ATQ/DEF ; chaque achat ultérieur dans cette même statistique coûte 2 fragments de plus que le précédent. Les plafonds sont `+50 PV`, `+8 ATQ` et `+8 DEF` par héros. Ces valeurs de coût et de plafond sont les réglages initiaux proposés pour pouvoir vérifier l’économie et seront faciles à ajuster après les premiers essais.

## Interface proposée

- Fenêtre native redimensionnable, démarrage en 1280×760 avec une taille minimale de 960×760.
- Écrans distincts pour l’accueil, la création du roster, l’entraînement, la progression, le combat, les récompenses et le résumé ; la progression montre le chemin de salles et l’équipe, tandis que le combat affiche l’équipe à gauche, l’ennemi et ses statistiques au centre, et les actions dans un bandeau inférieur.
- Palette sombre et contrastée cohérente avec la Faille ; dessin 2D avec formes, cadres et texte Raylib, sans exiger d’illustrations externes.
- Survol/focus visuel, boutons souris et raccourcis clavier correspondants ; taille de fenêtre et lisibilité adaptées au premier prototype.
- Les valeurs de PV, équipement volé, bonus Némésis, prochaine étape et état de run restent visibles, afin que la mécanique soit compréhensible sans journal terminal.

## Approches considérées

1. **Raylib natif C++ — retenue par l’utilisateur.** Ajoute une bibliothèque graphique 2D simple et permet de conserver le langage et les données de jeu. C’est la voie la plus directe pour rendre ce dépôt réellement visuel ; elle impose de rendre Raylib disponible au build et de séparer la boucle d’affichage du modèle de jeu.
2. **SDL2 natif C++.** Alternative desktop éprouvée avec un contrôle plus bas niveau ; elle implique plus d’infrastructure d’entrée/rendu pour ce prototype sans bénéfice demandé.
3. **Interface navigateur.** Pourrait s’exécuter dans un onglet, mais demande soit de porter les règles en JavaScript, soit de configurer WebAssembly/Emscripten. Le dépôt n’a actuellement ni ces outils ni une architecture web ; cette option élargit nettement le changement.

## Contraintes et décisions laissées au constructeur

- Pas de changement de portée ou d’effacement des sauvegardes de campagne.
- Rester en C++17 et ne pas ajouter de bibliothèque autre que Raylib et les outils de test/build nécessaires.
- Conserver les classes, objets, couleurs élémentaires et textes d’univers déjà présents autant que possible.
- Vérifier l’installation de Raylib dans l’environnement et fournir une commande reproductible documentée ; aucune dépendance ne doit être présentée comme installée si elle ne l’est pas.
- Le constructeur peut choisir le découpage précis des nouveaux fichiers, le dessin de la carte, la graine et la table de rencontres, à condition que le parcours reste court, lisible et déterministe à graine identique.
- Le schéma d’une éventuelle sauvegarde de run est nouveau et versionné ; il ne doit pas invalider le format V3 existant.

## Architecture envisagée

- Introduire un modèle de run indépendant de Raylib (état d’équipe, progression, graine, rencontres, fin de run) et des fonctions de transition d’état pilotées par des actions explicites.
- Introduire un profil méta-progression indépendant des règles d’une run : roster stable, fragments, compteurs d’achats/caps et opérations d’achat validant le coût avant application.
- Extraire ou réutiliser les règles de dégâts, choix de cible, fin de combat et attribution des récompenses sans dépendre de `std::cin`/`std::cout`; le mode terminal actuel doit appeler les règles communes ou rester fonctionnel avec un adaptateur compatible.
- Ajouter un système Némésis testable isolément : prise en charge unique d’un événement de mort, transfert d’équipement, bonus bornés, réapparition contrôlée et butin à la défaite.
- Ajouter une interface Raylib distincte du modèle : affichage, focus, souris/clavier et rendu des mêmes données. Aucun appel de menu bloquant ne doit être exécuté depuis une frame.
- Ajouter les cibles de build/tests appropriées. Le README documente réellement les commandes et prérequis vérifiés ; il ne doit plus promettre de commandes CMake inexistantes.

## Vérification et critères d’acceptation proposés

- Tests automatisés de logique sans fenêtre : run créée et progression entre salles, héros mort non sélectionnable, fin de run sur équipe à terre, transfert des deux pièces et bonus Némésis exacts, rival vaincu qui restitue le butin, fragments attribués/bancarisés, achat qui débite le coût sans dépasser les caps, bonus permanents retenus au démarrage d’une nouvelle run, sauvegarde/reprise du profil et d’une run valide, et lecture inchangée de la sauvegarde de campagne V3.
- Tests écrits avant le code correspondant, échec observé pour la raison attendue, puis succès après implémentation ; exécution de toute la suite et du build documenté.
- Build graphique Raylib et build terminal séparé réussis avec les avertissements activés. Si l’environnement ne permet pas d’ouvrir une fenêtre, le rapport distingue explicitement compilation et validation visuelle/runtime.
- Vérification manuelle possible : nouvelle run, combat, événement de mort, indication du vol et de la hausse de stats, retour d’un rival, victoire et récupération du butin, sauvegarde/reprise et défaite d’équipe.
- `AMELIORATIONS.md` ne contient que les changements réellement effectués, avec pour chacun nom, type, chemins exacts, description, justification, source et impact.

## Idées à proposer après livraison, sans les inclure d’office

- Des tempéraments et traits uniques pour les rivaux Némésis, au-delà de leur progression statistique.
- Des événements de salles et des chemins alternatifs plus variés.
- Des objets ou talents permanents à débloquer en plus des améliorations de statistiques de base.
