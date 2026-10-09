# Campagne coop commune — 1.0.538

Tornie a confirmé le bon fonctionnement de la coop 1.0.538 sur deux PC (2026-10-09).

[English guide](COOP-CAMPAIGN-EN.md)

L’entrée **Campagne coop** du menu principal permet de jouer à deux sur LAN ou Internet. Chaque joueur choisit librement sa faction et contrôle sa propre base, ses unités et ses crédits, dans la même équipe. Les alliés peuvent choisir la même faction. Leurs propriétaires et crédits restent séparés ; les contrôles peuvent être partagés avec l’option du lobby.

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

1. Utiliser la **même version 1.0.538** sur les deux PC. Son protocole réseau est la version **7**. La 1.0.536 utilise un autre protocole réseau et ne peut pas rejoindre ce lobby.
2. Activer la même édition dans le menu **Mods**. Utiliser deux noms de joueur différents.
3. Ouvrir **Campagne coop**, puis choisir le navigateur LAN ou Internet. L’hôte crée la partie et choisit **Nouvelle campagne**. Le second joueur rejoint sa partie depuis le navigateur ou la connexion par adresse.
4. Dans le lobby, connecter un invité ou sélectionner une IA au deuxième emplacement, puis choisir les factions. L’hôte peut régler le type et la difficulté de chaque IA adverse. Les factions peuvent être identiques. Les deux joueurs sont alliés ; les places adverses restent contrôlées par l’IA.
5. L’hôte lance la partie après la synchronisation des versions, règles et ressources du mod.

Les adversaires suivent la carte d’origine : les premiers niveaux peuvent n’avoir qu’un adversaire. Les finales des mods conservent leurs trois rôles ; certaines finales Vanilla ont quatre ou cinq adversaires. Si un adversaire utilise la faction d’un des deux alliés, il reçoit une autre faction disponible tout en conservant son rôle et ses forces. Les couleurs des deux alliés et des adversaires sont distinctes.

Le joueur principal conserve sa base et ses unités de départ. Le partenaire reçoit les mêmes forces et crédits, avec un MCV supplémentaire sur du rocher libre près du chantier principal. Il déploie sa propre base. Dans une nouvelle intro, il reçoit aussi un WOR sur de la roche libre, au plus près du WOR principal (ou du chantier si le premier joueur n’a pas de WOR). Les Special et les unités indisponibles utilisent un équivalent compatible avec sa faction.

Les objectifs de récolte sont partagés : l’épice stockée par les deux joueurs est additionnée, avec un quota commun égal à deux fois le quota solo. La victoire est calculée de la même façon sur les deux PC.

Les campagnes coop créées avec les anciennes cartes gardent leur disposition enregistrée. Créer une nouvelle campagne pour recevoir les cartes d’origine et le départ avec MCV.

## Victoire, défaite et reprise

La victoire et la défaite concernent l’équipe. Si une base alliée est détruite, l’autre joueur peut continuer tant que l’équipe possède encore des forces selon les règles de la carte.

Après une mission avec deux humains, chacun confirme qu’il est prêt. Avec une IA alliée, l’hôte peut continuer seul. Une victoire débloque la mission suivante ; une défaite permet de rejouer la mission actuelle sans avancer la progression. La victoire à la neuvième mission termine la campagne commune. Quitter ou perdre la connexion ne débloque pas une mission.

La progression et les checkpoints sont enregistrés dans le dossier `coop/` du profil utilisateur, à côté de `Dune Legacy.ini` :

```text
coop/
    <session>.ini   progression commune et réglages
    <session>.dls   checkpoint de la mission
```

L’identifiant de session distingue les campagnes communes. Les fichiers de progression et les sauvegardes solo restent séparés. Conserver la progression et son checkpoint dans le profil utilisé pour reprendre la partie.

Pour reprendre, l’hôte active l’édition concernée, ouvre **Campagne coop**, choisit **Reprendre une campagne commune** et sélectionne son fichier `.ini`. Le second joueur rejoint le lobby. Un checkpoint disponible permet de reprendre la mission enregistrée ; sans checkpoint référencé, la progression relance la mission courante depuis son départ. Les noms des deux joueurs peuvent être réattribués dans le lobby sans fusionner leurs bases.

Les enregistrements de progression utilisent un fichier temporaire et une copie de secours pendant le remplacement. Un enregistrement endommagé peut être récupéré depuis cette dernière copie complète. Une campagne terminée reste enregistrée ; créer une nouvelle campagne pour recommencer.

Depuis le lobby de l’hôte, **Charger une sauvegarde** ouvre deux onglets : **Manuelles** (`mpsave/`, fichiers `.dls`) et **Automatiques** (`coop/`, checkpoints `.dls`). La sauvegarde doit appartenir à une campagne coop du mod actif. Le chargement ferme l’ancien lobby et en ouvre un sur la mission enregistrée ; l’invité doit le rejoindre à nouveau. Annuler ou choisir un fichier incompatible conserve le lobby actuel. La reprise automatique par le fichier de progression `.ini` reste disponible.

## Contrôles alliés et IA

**Contrôles alliés**, désactivé par défaut, autorise les ordres aux unités et bâtiments de l’allié : déplacement, combat, production, placement, réparation et capacités. La production utilise les crédits du propriétaire. Les maisons et leurs statistiques restent séparées. Sans cette option, chacun contrôle uniquement ses objets.

Au deuxième emplacement, l’hôte peut choisir une IA qBot, Mentat, SmartBot ou AI Player. Il choisit aussi le type et la difficulté des IA adverses ; l’IA de campagne reste la valeur initiale. Ces choix et le contrôle allié persistent lors des missions suivantes et des reprises. Les types supportent leurs niveaux propres, du défensif au brutal selon l’IA choisie.

## Options et hauts faits

## Nouveautés 1.0.538

Deux options de lobby sont désactivées par défaut : **Mode facile** et **Forces ennemies +5**.
Le mode facile commence une nouvelle campagne à la mission 2. Chacun des deux alliés,
humain ou IA, reçoit 500 crédits supplémentaires au départ de cette mission seulement.
Leurs achats d’unités et de bâtiments coûtent 25 crédits de moins (minimum 1) pendant
la campagne. Les ennemis ne reçoivent ni crédits ni réduction.

Forces ennemies +5 ajoute cinq unités adaptées à la technologie par adversaire présent
à chaque mission, près de ses bâtiments ou forces. Un adversaire arrivant uniquement
par renforts reçoit ses unités sur une bordure opposée à la base principale. Les unités
commencent en garde de zone. Les adversaires absents de la mission ne reçoivent rien.

Les renforts destinés au premier joueur sont aussi envoyés au second allié, avec le
même nombre, le même moment, le même lieu relatif et la même répétition. Une unité
indisponible est adaptée à la faction alliée. Les renforts et forces déjà enregistrés
dans un checkpoint sont restaurés, jamais ajoutés une seconde fois.

**Ensemble sur Arrakis / COOP_CONQUEROR** demande de terminer toutes les étapes de la
campagne commune, avec un humain ou une IA. Le mode facile autorise le saut de l’intro.
Le déblocage reste local et une session terminée ne compte qu’une fois dans le cumul.
Les succès des campagnes solo gardent leurs conditions.

Les deux PC doivent utiliser **1.0.538**, protocole **7**. Les nouvelles sauvegardes
portent la version **9829** ; les anciennes restent lisibles avec leurs forces et
renforts enregistrés. Commencer une nouvelle campagne ou passer à la mission suivante
pour profiter des nouveaux renforts. La version publique est 1.0.538. La coop sur deux PC est confirmée par Tornie. Mint 22.3 / noyau 7.0 reste à essayer.


Le lobby propose le **Mode Chaos**, désactivé par défaut, dans les quatre mods ; Vanilla le désactive. Le tirage utilise une graine commune et les règles sont identiques sur les deux PC. Les sauvegardes conservent les règles de la mission.

Les hauts faits **Commander**, la conquête des campagnes et **Maître du chaos / `CHAOS_CONQUEROR`** concernent les campagnes solo. Terminer la campagne commune ne valide pas la campagne solo d’une faction. Les autres hauts faits utilisent leurs règles ordinaires de multijoueur et les événements du joueur local ; les profils de hauts faits restent locaux.

## État de validation et maintenance

Tornie a testé la coop 1.0.538 sur deux PC et confirmé son bon fonctionnement le 9 octobre 2026. Le test Mint 22.3 / noyau 7.0 reste à faire sur le PC concerné.

Les templates de campagne d’origine sont dans `coop/faction<id>/coopNN.ini` sous `data/` pour Vanilla et `mods/<mod>/campaign/` pour les mods. Les 45 templates précédents à la racine sont conservés pour les anciennes sessions. `scripts/build-coop-campaign.py --check` vérifie les 405 variantes sources et les cartes précédentes ; `--write` actualise uniquement les templates coop et les checksums. Le cache `config/CoopCampaignTerrain.json` est calculé par l’algorithme `MapSeed.cpp` du jeu.

Le modèle `Campaign/CoopCampaignSession` conserve la session, les participants et la progression. Le contexte `[COOP]` est inclus dans les données de carte de `GameInitSettings`, déjà transmises par le réseau et sauvegardées. Il distingue la maison de contrôle d’une faction et sa colonne de règles, ce qui permet deux joueurs de même faction. Le contrôleur `Campaign/CoopCampaignRuntime` garde la connexion pendant l’enchaînement des missions ; les transitions vérifient le partenaire, la session et l’étape avant d’avancer.
