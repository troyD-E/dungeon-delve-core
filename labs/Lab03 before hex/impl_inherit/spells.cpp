// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION A — INHERITANCE
//
//  Model the three spells in spellbook.h as a class hierarchy: one abstract
//  base, one derived class per spell, virtual dispatch to the effect.
//
//  The base class owns the shared mana rule; each derived class supplies its
//  spell-specific effect.
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

namespace {

class Spell {
public:
    virtual ~Spell() = default;
    virtual const char* name() const = 0;
    virtual int mana_cost() const = 0;

    CastResult cast(Caster& caster, Target& target) const
    {
        CastResult result{false, 0, 0, 0, 0, 0};
        const int cost = mana_cost();
        if (caster.mana < cost)
            return result;

        caster.mana -= cost;
        result.cast = true;
        result.mana_spent = cost;
        effect(caster, target, result);
        return result;
    }

protected:
    virtual void effect(Caster& caster, Target& target,
                        CastResult& result) const = 0;
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

class Fireball final : public Spell {
public:
    const char* name() const override { return "Fireball"; }
    int mana_cost() const override { return 6; }

private:
    void effect(Caster&, Target& target, CastResult& result) const override
    {
        apply_damage(target, 9, result.damage_dealt);
    }
};

class Mend final : public Spell {
public:
    const char* name() const override { return "Mend"; }
    int mana_cost() const override { return 4; }

private:
    void effect(Caster& caster, Target&, CastResult& result) const override
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
};

class Summon final : public Spell {
public:
    const char* name() const override { return "Summon"; }
    int mana_cost() const override { return 8; }

private:
    void effect(Caster& caster, Target& target, CastResult& result) const override
    {
        if (caster.summon_budget > 0) {
            --caster.summon_budget;
            ++caster.minions;
            result.minions_added = 1;
        } else {
            apply_damage(target, 3, result.damage_dealt);
        }
    }
};

const Fireball fireball;
const Mend mend;
const Summon summon;
const Spell* const spells[] = {&fireball, &mend, &summon};
constexpr int spell_total = static_cast<int>(sizeof(spells) / sizeof(spells[0]));

const Spell* spell_at(int index)
{
    if (index < 0 || index >= spell_total)
        return nullptr;
    return spells[index];
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
    return spell == nullptr ? nullptr : spell->name();
}

int spell_mana_cost(int index)
{
    const Spell* spell = spell_at(index);
    return spell == nullptr ? 0 : spell->mana_cost();
}

CastResult cast_spell(int index, Caster& caster, Target& target)
{
    const Spell* spell = spell_at(index);
    if (spell == nullptr)
        return CastResult{false, 0, 0, 0, 0, 0};
    return spell->cast(caster, target);
}
