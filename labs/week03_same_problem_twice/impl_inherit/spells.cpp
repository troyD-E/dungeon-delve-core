// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION A — INHERITANCE
//
//  Model the three spells in spellbook.h as a class hierarchy: one abstract
//  base, one derived class per spell, virtual dispatch to the effect.
//
//  This file compiles as delivered and registers nothing, so the tests fail
//  until you implement it. Build it first anyway -- a scaffold that does not
//  compile is a different problem than the one this lab is about.
//
//  You may add more files to this folder. The build picks up every .cpp in
//  impl_inherit/ automatically; you never edit CMakeLists.txt. Whether you
//  ended up adding files, and how many, is one of the things you measure.
//
//  RULES FOR THIS VERSION
//    - Inheritance does the dispatch. No switch, no if-chain, no enum tag
//      deciding which effect runs.
//    - The shared "not enough mana" rule is written ONCE. If you find
//      yourself pasting it into each spell, stop and move it.
// ===========================================================================
#include "spellbook.h"

// ------------------------------------------------------------------ sketch
//
// A shape that works. You do not have to use it, but if you invent your own,
// it still has to satisfy spellbook.h exactly.
//
//   class Spell {
//   public:
//       virtual ~Spell() = default;
//       virtual const char* name() const = 0;
//       virtual int mana_cost() const = 0;
//
//       // The shared rule lives here, in the base, and calls the hook below
//       // only once the mana is paid.
//       CastResult cast(Caster& caster, Target& target) const;
//
//   protected:
//       // What makes one spell different from another. Nothing else.
//       virtual void effect(Caster& caster, Target& target,
//                           CastResult& out) const = 0;
//   };
//
// Then Fireball, Mend and Summon derive from Spell and override name(),
// mana_cost() and effect(). A file-scope table of pointers to one static
// instance of each gives you spell_count(), spell_name() and cast_spell().
//
// Ownership note, since Build 1 just made you care about this: static
// instances with static storage duration need no new and no delete, and the
// sanitizer will not report them. If you reach for new here, you also own
// the matching delete.

// TODO: declare the abstract base class and its three derived classes here.

// TODO: build the registry -- whatever lets the four functions below answer
//       by index. Keep the order the same as in impl_compose/spells.cpp so
//       the two drivers print identical transcripts.

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
