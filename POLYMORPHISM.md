# Build 04: The Ability System

## Resolver comparison

Raw line counts are for the resolver functions themselves, including braces
and blank lines, but excluding nearby section comments.

| Before: tag-switch resolver | After: polymorphic resolver |
|---|---|
| `resolve_ability(int tag, ...)`: **101 lines** in the original `src/dungeon_delve.cpp` (lines 485–585). | `resolve_ability(const Ability&, ...)` plus `resolve_actions(...)`: **26 lines** in the updated `src/dungeon_delve.cpp` (lines 655–680); the dispatch loop itself is **4 lines**. |
| `switch (tag)` selected an effect; each `case ABIL_*` implemented behavior. | `resolve_ability(*action.ability, action.context)` calls the virtual `apply()` through an `Ability` reference. |

The complete old resolver:

```cpp
int resolve_ability(int tag, Monster& self, Hero& hero,
                    Wave& wave, int verbose)
{
    int amount = 0;

    switch (tag) {

        case ABIL_NONE:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s flails .................. %d damage\n",
                            self.name, amount);
            break;

        case ABIL_CLEAVE:
            amount = resolve_attack_on_hero(self, hero);
            amount = amount + (amount / 2);
            apply_damage_to_hero(hero, amount / 2);
            if (verbose)
                std::printf("  %s cleaves ................. %d damage\n",
                            self.name, amount);
            break;

        case ABIL_BITE:
            amount = resolve_attack_on_hero(self, hero);
            if (dd_rand_range(1, 100) > 70) {
                apply_damage_to_hero(hero, 2);
                amount += 2;
                if (verbose)
                    std::printf("  %s bites deep ............. %d damage\n",
                                self.name, amount);
            } else {
                if (verbose)
                    std::printf("  %s bites ................... %d damage\n",
                                self.name, amount);
            }
            break;

        case ABIL_MEND:
            amount = roll_dice(2, 4);
            self.hp += amount;
            if (self.hp > self.max_hp) self.hp = self.max_hp;
            if (verbose)
                std::printf("  %s mends ................... +%d HP\n",
                            self.name, amount);
            break;

        case ABIL_HEX:
            amount = roll_dice(1, 4);
            hero.strength -= amount;
            if (hero.strength < 1) hero.strength = 1;
            if (verbose)
                std::printf("  %s hexes ................... -%d STR\n",
                            self.name, amount);
            break;

        case ABIL_SUMMON:
            if (self.summon_budget > 0) {
                self.summon_budget--;
                MonsterPtr pup = make_monster("Whelp", "whelp", 6, 2, 0, 3, ABIL_NONE);
                wave.push_back(std::move(pup));
                amount = 1;
                if (verbose)
                    std::printf("  %s summons a Whelp!\n", self.name);
            } else {
                amount = resolve_attack_on_hero(self, hero);
                if (verbose)
                    std::printf("  %s is out of whelps ....... %d damage\n",
                                self.name, amount);
            }
            break;

        case ABIL_GUARD:
            self.armor += 2;
            amount = 2;
            if (verbose)
                std::printf("  %s guards .................. +2 armor\n",
                            self.name);
            break;

        case ABIL_RALLY:
            for (size_t i = 0; i < wave.size(); i++) {
                if (!wave[i]->is_dead && raw(wave[i]) != &self)
                    wave[i]->strength += 1;
            }
            amount = 1;
            if (verbose)
                std::printf("  %s rallies the pack ........ +1 STR each\n",
                            self.name);
            break;

        default:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s does something ......... %d damage\n",
                            self.name, amount);
            break;
    }

    return amount;
}
```

The complete new dispatch functions:

```cpp
int resolve_ability(const Ability& ability, AbilityContext& context)
{
    try {
        const AbilityResult result = ability.apply(context);
        if (context.verbose) {
            std::printf("  %s uses %-10s ........... %s%d %s\n",
                        context.actor.name, ability.describe(), result.sign,
                        result.amount, result.unit);
        }
        return result.amount;
    } catch (const std::exception&) {
        std::throw_with_nested(std::runtime_error(
            std::string("Ability ") + ability.describe() + " failed"));
        throw;
    }
}

static int resolve_actions(std::vector<AbilityAction>& actions)
{
    int total = 0;
    for (AbilityAction& action : actions)
    {
        total += resolve_ability(*action.ability, action.context);
    }
    return total;
}
```

The dispatch loop alone:

```cpp
for (AbilityAction& action : actions)
{
    total += resolve_ability(*action.ability, action.context);
}
```

`resolve_ability()` invokes `ability.apply(context)` and handles contextual
exception passing. The concrete ability supplies its own description and
effect. `AttackAbility::apply()` is overridden again by `Cleave::apply()`,
showing that the virtual override remains virtual at a further derived level.
The named `ABIL_*` values retained for existing construction call sites are
references to ability prototypes, not integer tags; the resolver does not
inspect them.

## What got worse

The single switch used to put the roster of effects in one place. Now each ability has its own `describe()`, `apply()`, and `clone()` implementation, while the public interface, prototype references, actor setup, and resolver are elsewhere. That adds boilerplate and makes it less convenient to scan or balance all abilities together. The combat output also changes because Ash now cycles through Cleave and Mend instead of using the old basic attack.

## Open/closed check

I temporarily added a concrete Shatter ability, its prototype, and a bestiary entry, then removed the trial code. The diff touched 1 existing file: `src/dungeon_delve.cpp`. neither the public interface nor the dispatch loop needed edits. The engine already retains eight built in abilities, so Shatter
was technically a ninth in this version, rather than a literal sixth. The file count result is the same for adding one more ability. This was an extension/file count check, not a second build, and does not mean the engine can load arbitrary ability source files without build system changes, its implementation is currently compiled from one source file.
