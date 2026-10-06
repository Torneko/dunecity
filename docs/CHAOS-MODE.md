# Chaos Mode — Dune Legacy Tornie 1.0.534

Chaos Mode is off by default. Enable it in Game Options for campaigns, custom games or multiplayer with Tornie, Tornie Lite or Jericho. Vanilla always disables it.

Each new game, including the next campaign mission, draws one donor faction for each eligible building type and faction. Donors must have an enabled, buildable instance of that same building at exactly the recipient's configured technology level. Another faction is preferred when possible. Without a compatible donor, the building keeps its usual rules.

Eligible types: Barracks, WOR, Worfinery, Light Factory, Heavy Factory, Hightech Factory, Starport, Love Factory, Chaos Factory, Palace, Gun Turret, Rocket Turret, Scoutpost, Flamepost and Chemipost. Construction Yard, power, harvesting, storage, radar, IX, repair and other economy/support buildings are excluded.

The complete donor production tree keeps its unit technology levels, upgrades, prerequisites, prices and unit characteristics. Existing special production rules still apply, including Worfinery direct products and five-unit squads. Starport uses its usual CHOAM ordering rules, and Chaos Factory retains its existing global random offers. Palaces use the donor's abilities. Structure characteristics and construction requirements come from the donor building. Buildings remain restricted to the same configured technology level; different prerequisite chains can still affect actual availability.

Your chosen faction keeps its ownership, colours, graphics and voices. A captured building retains the draw of its original faction. Units produced from it inherit its donor's technology using the existing captured-technology mechanism.

The draw uses a separate deterministic random stream derived from the shared game seed. It does not consume terrain or combat randomness. Multiplayer GameInit settings carry the option and seed; all peers generate the same table from the same mod data. The game saves both the modified ObjectData and donor table, so loading never rerolls a game or a queued unit. Peers must use the same game version and mod configuration.

Save version 9826 adds the table; GameInit marker MOD4 adds the option. Older supported saves load with Chaos disabled and retain their saved ObjectData. Older executables cannot load new-format saves. The option hash is unchanged when Chaos is disabled.

Normal unit and building technologies on the documentation website describe games with Chaos disabled.
