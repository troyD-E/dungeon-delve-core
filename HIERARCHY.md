# Build 3: Entity Hierarchy

## Diagram

```text
Describable
    ^
    | virtual
  Entity
    ^                         (shared virtual Entity base)
    | virtual
 Combatant                 Lootable
    ^   \                   /   ^
    |    \                 /    |
  Hero    \---- Monster --/   Merchant
                  (virtual-base diamond)
```

More explicitly, `Hero` derives from `Combatant`, `Merchant` derives from
`Lootable`, and `Monster` derives from both `Combatant` and `Lootable`.
`Combatant` and `Lootable` each virtually inherit `Entity`; `Entity` virtually
inherits `Describable`.

## Inheritance-edge substitutability

- **`Describable` → `Entity`:** `Describable::describe(std::ostream&)` can be
  called through a `Describable&` referring to an `Entity`; virtual dispatch
  reaches the concrete type's description.
- **`Entity` → `Combatant`:** `Combatant::take_damage(int)` works on a
  `Combatant&` referring to either a `Hero` or `Monster`, applying that
  object's armor and hit points.
- **`Entity` → `Lootable`:** `Lootable::loot_value() const` works through a
  `Lootable&` referring to either a `Monster` or `Merchant`, returning the
  appropriate gold value.
- **`Combatant` → `Hero`:** `Roster::add(const Entity&)` accepts a `Hero`
  through its `Combatant` and shared `Entity` bases; `Hero::describe()` then
  supplies its dynamic description.
- **`Combatant` → `Monster`:** `Combatant::take_damage(int)` works through a
  `Combatant&` referring to a `Monster`; virtual dispatch reaches
  `Monster::take_damage(int)` so reaching zero HP also marks it dead.
- **`Lootable` → `Monster`:** `Lootable::loot_value() const` works through a
  `Lootable&` referring to a `Monster` and returns its `gold_drop`.
- **`Lootable` → `Merchant`:** `Lootable::loot_value() const` works through a
  `Lootable&` referring to a `Merchant` and returns its current `gold`.

## Most debatable decision

The most debatable decision is making both mixins virtually inherit `Entity` instead of placing the shared identity only in a separate common interface.

`Monster` needs to be both a `Combatant` and `Lootable`, and all three concrete types also need the common `Entity` API used by `Roster::add(const Entity&)`.

Virtual inheritance ensures that a `Monster` has one `Entity` subobject andone `name`, so access and conversion through either mixin are unambiguous.

The tradeoff is that virtual inheritance adds complexity to construction. The most derived class must initialize the virtual base if `Entity` later gains a
non-default constructor.
