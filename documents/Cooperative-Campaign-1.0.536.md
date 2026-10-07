# Campagne coop commune — 1.0.536

[English guide](COOP-CAMPAIGN-EN.md)

L’entrée **Campagne coop** du menu principal permet de jouer à deux sur LAN ou Internet. Chaque joueur choisit librement sa faction et contrôle sa propre base, ses unités et ses crédits, dans la même équipe. Les deux joueurs peuvent choisir la même faction ; leurs commandes et leurs armées restent séparées.

La campagne coop suit les neuf étapes de la campagne d’origine de la faction choisie par l’hôte au lancement. Ses terrains, structures, forces adverses et objectifs sont conservés. La campagne source reste fixée lors des reprises ; la progression et les checkpoints restent indépendants du solo.

Factions disponibles : 12 dans Tornie et Jericho, 6 dans Tornie Lite et Jericho Lite, 9 en Vanilla. Les 45 campagnes sources fournissent 405 variantes coop, à raison de neuf étapes chacune.

| Édition | Factions disponibles | Missions coop |
|---|---:|---:|
| Vanilla | 9 | 9 |
| Tornie | 12 | 9 |
| Jericho | 12 | 9 |
| Tornie Lite | 6 | 9 |
| Jericho Lite | 6 | 9 |

## Créer et rejoindre une partie

1. Utiliser le **même version 1.0.536** sur les deux PC. Son protocole réseau est la version **5**. La 1.0.535 utilise un autre protocole réseau et ne peut pas rejoindre ce lobby.
2. Activer la même édition dans le menu **Mods**. Utiliser deux noms de joueur différents.
3. Ouvrir **Campagne coop**, puis choisir le navigateur LAN ou Internet. L’hôte crée la partie et choisit **Nouvelle campagne**. Le second joueur rejoint sa partie depuis le navigateur ou la connexion par adresse.
4. Dans le lobby, remplir les deux places humaines et choisir les factions. Les factions peuvent être identiques. Les deux joueurs sont alliés ; les places adverses restent contrôlées par l’IA.
5. L’hôte lance la partie après la synchronisation des versions, règles et ressources du mod.

Les adversaires suivent la carte d’origine : les premiers niveaux peuvent n’avoir qu’un adversaire. Les finales des mods conservent leurs trois rôles ; certaines finales Vanilla ont quatre ou cinq adversaires. Choisir une faction aussi présente chez l’ennemi reste possible : ses forces conservent une autre couleur et un propriétaire distinct.

Le joueur principal conserve sa base et ses unités de départ. Le partenaire reçoit les mêmes forces et crédits, avec un MCV supplémentaire sur du rocher libre près du chantier principal. Il déploie sa propre base. Les Special et les unités indisponibles utilisent un équivalent compatible avec sa faction.

Les objectifs de récolte sont partagés : l’épice stockée par les deux joueurs est additionnée, avec un quota commun égal à deux fois le quota solo. La victoire est calculée de la même façon sur les deux PC.

Les campagnes coop créées avec les anciennes cartes gardent leur disposition enregistrée. Créer une nouvelle campagne pour recevoir les cartes d’origine et le départ avec MCV.

## Victoire, défaite et reprise

La victoire et la défaite concernent l’équipe. Si une base alliée est détruite, l’autre joueur peut continuer tant que l’équipe possède encore des forces selon les règles de la carte.

Après une mission, les deux joueurs confirment qu’ils sont prêts. Une victoire débloque la mission suivante ; une défaite permet de rejouer la mission actuelle sans avancer la progression. La victoire à la neuvième mission termine la campagne commune. Quitter ou perdre la connexion ne débloque pas une mission.

La progression et les checkpoints sont enregistrés dans le dossier `coop/` du profil utilisateur, à côté de `Dune Legacy.ini` :

```text
coop/
    <session>.ini   progression commune et réglages
    <session>.dls   checkpoint de la mission
```

L’identifiant de session distingue les campagnes communes. Les fichiers de progression et les sauvegardes solo restent séparés. Conserver la progression et son checkpoint dans le profil utilisé pour reprendre la partie.

Pour reprendre, l’hôte active l’édition concernée, ouvre **Campagne coop**, choisit **Reprendre une campagne commune** et sélectionne son fichier `.ini`. Le second joueur rejoint le lobby. Un checkpoint disponible permet de reprendre la mission enregistrée ; sans checkpoint référencé, la progression relance la mission courante depuis son départ. Les noms des deux joueurs peuvent être réattribués dans le lobby sans fusionner leurs bases.

Les enregistrements de progression utilisent un fichier temporaire et une copie de secours pendant le remplacement. Un enregistrement endommagé peut être récupéré depuis cette dernière copie complète. Une campagne terminée reste enregistrée ; créer une nouvelle campagne pour recommencer.

## Options et hauts faits

Le **Mode facile** reste réservé aux campagnes solo : son départ à la mission 2, ses 500 crédits et ses réductions de coût ne s’appliquent pas à la coop. Les deux joueurs et leurs adversaires utilisent les prix normaux de l’édition.

Le lobby propose le **Mode Chaos**, désactivé par défaut, dans les quatre mods ; Vanilla le désactive. Le tirage utilise une graine commune et les règles sont identiques sur les deux PC. Les sauvegardes conservent les règles de la mission.

Les hauts faits **Commander**, la conquête des campagnes et **Maître du chaos / `CHAOS_CONQUEROR`** concernent les campagnes solo. Terminer la campagne commune ne valide pas la campagne solo d’une faction. Les autres hauts faits utilisent leurs règles ordinaires de multijoueur et les événements du joueur local ; les profils de hauts faits restent locaux.

## État de validation et maintenance

La coop de la 1.0.536 est **disponible pour les essais**. La compilation Windows et des tests automatisés ont réussi ; aucune partie humaine complète entre deux PC n’est confirmée. Il reste à vérifier une vraie connexion LAN/Internet, la sélection identique de faction, les transitions après victoire/défaite et la reprise d’un checkpoint avec deux joueurs.

Les templates de campagne d’origine sont dans `coop/faction<id>/coopNN.ini` sous `data/` pour Vanilla et `mods/<mod>/campaign/` pour les mods. Les 45 templates précédents à la racine sont conservés pour les anciennes sessions. `scripts/build-coop-campaign.py --check` vérifie les 405 variantes sources et les cartes précédentes ; `--write` actualise uniquement les templates coop et les checksums. Le cache `config/CoopCampaignTerrain.json` est calculé par l’algorithme `MapSeed.cpp` du jeu.

Le modèle `Campaign/CoopCampaignSession` conserve la session, les participants et la progression. Le contexte `[COOP]` est inclus dans les données de carte de `GameInitSettings`, déjà transmises par le réseau et sauvegardées. Il distingue la maison de contrôle d’une faction et sa colonne de règles, ce qui permet deux joueurs de même faction. Le contrôleur `Campaign/CoopCampaignRuntime` garde la connexion pendant l’enchaînement des missions ; les transitions vérifient le partenaire, la session et l’étape avant d’avancer.
