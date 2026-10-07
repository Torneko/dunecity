// Generated from config/Achievements.ini.
#pragma once
namespace achievements {
inline constexpr const char* defaultCatalog=R"ACHIEVEMENTS(
; Dune Legacy Tornie — UTF-8, local achievements. No gameplay effects.
; Rule + Statistic + Target define extensible rules. Secret=true hides locked names.
; Roadkill uses a 2000 ms simulation window; spice credits count refinery deposits.

[PACIFISM]
Name=Pacifism
NameFr=Pacifisme
Description=Win without destroying enemy units or structures. Captures are allowed.
DescriptionFr=Gagner sans détruire d’unité ou de bâtiment ennemi. Les captures sont autorisées.
Rule=Pacifism
Statistic=
Target=1
Secret=false

[TRUE_PACIFIST]
Name=True Pacifist
NameFr=Véritable pacifiste
Description=Win without dealing damage to an opponent.
DescriptionFr=Gagner sans infliger de dégâts à un adversaire.
Rule=TruePacifist
Statistic=
Target=1
Secret=false

[PRISE_DE_CONTROLE]
Name=Taking Control
NameFr=Prise de contrôle
Description=Capture your first enemy structure with infantry.
DescriptionFr=Capturer un premier bâtiment ennemi avec de l’infanterie.
Rule=Lifetime
Statistic=StructuresCaptured
Target=1
Secret=false

[INFILTRATION]
Name=Infiltration
NameFr=Infiltration
Description=Capture 10 enemy structures in total.
DescriptionFr=Capturer 10 bâtiments ennemis au total.
Rule=Lifetime
Statistic=StructuresCaptured
Target=10
Secret=false

[SANS_TIRER]
Name=Without Firing a Shot
NameFr=Sans tirer un coup de feu
Description=Win after capturing an enemy structure, without any enemy destruction attributed to you.
DescriptionFr=Gagner en capturant un bâtiment ennemi sans destruction ennemie attribuée au joueur.
Rule=CapturePacifism
Statistic=
Target=1
Secret=false

[NO_MERCY]
Name=No Mercy
NameFr=Aucune pitié
Description=Win campaign mission 1 with no enemy units or structures remaining.
DescriptionFr=Gagner la mission 1 d’une campagne après élimination de toutes les forces adverses.
Rule=NoMercy
Statistic=
Target=1
Secret=false

[TOTAL_ANNIHILATION]
Name=Total Annihilation
NameFr=Anéantissement total
Description=Win a skirmish or custom game with no opposing units or structures remaining.
DescriptionFr=Gagner une escarmouche ou partie personnalisée sans forces adverses restantes.
Rule=Annihilation
Statistic=
Target=1
Secret=false

[ROADKILL]
Name=Roadkill
NameFr=Charge dévastatrice
Description=Crush 5 enemy infantry with the same vehicle within 2 simulation seconds.
DescriptionFr=Écraser 5 fantassins ennemis avec le même véhicule en 2 secondes de simulation.
Rule=Roadkill
Statistic=
Target=5
Secret=false

[STEAMROLLER]
Name=Steamroller
NameFr=Rouleau compresseur
Description=Crush 10 enemy infantry in one match.
DescriptionFr=Écraser 10 fantassins ennemis dans une même partie.
Rule=Match
Statistic=InfantryCrushed
Target=10
Secret=false

[TRUE_COLORS]
Name=True Colors
NameFr=Vos vraies couleurs
Description=Win a custom or multiplayer game with a nonstandard house color.
DescriptionFr=Gagner une partie personnalisée ou multijoueur avec une couleur différente de celle de la maison.
Rule=TrueColors
Statistic=
Target=1
Secret=false

[THE_SPICE_MUST_FLOW]
Name=The Spice Must Flow
NameFr=L’épice doit couler
Description=Refine 10,000 spice credits in one match.
DescriptionFr=Raffiner 10 000 crédits d’épice dans une partie.
Rule=Match
Statistic=SpiceHarvested
Target=10000
Secret=false

[SPICE_EMPIRE]
Name=Spice Empire
NameFr=Empire de l’épice
Description=Refine 100,000 spice credits in total.
DescriptionFr=Raffiner 100 000 crédits d’épice cumulés.
Rule=Lifetime
Statistic=SpiceHarvested
Target=100000
Secret=false

[FIRST_VICTORY]
Name=First Victory
NameFr=Première victoire
Description=Win your first match.
DescriptionFr=Gagner une première partie.
Rule=Lifetime
Statistic=GamesWon
Target=1
Secret=false

[CONQUER_ARRAKIS]
Name=Conquer Arrakis
NameFr=Conquérir Arrakis
Description=Complete a campaign by winning its final mission.
DescriptionFr=Terminer une campagne en gagnant sa mission finale.
Rule=CampaignCount
Statistic=Any
Target=1
Secret=false

[MASTER_OF_ARRAKIS]
Name=Master of Arrakis
NameFr=Maître d’Arrakis
Description=Complete campaigns for all eight H/A/O/F/S/M/N/R houses across the included mods.
DescriptionFr=Terminer les campagnes des huit maisons H/A/O/F/S/M/N/R, tous modes confondus.
Rule=CampaignCount
Statistic=
Target=8
Secret=false

[ATREIDES_COMMANDER]
Name=Atreides Commander
NameFr=Commandant Atreides
Description=Complete an Atreides campaign.
DescriptionFr=Terminer une campagne Atreides.
Rule=CampaignHouse
Statistic=Atreides
Target=1
Secret=false

[HARKONNEN_COMMANDER]
Name=Harkonnen Commander
NameFr=Commandant Harkonnen
Description=Complete a Harkonnen campaign.
DescriptionFr=Terminer une campagne Harkonnen.
Rule=CampaignHouse
Statistic=Harkonnen
Target=1
Secret=false

[ORDOS_COMMANDER]
Name=Ordos Commander
NameFr=Commandant Ordos
Description=Complete an Ordos campaign.
DescriptionFr=Terminer une campagne Ordos.
Rule=CampaignHouse
Statistic=Ordos
Target=1
Secret=false

[FREMEN_COMMANDER]
Name=Fremen Commander
NameFr=Commandant Fremen
Description=Complete a Fremen campaign.
DescriptionFr=Terminer une campagne Fremen.
Rule=CampaignHouse
Statistic=Fremen
Target=1
Secret=false

[SARDAUKAR_COMMANDER]
Name=Sardaukar Commander
NameFr=Commandant Sardaukar
Description=Complete a Sardaukar campaign.
DescriptionFr=Terminer une campagne Sardaukar.
Rule=CampaignHouse
Statistic=Sardaukar
Target=1
Secret=false

[MERCENARY_COMMANDER]
Name=Mercenary Commander
NameFr=Commandant mercenaire
Description=Complete a Mercenary campaign.
DescriptionFr=Terminer une campagne des Mercenaires.
Rule=CampaignHouse
Statistic=Mercenary
Target=1
Secret=false

[NEUTRAL_COMMANDER]
Name=Neutral Commander
NameFr=Commandant neutre
Description=Complete a Neutral campaign.
DescriptionFr=Terminer une campagne des Neutres.
Rule=CampaignHouse
Statistic=Neutral
Target=1
Secret=false

[REBEL_COMMANDER]
Name=Rebel Commander
NameFr=Commandant rebelle
Description=Complete a Rebel campaign.
DescriptionFr=Terminer une campagne des Rebelles.
Rule=CampaignHouse
Statistic=Rebels
Target=1
Secret=false

[NO_CASUALTIES]
Name=No Casualties
NameFr=Aucune perte
Description=Win a match without losing a unit.
DescriptionFr=Gagner une partie sans perdre d’unité.
Rule=NoCasualties
Statistic=
Target=1
Secret=false

[WORM_HUNTER]
Name=Worm Hunter
NameFr=Chasseur de vers
Description=Kill a sandworm.
DescriptionFr=Tuer un ver des sables.
Rule=Lifetime
Statistic=SandwormsKilled
Target=1
Secret=false

[SPICE_BLOOM]
Name=Spice Bloom
NameFr=Éclosion d’épice
Description=Trigger your first spice bloom.
DescriptionFr=Déclencher une première éclosion d’épice.
Rule=Lifetime
Statistic=SpiceBloomsTriggered
Target=1
Secret=false

[VETERAN_COMMANDER]
Name=Veteran Commander
NameFr=Commandant vétéran
Description=Win 25 matches.
DescriptionFr=Gagner 25 parties.
Rule=Lifetime
Statistic=GamesWon
Target=25
Secret=false

[BUILDER]
Name=Builder
NameFr=Bâtisseur
Description=Build 100 structures in total.
DescriptionFr=Construire 100 structures au total.
Rule=Lifetime
Statistic=StructuresBuilt
Target=100
Secret=false

[ARMY_OF_ARRAKIS]
Name=Army of Arrakis
NameFr=Armée d’Arrakis
Description=Produce 500 units in total.
DescriptionFr=Produire 500 unités au total.
Rule=Lifetime
Statistic=UnitsBuilt
Target=500
Secret=false

[PALACE_POWER]
Name=Palace Power
NameFr=Pouvoir du palais
Description=Successfully activate a palace ability.
DescriptionFr=Activer avec succès une capacité de palais.
Rule=Lifetime
Statistic=PalaceAbilitiesUsed
Target=1
Secret=false

[FLAME_MASTER]
Name=Flame Master
NameFr=Maître des flammes
Description=Destroy 50 enemies with Flame Tanks in total.
DescriptionFr=Détruire 50 ennemis avec des chars lance-flammes au total.
Rule=Lifetime
Statistic=FlameTankKills
Target=50
Secret=false

[TORNIE_ARSENAL]
Name=Tornie Arsenal
NameFr=Arsenal Tornie
Description=Produce three different Tornie-exclusive unit types in one match.
DescriptionFr=Produire trois types différents d’unités exclusives de Tornie dans une partie.
Rule=Arsenal
Statistic=
Target=3
Secret=false

[AGAINST_THE_ODDS]
Name=Against the Odds
NameFr=Contre toute attente
Description=Win a custom game against at least one Hard or Brutal enemy AI.
DescriptionFr=Gagner une partie personnalisée contre au moins une IA ennemie difficile ou brutale.
Rule=HighDifficulty
Statistic=
Target=1
Secret=false

[HOUSE_COLLECTOR]
Name=House Collector
NameFr=Collectionneur de maisons
Description=Win with all twelve factions: Atreides, Harkonnen, Ordos, Fremen, Sardaukar, Mercenary, Neutral, Rebels, Wildspade, Kleshmersh, Tharpique and Corruptique.
DescriptionFr=Gagner avec les douze factions : Atreides, Harkonnen, Ordos, Fremen, Sardaukar, Mercenary, Neutral, Rebels, Wildspade, Kleshmersh, Tharpique et Corruptique.
Rule=VictoryHouses
Statistic=
Target=12
Secret=false

[RED_HARVEST]
Name=Red Harvest
NameFr=Récolte rouge
Description=Collect red spice with a harvester.
DescriptionFr=Collecter de l’épice rouge avec une moissonneuse.
Rule=SpiceTypes
Statistic=
Target=2
Secret=false

[GREEN_HARVEST]
Name=Green Harvest
NameFr=Récolte verte
Description=Collect green spice with a harvester.
DescriptionFr=Collecter de l’épice verte avec une moissonneuse.
Rule=SpiceTypes
Statistic=
Target=4
Secret=false

[SPICE_COLLECTOR]
Name=Spice Collector
NameFr=Collectionneur d’épices
Description=Collect at least two different spice types in one match.
DescriptionFr=Collecter au moins deux types différents d’épice dans une partie.
Rule=SpiceCount
Statistic=
Target=2
Secret=false

[RULER_OF_ARRAKIS]
Name=Ruler of Arrakis
NameFr=Souverain d’Arrakis
Description=Win a campaign mission with a displayed score of at least 1000.
DescriptionFr=Gagner une mission de campagne avec un score affiché de 1000 ou plus.
Rule=Lifetime
Statistic=BestCampaignScore
Target=1000
Secret=false

[WILDSPADE_COMMANDER]
Name=Wildspade Commander
NameFr=Commandant Wildspade
Description=Complete the Wildspade campaign.
DescriptionFr=Terminer la campagne Wildspade.
Rule=CampaignHouse
Statistic=Wildspade
Target=1
Secret=false

[KLESHMERSH_COMMANDER]
Name=Kleshmersh Commander
NameFr=Commandant Kleshmersh
Description=Complete the Kleshmersh campaign.
DescriptionFr=Terminer la campagne Kleshmersh.
Rule=CampaignHouse
Statistic=Kleshmersh
Target=1
Secret=false

[THARPIQUE_COMMANDER]
Name=Tharpique Commander
NameFr=Commandant Tharpique
Description=Complete the Tharpique campaign.
DescriptionFr=Terminer la campagne Tharpique.
Rule=CampaignHouse
Statistic=Tharpique
Target=1
Secret=false

[CORRUPTIQUE_COMMANDER]
Name=Corruptique Commander
NameFr=Commandant Corruptique
Description=Complete the Corruptique campaign.
DescriptionFr=Terminer la campagne Corruptique.
Rule=CampaignHouse
Statistic=Corruptique
Target=1
Secret=false

[JERICHO_MASTER]
Name=Master of Jericho
NameFr=Maître de Jericho
Description=Complete all twelve campaigns in Jericho.
DescriptionFr=Terminer les douze campagnes dans Jericho.
Rule=ModCampaignCount
Statistic=Jericho
Target=12
Secret=false

[MISSILE_BARRAGE]
Name=Missile Barrage
NameFr=Barrage de missiles
Description=Launch at least three palace missiles in a single match.
DescriptionFr=Lancer au moins trois missiles de palais dans une même partie.
Rule=Match
Statistic=PalaceMissilesLaunched
Target=3
Secret=false

[BLUE_HARVEST]
Name=Blue Harvest
NameFr=Récolte bleue
Description=Collect blue spice with a harvester.
DescriptionFr=Collecter de l’épice bleue avec une moissonneuse.
Rule=SpiceTypes
Statistic=
Target=16
Secret=false

[PURPLE_HARVEST]
Name=Purple Harvest
NameFr=Récolte mauve
Description=Collect purple spice with a harvester.
DescriptionFr=Collecter de l’épice mauve avec une moissonneuse.
Rule=SpiceTypes
Statistic=
Target=8
Secret=false

[CHAOS_CONQUEROR]
Name=Chaos Conqueror
NameFr=Maître du chaos
Description=Complete a campaign started with Chaos Mode enabled.
DescriptionFr=Terminer une campagne commencée avec le mode Chaos activé.
Rule=ChaosCampaign
Statistic=
Target=1
Secret=false

)ACHIEVEMENTS";
}
