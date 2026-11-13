// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  THIS IS THE FILE YOU REPLACE.
//
//  It works. Every supplied test passes against it right now, and that is
//  the baseline you have to preserve: run the tests before you touch
//  anything, so that when they pass afterwards you have proved behavior is
//  unchanged rather than hoped it.
//
//  It dispatches on an integer type tag, which means that adding one ability
//  means editing FIVE separate places in this file. They are marked:
//
//      SITE 1 -- the ALL_TAGS table
//      SITE 2 -- the ability_name() switch
//      SITE 3 -- the ability_mana_cost() switch
//      SITE 4 -- the ability_is_hostile() switch
//      SITE 5 -- the resolve_ability() switch
//
//  Miss one and the program still compiles. Miss SITE 3 and the new ability
//  is free to cast. Miss SITE 1 and it never appears in the roster. This is
//  what the engine's own resolve_ability() looks like, at a quarter of the
//  size, and it is what Build 4 removes.
//
//  You replace this with an abstract base class and one concrete derived
//  class per ability. The build globs impl/*.cpp, so you may delete this
//  file and add as many as you like without editing CMakeLists.txt.
// ===========================================================================
#include "kill_the_switch.h"

#include <cstdio>

// One verbose line, so that each case below is about its effect and not
// about its printf. The exact text is not tested; the numbers are.
static void announce(int verbose, const Monster& self,
                     const char* phrase, int amount)
{
    if (verbose)
        std::printf("  %s %s %d\n", self.name, phrase, amount);
}

// --- SITE 1: the roster ----------------------------------------------------
//
// Every tag that exists, in the order the driver walks them.
static const int ALL_TAGS[] = {
    ABIL_NONE,
    ABIL_CLEAVE,
    ABIL_BITE,
    ABIL_MEND,
    ABIL_HEX,
    ABIL_SUMMON,
    ABIL_GUARD,
    ABIL_RALLY,
};

static const int ALL_TAGS_COUNT =
    static_cast<int>(sizeof(ALL_TAGS) / sizeof(ALL_TAGS[0]));

int ability_count() { return ALL_TAGS_COUNT; }

int ability_tag_at(int index)
{
    if (index < 0 || index >= ALL_TAGS_COUNT) return -1;
    return ALL_TAGS[index];
}

// --- SITE 2: the name switch -----------------------------------------------
const char* ability_name(int tag)
{
    switch (tag) {
        case ABIL_NONE:   return "Flail";
        case ABIL_CLEAVE: return "Cleave";
        case ABIL_BITE:   return "Bite";
        case ABIL_MEND:   return "Mend";
        case ABIL_HEX:    return "Hex";
        case ABIL_SUMMON: return "Summon";
        case ABIL_GUARD:  return "Guard";
        case ABIL_RALLY:  return "Rally";
    }
    return "???";
}

// --- SITE 3: the cost switch -----------------------------------------------
int ability_mana_cost(int tag)
{
    switch (tag) {
        case ABIL_NONE:   return 0;
        case ABIL_CLEAVE: return 3;
        case ABIL_BITE:   return 2;
        case ABIL_MEND:   return 4;
        case ABIL_HEX:    return 5;
        case ABIL_SUMMON: return 8;
        case ABIL_GUARD:  return 2;
        case ABIL_RALLY:  return 6;
    }
    return 0;
}

// --- SITE 4: the hostility switch ------------------------------------------
bool ability_is_hostile(int tag)
{
    switch (tag) {
        case ABIL_NONE:   return true;
        case ABIL_CLEAVE: return true;
        case ABIL_BITE:   return true;
        case ABIL_MEND:   return false;
        case ABIL_HEX:    return true;
        case ABIL_SUMMON: return false;
        case ABIL_GUARD:  return false;
        case ABIL_RALLY:  return false;
    }
    return false;
}

// --- SITE 5: the dispatch switch -------------------------------------------
//
// Do not touch this. It took forever to get right.
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave,
                    int verbose)
{
    // The one rule every ability shares. Written once, here, above the
    // switch -- and note that it has to call SITE 3 to find out what the cost
    // even is.
    const int cost = ability_mana_cost(tag);
    if (self.mana < cost) {
        if (verbose)
            std::printf("  %s tries to %s and fizzles\n",
                        self.name, ability_name(tag));
        return 0;
    }
    self.mana -= cost;

    int amount = 0;

    switch (tag) {
        case ABIL_NONE:
            amount = attack_hero(self, hero);
            announce(verbose, self, "flails ..................", amount);
            break;

        case ABIL_CLEAVE: {
            const int first  = attack_hero(self, hero);
            const int follow = (first / 2 > 1) ? first / 2 : 1;
            hero.hp -= follow;                       // no armor, no 2nd draw
            if (hero.hp < 0) hero.hp = 0;
            amount = first + follow;
            announce(verbose, self, "cleaves .................", amount);
            break;
        }

        case ABIL_BITE: {
            const int first = attack_hero(self, hero);
            const int roll  = lab_rand_range(1, 100);   // drawn either way
            amount = first;
            if (roll > 70) {
                hero.hp -= 2;
                if (hero.hp < 0) hero.hp = 0;
                amount += 2;
            }
            announce(verbose, self, "bites ...................", amount);
            break;
        }

        case ABIL_MEND: {
            const int rolled = roll_dice(2, 4);
            const int room   = (self.max_hp > self.hp) ? self.max_hp - self.hp : 0;
            amount   = (rolled < room) ? rolled : room;
            self.hp += amount;
            announce(verbose, self, "mends ...................", amount);
            break;
        }

        case ABIL_HEX: {
            const int before = hero.strength;
            hero.strength -= roll_dice(1, 4);
            if (hero.strength < 1) hero.strength = 1;
            amount = before - hero.strength;
            announce(verbose, self, "hexes ...................", amount);
            break;
        }

        case ABIL_SUMMON:
            if (self.summon_budget > 0) {
                self.summon_budget--;
                spawn_into_wave(wave, "Whelp", 6, 2, 0);
                amount = 1;
                announce(verbose, self, "summons a Whelp .........", amount);
            } else {
                amount = attack_hero(self, hero);
                announce(verbose, self, "is out of whelps ........", amount);
            }
            break;

        case ABIL_GUARD: {
            const int before = self.armor;
            self.armor = (self.armor + 2 > 8) ? 8 : self.armor + 2;  // the cap
            amount = self.armor - before;
            announce(verbose, self, "guards ..................", amount);
            break;
        }

        case ABIL_RALLY:
            for (const auto& m : wave)
                if (m && !m->is_dead && m.get() != &self) { m->strength += 1; amount++; }
            announce(verbose, self, "rallies the pack ........", amount);
            break;

        default:
            amount = attack_hero(self, hero);
            announce(verbose, self, "does something ..........", amount);
            break;
    }

    return amount;
}
