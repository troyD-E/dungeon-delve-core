// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION B — COMPOSITION
//
//  Model the same three spells from spellbook.h with no inheritance at all.
//  A spell is a plain value that HAS the pieces that vary -- a name, a cost,
//  and a thing it does -- rather than a type that IS a kind of spell.
//
//  This file compiles as delivered and registers nothing, so the tests fail
//  until you implement it. Build it first anyway.
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

// TODO: define the Spell value type and the three effect functions here.

// TODO: build the table. Keep the order the same as in
//       impl_inherit/spells.cpp so the two drivers print identical
//       transcripts.

// ---------------------------------------------------------------- the API
//
// These four are the contract from spellbook.h. Replace every stub below.

int spell_count()
{
    // TODO: return the number of registered spells (3 now, 4 after Step 4).
    return 0;
}

const char* spell_name([[maybe_unused]] int index)
{
    // TODO: return the name of the spell at this index, or nullptr if the
    //       index is out of range.
    return nullptr;
}

int spell_mana_cost([[maybe_unused]] int index)
{
    // TODO: return the mana cost of the spell at this index, or 0 if the
    //       index is out of range.
    return 0;
}

CastResult cast_spell([[maybe_unused]] int index,
                      [[maybe_unused]] Caster& caster,
                      [[maybe_unused]] Target& target)
{
    // TODO: pay the mana if the caster can afford it, run the spell's effect,
    //       and report what actually happened. An out-of-range index casts
    //       nothing: return the all-zero result below.
    return CastResult{false, 0, 0, 0, 0, 0};
}
