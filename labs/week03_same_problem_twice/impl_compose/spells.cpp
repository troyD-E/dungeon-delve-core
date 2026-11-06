// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION B — COMPOSITION
//
//  Model the same three spells from spellbook.h with no inheritance at all.
//  A spell is a plain value that HAS the pieces that vary -- a name, a cost,
//  and a thing it does -- rather than a type that IS a kind of spell.
//
//  A Spell value combines its name, mana cost, and effect function.
//
//  You may add more files to this folder. The build picks up every .cpp in
//  impl_compose/ automatically; you never edit CMakeLists.txt.
//
//  RULES FOR THIS VERSION
//    - No base classes and no virtual functions. Not one.
//    - The shared "not enough mana" rule is still written ONCE.
//    - Both versions must satisfy spellbook.h identically. If you find
//      yourself wanting to change the header to make this half easier, that
//      is a finding -- write it in ANALYSIS.md instead of changing it.
// ===========================================================================
#include "spellbook.h"

// ------------------------------------------------------------------ sketch
//
// A shape that works. Again, yours may differ, but it must satisfy
// spellbook.h exactly.
//
//   // The part that varies, as data instead of as a type.
//   using EffectFn = void (*)(Caster&, Target&, CastResult&);
//
//   struct Spell {
//       const char* name;
//       int         mana_cost;
//       EffectFn    effect;     // the spell HAS an effect; it is not a kind
//   };                          // of thing that overrides one
//
//   static void fireball_effect(Caster& c, Target& t, CastResult& out) { ... }
//   static void mend_effect    (Caster& c, Target& t, CastResult& out) { ... }
//   static void summon_effect  (Caster& c, Target& t, CastResult& out) { ... }
//
//   static const Spell SPELLS[] = {
//       {"Fireball", 6, &fireball_effect},
//       {"Mend",     4, &mend_effect},
//       {"Summon",   8, &summon_effect},
//   };
//
// A function pointer is the smallest version of this. std::function is
// another; so is a struct holding a small callable. Any of them is
// composition. Pick one and say in ANALYSIS.md why.
//
// The shared mana rule now lives in cast_spell() itself, which is a real
// difference from the inheritance version and worth a line in your write-up:
// there is no base class to hang it on, so it hangs on the dispatcher.

namespace {

using EffectFn = void (*)(Caster&, Target&, CastResult&);

struct Spell {
    const char* name;
    int mana_cost;
    EffectFn effect;
};

static void apply_damage(Target& target, int raw_damage, int& damage_dealt)
{
    damage_dealt = raw_damage - target.ward;
    if (damage_dealt < 1)
        damage_dealt = 1;
    target.hp -= damage_dealt;
    if (target.hp < 0)
        target.hp = 0;
}

static void fireball_effect(Caster&, Target& target, CastResult& result)
{
    apply_damage(target, 9, result.damage_dealt);
}

static void mend_effect(Caster& caster, Target&, CastResult& result)
{
    int healing = 7;
    const int room = caster.max_hp - caster.hp;
    if (healing > room)
        healing = room;
    if (healing < 0)
        healing = 0;
    caster.hp += healing;
    result.healing_done = healing;
}

static void summon_effect(Caster& caster, Target& target, CastResult& result)
{
    if (caster.summon_budget > 0) {
        --caster.summon_budget;
        ++caster.minions;
        result.minions_added = 1;
    } else {
        apply_damage(target, 3, result.damage_dealt);
    }
}

static void hex_effect(Caster&, Target& target, CastResult& result)
{
    int drained = 2;
    if (target.strength - drained < 1)
        drained = target.strength - 1;
    if (drained < 0)
        drained = 0;
    target.strength -= drained;
    result.strength_drained = drained;
}

const Spell spells[] = {
    {"Fireball", 6, &fireball_effect},
    {"Mend",     4, &mend_effect},
    {"Summon",   8, &summon_effect},
    {"Hex",      3, &hex_effect},
};
constexpr int spell_total = static_cast<int>(sizeof(spells) / sizeof(spells[0]));

const Spell* spell_at(int index)
{
    if (index < 0 || index >= spell_total)
        return nullptr;
    return &spells[index];
}

} // namespace

// ---------------------------------------------------------------- the API
//
// These four are the contract from spellbook.h. Replace every stub below.

int spell_count()
{
    return spell_total;
}

const char* spell_name(int index)
{
    const Spell* spell = spell_at(index);
    return spell == nullptr ? nullptr : spell->name;
}

int spell_mana_cost(int index)
{
    const Spell* spell = spell_at(index);
    return spell == nullptr ? 0 : spell->mana_cost;
}

CastResult cast_spell(int index, Caster& caster, Target& target)
{
    const Spell* spell = spell_at(index);
    if (spell == nullptr)
        return CastResult{false, 0, 0, 0, 0, 0};

    CastResult result{false, 0, 0, 0, 0, 0};
    if (caster.mana < spell->mana_cost)
        return result;

    caster.mana -= spell->mana_cost;
    result.cast = true;
    result.mana_spent = spell->mana_cost;
    spell->effect(caster, target, result);
    return result;
}
