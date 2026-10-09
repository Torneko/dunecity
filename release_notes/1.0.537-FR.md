# Dune Legacy Tornie 1.0.537

- Depuis le lobby de l’hôte, **Charger une sauvegarde** ouvre deux onglets : **Manuelles** (`mpsave/`, fichiers `.dls`) et **Automatiques** (`coop/`, checkpoints `.dls`). La sauvegarde doit appartenir à une campagne coop du mod actif. Le chargement ferme l’ancien lobby et en ouvre un sur la mission enregistrée ; l’invité doit le rejoindre à nouveau. Annuler ou choisir un fichier incompatible conserve le lobby actuel. La reprise automatique par le fichier de progression `.ini` reste disponible.
- Le lobby coop permet de choisir le type et la difficulté de chaque IA adverse. L’IA de campagne reste le choix initial ; les choix qBot, Mentat, SmartBot et AI Player sont également proposés.
- Une IA peut remplacer le deuxième joueur. Elle garde sa base, son armée et ses crédits dans l’équipe de l’hôte. Le passage entre missions ne demande aucune confirmation d’un invité absent.
- **Contrôles alliés** est une option du lobby, désactivée par défaut : les alliés peuvent donner des ordres aux unités et aux bâtiments de leur équipe sans fusionner leurs propriétaires ou leurs crédits. Sans cette option, les commandes restent séparées.
- Les nouvelles intros coop donnent un WOR au partenaire, sur de la roche libre aussi près que possible du WOR principal. En l’absence de WOR principal, le chantier sert de référence. Les unités de départ copiées et le MCV supplémentaire sont conservés.
- Si un adversaire utilise la faction de l’un des deux alliés, il reçoit une autre faction disponible. Ses forces, son emplacement et son rôle de campagne sont conservés. Les couleurs des deux alliés et des adversaires sont distinctes ; la sélection des couleurs met à jour immédiatement l’affichage du lobby.
- L’aide en jeu est disponible en français et en anglais. Elle inclut Jericho Lite et les options coop.
- Mentat Wildspade : bouche déplacée d’un pixel à droite et vers le haut ; yeux et bouche légèrement assombris à l’affichage. Les pixels du fond et les poses au repos restent intacts.

Les options coop sont conservées dans la progression et les checkpoints. Les anciens fichiers restent lisibles et désactivent le contrôle allié si le champ manque. Les nouvelles cartes s’appliquent à une nouvelle campagne ou à une mission suivante, pas aux positions d’un checkpoint existant.

Les deux PC doivent utiliser **1.0.537**, le même mod et le protocole réseau **6**. Les clients 1.0.536 ne peuvent pas rejoindre. Tornie a testé la coop sur ses deux PC et confirmé le chargement des sauvegardes depuis le lobby. Les tests automatisés Windows/Linux couvrent les nouveaux réglages et leur persistance. Les essais prolongés et le test de compatibilité Mint 22.3/noyau 7.0 restent à poursuivre.
