/* ===========================================================================
   dungeon_delve.h

   Somebody finally pulled the declarations out of the .cpp so the tests would
   stop duplicating them. That is as far as it got.

   READ THIS BEFORE BUILD 1.

   The three aliases below -- MonsterPtr, HeroPtr, MerchantPtr -- are the
   ownership model of this entire engine, written down in three lines.

   Your Build 1 conversion happens HERE first. Change an alias to a smart
   pointer and the whole engine follows, including the tests. That is the
   point of putting them in one place.

   Do not change the function signatures to match a particular alias. They
   are written in terms of references and the aliases so that they keep
   compiling whichever way you take the ownership.
   =========================================================================== */

#ifndef DUNGEON_DELVE_H
#define DUNGEON_DELVE_H

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Coin {
public:
    Coin() = default;
    explicit Coin(int amount);

    int value() const;
    Coin& operator+=(const Coin& other);

private:
    int amount_ = 0;
};

Coin operator+(Coin lhs, const Coin& rhs);
std::ostream& operator<<(std::ostream& out, const Coin& coin);

class Dice {
public:
    Dice(int count, int sides);

    int count() const;
    int sides() const;
    int roll() const;

private:
    int count_;
    int sides_;
};

class Vec2 {
public:
    Vec2(double x, double y);

    double x() const;
    double y() const;
    Vec2& operator+=(const Vec2& other);
    Vec2& operator-=(const Vec2& other);

private:
    double x_;
    double y_;
};

Vec2 operator+(Vec2 lhs, const Vec2& rhs);
Vec2 operator-(Vec2 lhs, const Vec2& rhs);

class Damage {
public:
    Damage() = default;
    explicit Damage(int amount);

    int value() const;
    Damage& operator+=(const Damage& other);

private:
    int amount_ = 0;
};

Damage operator+(Damage lhs, const Damage& rhs);

class Purse {
public:
    Purse() = default;
    explicit Purse(const Coin& initial_total);

    const Coin& total() const;
    void deposit(const Coin& amount);
    bool spend(const Coin& amount);

private:
    Coin total_;
};

/* ---------------------------------------------------------------------------
   Entities. Three separate structs. They share most of their fields but a
   hero is not a monster and a monster is not a merchant, so three structs.
   --------------------------------------------------------------------------- */

struct Hero {
    char name[32];
    int  hp;
    int  max_hp;
    int  strength;
    int  armor;
    Purse purse;
    int  xp;
    int  level;
    int  potions;
};

struct Monster {
    char name[32];
    char species[24];
    int  hp;
    int  max_hp;
    int  strength;
    int  armor;
    Coin gold_drop;
    int  ability_tag;
    int  is_dead;
    int  summon_budget;
};

struct Merchant {
    char name[32];
    Coin gold;
    int  markup_percent;
    int  stock_tags[8];
    int  stock_count;
};

class EncounterMath {
public:
    // const& avoids input copies; returning this small Damage is direct and
    // normally elided. The out-parameter saves no meaningful cost here and
    // requires a mutable result object, but supports callers needing that API.
    static Damage resolve(const Dice& dice, int strength);
    static Damage resolve(const Damage& incoming, int armor);
    static void resolve(const Damage& incoming, int armor, Damage& result);
    static Damage resolve(const Hero& attacker, Monster& target);
    static Damage resolve(Monster& attacker, Hero& target);

private:
    // Static-only utility: all operations are stateless, so instances have no purpose.
    EncounterMath() = delete;
};

/* ---------------------------------------------------------------------------
   THE OWNERSHIP MODEL.

   These aliases express unique ownership throughout the engine.
   --------------------------------------------------------------------------- */

using MonsterPtr  = std::unique_ptr<Monster>;
using HeroPtr     = std::unique_ptr<Hero>;
using MerchantPtr = std::unique_ptr<Merchant>;

using Wave     = std::vector<MonsterPtr>;
using Bestiary = std::vector<MonsterPtr>;

/* Produces a non-owning Monster* for observation without transferring ownership. */
inline Monster* raw(Monster* p)                            { return p; }
inline Monster* raw(const std::unique_ptr<Monster>& p)     { return p.get(); }

/* ---------------------------------------------------------------------------
   Ability tags.

   Every ability in the game is an int. resolve_ability() switches on it.
   Adding an ability means adding a tag here and a case in every switch that
   looks at tags, and there are a few of those now.
   --------------------------------------------------------------------------- */

#define ABIL_NONE    0
#define ABIL_CLEAVE  1
#define ABIL_BITE    2
#define ABIL_MEND    3
#define ABIL_HEX     4
#define ABIL_SUMMON  5
#define ABIL_GUARD   6
#define ABIL_RALLY   7

/* ---------------------------------------------------------------------------
   The engine.
   --------------------------------------------------------------------------- */

/* Entity references in engine APIs are borrowed; ownership stays with the
   caller or the owning Wave/Bestiary. */

/* RNG -- deterministic for a given seed, so a bug you can reproduce once you
   can reproduce every time. */
void dd_seed(unsigned s);
int  dd_rand();
int  dd_rand_range(int lo, int hi);      /* inclusive on both ends */
int  roll_dice(int count, int sides);

/* Loot. The table is a raw C array; the content team edits it by hand. */
const char* loot_at(int i);
int         loot_count();
const char* roll_loot();
int         loot_value_of(const char* loot_name);

/* Construction. */
HeroPtr     make_hero(const char* name, int hp, int strength, int armor);
MonsterPtr  make_monster(const char* name, const char* species, int hp,
                         int strength, int armor, int gold_drop, int ability_tag);
MerchantPtr make_merchant(const char* name, int gold, int markup);

/* Damage. */
int compute_raw_damage(int strength, int dice_count, int dice_sides);
int apply_armor(int raw_damage, int armor);
int apply_damage_to_monster(Monster& m, int incoming);
int apply_damage_to_hero(Hero& h, int incoming);
int resolve_attack_on_monster(Hero& attacker, Monster& target);
int resolve_attack_on_hero(Monster& attacker, Hero& target);

/* Abilities. */
const char* ability_name(int tag);
/* self and hero are non-owning references; monsters remain owned by wave. */
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave, int verbose);

/* Bestiary. */
Bestiary   build_bestiary();
Monster*   find_in_bestiary(Bestiary& bestiary, const char* species); /* non-owning */
int        count_species_in_bestiary(Bestiary& bestiary, const char* species);
MonsterPtr spawn_from_bestiary(Bestiary& bestiary, const char* species);

/* Encounters. */
int  wave_is_cleared(Wave& wave);
int  living_in_wave(Wave& wave);
/* acted_names is borrowed when non-null; run_monster_round does not own it. */
int  run_monster_round(Wave& wave, Hero& hero, int verbose,
                       std::vector<std::string>* acted_names);
int  run_hero_round(Wave& wave, Hero& hero, int verbose);
int  run_encounter(Hero& hero, Wave& wave, int verbose);
Wave build_wave(Bestiary& bestiary, const char* a, const char* b, const char* c);

/* Progression and the merchant. */
int  xp_for_next_level(int level);
void check_level_up(Hero& h, int verbose);
int  merchant_price(Merchant& mer, int base_value);
void visit_merchant(Merchant& mer, Hero& hero, int verbose);
void quaff_potion(Hero& h, int verbose);

/* Save and load. Plain text, one field per line, order matters. */
int     save_hero(Hero& h, const char* path);
HeroPtr load_hero(const char* path);

/* Display. Printing is done wherever it is convenient. */
void print_banner();
void print_hero(Hero& h);
void print_wave(Wave& wave);
void print_bestiary(Bestiary& bestiary);
void print_loot_table();

/* The campaign. */
int run_campaign(int verbose);

#endif /* DUNGEON_DELVE_H */
