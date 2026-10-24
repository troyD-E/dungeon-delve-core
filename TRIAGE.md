# Build 1: Baseline and Triage

## 1. The three defects

### Defect 1 — Armor is applied twice
*How I found it:* The `encounter_resolves` regression test compares damage against otherwise identical monsters with different armor. Tracing `resolve_attack_on_monster()` and `resolve_attack_on_hero()` into the damage helpers reveals the repeated calculation.
*What was actually wrong:* Each attack resolver applied `apply_armor()` and then passed the reduced value to `apply_damage_to_monster()` or `apply_damage_to_hero()`, which applies armor again. The resolver also returned its first calculation rather than the amount actually applied.
*What I changed:* The attack resolvers now pass raw rolled damage to the damage helpers and return the helpers' result, so armor is applied once and the reported damage corresponds to that calculation.

### Defect 2 — Loot can be selected past the end of the table
*How I found it:* The `loot_table_intact` test checks that every result from `roll_loot()` points to an entry in the loot table. Tracing the range call shows it can produce an invalid final index.
*What was actually wrong:* `dd_rand_range()` includes both endpoints, but `roll_loot()` used `LOOT_COUNT` as its upper endpoint. Valid indices end at `LOOT_COUNT - 1`, so the last possible random value reads past `LOOT_TABLE`.
*What I changed:* `roll_loot()` now requests an inclusive range from zero through `LOOT_COUNT - 1`.

### Defect 3 — Summoning invalidates the monster-round iterator
*How I found it:* The `roster_survives_reallocation` test makes a full `Wave` and has its first monster summon while the round is iterating. Tracing the loop shows that adding the summon can reallocate the vector.
*What was actually wrong:* `run_monster_round()` retained a `Wave` iterator across `resolve_ability()`. A summon calls `wave.push_back()`, which may reallocate the vector and invalidate that iterator; continuing the loop then has undefined behavior.
*What I changed:* The round now iterates by index and rechecks `wave.size()`, so vector growth does not invalidate its traversal state. Newly summoned monsters can still act later in the same round, matching the existing encounter flow.

## 2. Ownership table

The table describes ownership before conversion in the starter code and the owner model now in place.

| Allocation site | Who owned it before | Converted to | Why that choice |
|:--|:--|:--|:--|
| `make_hero()` (`new Hero`) | The caller held the returned raw pointer, such as `hero_owner` in `run_campaign()`, but nothing deleted it. | `HeroPtr` (`std::unique_ptr<Hero>`) | The campaign has one owner; local scope should release it automatically. |
| `make_monster()` (`new Monster`) | The caller stored the pointer in a `Bestiary` or `Wave`; those containers did not delete their pointed-to monsters. | `MonsterPtr` (`std::unique_ptr<Monster>`) | A monster belongs to one container at a time. Moving its owner into that container gives it a clear lifetime. |
| `make_merchant()` (`new Merchant`) | The caller held the returned raw pointer, such as `yarrow_owner` in `run_campaign()`, but nothing deleted it. | `MerchantPtr` (`std::unique_ptr<Merchant>`) | The campaign is its sole owner. |
| `load_hero()` (`new Hero`) | The successful result was returned for its caller to own, but there is no call to `load_hero()` in the current engine. The first-line error path manually deleted the allocation. | `HeroPtr` (`std::unique_ptr<Hero>`) | Ownership is explicit on return, and early failure releases the hero automatically. |

Non-owning observations remain references or raw pointers. `find_in_bestiary()` returns a `Monster*` into a `Bestiary` that retains ownership; `proto` in `spawn_from_bestiary()` and `m` in `run_monster_round()` are temporary observations. `resolve_ability()` borrows its `Monster&` and `Hero&`; its `Wave&` remains the owner of its monsters. Comments mark these borrowed pointers/references and the optional `acted_names` pointer. The `raw()` helper explicitly returns a non-owning `Monster*`.

## 3. The three worst things about this codebase

1. [include/dungeon_delve.h](./include/dungeon_delve.h:33), [include/dungeon_delve.h](./include/dungeon_delve.h:45), [include/dungeon_delve.h](./include/dungeon_delve.h:59): entity names use fixed-size `char` arrays, and the construction code truncates input to fit. A later build should plan a safe conversion to `std::string` and consider save-file compatibility; this is Week 2 work.
2. [src/dungeon_delve.cpp](./src/dungeon_delve.cpp:254): `resolve_ability()` is a large switch on integer tags, making behavior harder to extend without editing the central function. Redesigning ability dispatch belongs to Week 4, not this ownership build.
3. [include/dungeon_delve.h](./include/dungeon_delve.h:33), [include/dungeon_delve.h](./include/dungeon_delve.h:45), [include/dungeon_delve.h](./include/dungeon_delve.h:58): `Hero`, `Monster`, and `Merchant` duplicate several fields and represent related concepts separately. The types should be reconsidered in the planned Week 3 architecture work rather than refactored during Week 1.
