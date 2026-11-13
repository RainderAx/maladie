# Conception — Équipement, économie et extraction du roguelite

**Date :** 2026-11-13
**Dépôt de référence :** `RainderAx/maladie`, branche `main`, commit `868cc3c2561a66717ec0e7e159a01c3e6a3fedde`
**État de référence :** clone propre. `make` échoue avant compilation car CMake n'est pas installé dans l'environnement; Raylib n'a pas encore été vérifiée. La logique roguelite est séparée de l'interface dans `RogueliteSystem`, mais les objets ne persistent aujourd'hui qu'en run et l'atelier ne vend aucune pièce d'équipement.

## Intention

Approfondir les choix de progression de **Chroniques de la Faille** sans refondre le jeu : rendre les fragments liés à la qualité et au danger des combats, donner à l'atelier une véritable armurerie, permettre d'améliorer une pièce pendant une expédition, et rendre les objets remplacés récupérables par un système d'extraction vers une réserve de base. Corriger simultanément les bugs reproductibles cités dans l'audit fourni qui touchent les règles, les sauvegardes et la campagne terminale.

## Objectifs

- Remplacer la récompense roguelite fixe par combat par un barème déterministe lié à la puissance de l'adversaire et à la survie de l'équipe.
- Permettre l'achat d'équipements au camp, leur rangement en armurerie et leur attribution aux héros avant le départ.
- Faire qu'un équipement remplacé ne disparaisse jamais silencieusement.
- Autoriser une amélioration facultative d'arme ou d'armure entre les salles, en consommant l'or de run.
- Permettre de sécuriser un objet de butin à la base après chaque salle normale; une défaite ne doit pas effacer les fragments déjà gagnés ni le butin déjà extrait.
- Préserver les sauvegardes existantes de campagne et accepter les profils/runs roguelite V1 déjà présents.
- Corriger les bugs confirmés de l'audit dans les mêmes systèmes, avec tests de non-régression écrits avant les correctifs.
- Maintenir la campagne terminale et documenter les modifications dans `AMELIORATIONS.md`.

## Hors périmètre

- Pas de fabrication, de raretés, d'affixes aléatoires, d'arbres de talents, de nouvelles classes, de génération de cartes ou de refonte graphique complète.
- Pas de suppression de l'ancienne sauvegarde, du prototype `legacy/`, ni des documents dont l'usage n'est pas clairement abandonné.
- Pas de changement au format V3 des sauvegardes de campagne, sauf corrections ciblées nécessaires à leurs règles de lecture/écriture.
- Les défauts purement visuels qui ne sont pas vérifiables sans Raylib (polices, mise en page sur petits écrans) ne seront pas déclarés corrigés sans exécution graphique.

## Approches considérées

1. **Enrichissement minimal :** récompenses un peu variables et équipement de base, sans extraction sécurisée. Simple, mais les objets gagnés restent difficiles à conserver et les choix entre deux pièces ne résolvent pas complètement le problème signalé.
2. **Boucle d'équipement et d'extraction — retenue par l'utilisateur :** fragments liés aux combats, achats persistants au camp, améliorations temporaires en run, inventaire de butin, et extraction d'un objet par salle sûre. Elle prolonge les systèmes actuels sans ajouter de matériaux ou d'aléas complexes.
3. **Fabrication complète :** matériaux, raretés et effets spéciaux. Écartée pour cette livraison, car l'équilibrage et l'interface seraient disproportionnés à la demande.

## Règles de jeu proposées

Ces valeurs précises sont des choix d'implémentation proposés à valider avec ce document.

### Fragments et récompenses de combat

Les fragments restent une monnaie persistante, distincte de l'or dépensé pendant la run. Ils sont crédités au profil dès la victoire, comme aujourd'hui, afin qu'une mort ultérieure ne retire pas une récompense déjà acquise. Le montant devient dépendant de l'adversaire :

- Rencontre normale : `1 + min(2, XP ennemi / 100)` fragments (division entière).
- Némésis vaincue : même barème, avec un bonus de `+1`.
- Boss : `4 + min(2, XP boss / 250)` fragments.
- Toute victoire obtenue sans mort de héros pendant cette salle donne `+1` fragment de maîtrise. Le compteur de morts est remis à zéro au début de chaque salle.

Cela récompense davantage les rencontres risquées et les combats propres tout en bornant les gains. Les formules sont indépendantes de l'interface et testées avec des valeurs littérales.

### Armurerie du camp

- Les offres d'armes et d'armures ont des coûts fixes en fragments, distincts du champ `Item::value` utilisé comme prix en or de la campagne.
- Le catalogue initial réutilise les trois pièces déjà présentes dans l'échoppe de campagne : Épée turquoise (ATQ +9, Eau) et Plastron forestier (DEF +8, Plante) à 6 fragments chacun, Lame des braises (ATQ +12, Feu) à 9 fragments. Les offres sont achetables plusieurs fois tant que le profil a les fragments nécessaires.
- Un achat va dans l'armurerie persistante du profil. À l'atelier, le joueur peut équiper une pièce de l'armurerie à un héros; la pièce remplacée est remise dans l'armurerie au lieu d'être détruite.
- L'équipement choisi sur le roster est le chargement de départ de la run. Les améliorations effectuées pendant une run ne modifient le roster permanent que si l'expédition se termine par une victoire.
- Les règles élémentaires s'appliquent uniquement à une pièce réellement équipée; un emplacement vide ne donne aucun bonus.

### Amélioration pendant une run

- À l'écran de récompense entre deux salles, une option facultative de forge permet d'améliorer l'arme ou l'armure équipée d'un héros vivant.
- Une amélioration ajoute `+2 ATQ` à une arme ou `+2 DEF` à une armure, jusqu'à trois rangs par objet.
- Le coût, payé sur la bourse d'or du héros sélectionné, est de `45 + 30 × rang_actuel` or. Aucun fragment n'est dépensé en run.
- La forge est indépendante du choix existant Potion/Repos; elle ne supprime pas ce choix et ne consomme pas de récompense de salle.

### Réserve et extraction

- Lorsqu'une arme ou une armure est remplacée pendant la run, l'ancienne pièce va dans l'inventaire de butin du héros.
- La seule source de nouvelles pièces pendant une expédition reste la récupération de l'équipement volé sur une Némésis vaincue; il est ajouté au sac du survivant comme dans la boucle existante. Aucun butin aléatoire ni nouveau type de récompense n'est introduit.
- Un combat contre une Némésis compte comme la salle normale qu'il termine : sa victoire ouvre donc le même checkpoint d'extraction avant le choix de la salle suivante. Dans le code actuel, une rencontre non-boss — y compris une Némésis — fait progresser le compteur de salle.
- Après chaque salle normale remportée, le joueur peut extraire au plus une pièce d'équipement non équipée provenant de l'inventaire de n'importe quel héros. L'objet est déplacé dans l'armurerie persistante; le joueur peut aussi ne rien extraire.
- Les fragments déjà gagnés et les objets extraits restent au profil après défaite. Le butin non extrait est perdu si toute l'équipe tombe.
- La victoire contre le boss clôt la run en sécurité : les pièces équipées à la fin remplacent les chargements du roster; aucune pièce de sac supplémentaire n'est transférée automatiquement, car les occasions d'extraction ont déjà été offertes aux checkpoints.

### Sauvegarde

- Introduire un format de profil roguelite versionné qui conserve l'armurerie et les nouveaux champs d'objet; continuer à lire `ROGUELITE_PROFILE_V1` et `ROGUELITE_RUN_V1`, en initialisant les nouveaux champs à leurs valeurs par défaut.
- Écrire les nouveaux fichiers par fichier temporaire puis remplacement contrôlé; valider les objets, bornes et compteurs avant d'ouvrir le fichier de destination.
- Ne jamais tronquer le profil ou la run valides si la validation ou l'écriture échoue.
- Les sauvegardes de campagne V3 restent lisibles sans migration destructive.

## Bugs de l'audit inclus dans ce lot

Les correctifs envisagés couvrent les constats reproductibles suivants : PV roguelite négatifs rejetés au chargement (C1); XP qui ressuscite les héros (C2); PV Némésis restaurés de façon incohérente (C3); bonus d'équipement fantôme après vol (C4); boucle terminale sur EOF; quête progressant à chaque frappe plutôt qu'à la victoire; bonus initial excessif de Pile Face; acceptation d'équipes de campagne trop courtes; destruction d'équipement remplacé; fuite contre un boss interprétée comme une attaque; choix d'ennemis d'arène non borné par vague; et validations/écritures de sauvegarde roguelite pouvant laisser un fichier partiel. Les tests et builds documenteront les défauts qui restent hors périmètre ou que l'environnement ne permet pas de vérifier.

## Réponses proposées aux questions ouvertes de l'audit

- **Échap en combat :** conserver le retour au checkpoint de salle, déjà annoncé dans le README. C'est un moyen explicite de reprendre une rencontre sans transformer un essai avorté en mort; le butin n'est sécurisé qu'aux checkpoints gagnés.
- **PV Némésis :** conserver les PV réellement restants quand le héros tombe et quand le rival réapparaît; les dégâts infligés ne sont pas annulés. Un rival vaincu est retiré comme aujourd'hui.
- **Campagne terminale :** la maintenir, sans en faire une réécriture; elle reste accessible comme mode autonome.
- **Rendu et plateformes :** toute limite liée à Raylib, Windows/macOS ou aux polices sera rapportée comme non testée si elle ne peut être exécutée ici.

## Contraintes et décisions laissées au constructeur

- Rester en C++17 et utiliser la bibliothèque standard et Raylib déjà prévues; aucune dépendance de gameplay supplémentaire.
- Séparer les règles métier de l'UI et garder la suite de tests sans fenêtre.
- Conserver le style/noms du dépôt, les objets de la campagne, la graine déterministe et les fichiers existants dont le rôle est incertain.
- Construire les changements dans une copie de travail et vérifier le build, les tests et la compatibilité avant publication.
- L'organisation précise des fonctions et les détails visuels des nouveaux contrôles sont laissés au constructeur, sous réserve que les coûts, limites et conséquences énoncés ci-dessus restent visibles.

## Critères d'acceptation

- Les gains de fragments correspondent aux formules ci-dessus pour une rencontre normale, une Némésis, un boss et une victoire avec/sans décès.
- Une run avec un héros tombé s'enregistre et se recharge; aucun gain d'XP ne ressuscite un héros.
- Les dégâts subis par la Némésis restent acquis; un emplacement vide n'octroie ni bonus élémentaire ni critique.
- Un achat d'équipement, une affectation au héros, une permutation, une amélioration, une extraction, une sauvegarde puis un rechargement conservent les objets et les limites attendues.
- Le butin extrait et les fragments survivent à une défaite; le butin non extrait n'est pas crédité par inadvertance.
- Les défauts ciblés de la campagne ont des tests de non-régression.
- Build C++ et suite complète passent, ou leurs limites/outils manquants sont décrits exactement. Aucune exécution graphique ne sera revendiquée sans l'avoir réalisée.
- `AMELIORATIONS.md` indique pour chaque changement nom, type, fichiers, description, justification, source et impact; ses vérifications correspondent aux commandes vraiment exécutées.
