/* ===========================================================================
   dungeon_delve.cpp

   Dungeon Delve -- encounter engine.

   Started this as a weekend thing to test the combat math and it sort of
   became the whole game. It works. Ship it.

   TODO (someday):
     - split this file up
     - the loot thing is weird sometimes, look at it
     - save files break if you rename a hero, don't rename heroes

   -- B.R.
   =========================================================================== */

#include "dungeon_delve.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Coin::Coin(int amount) : amount_(amount)
{
    if (amount < 0)
        throw std::invalid_argument("Coin amount cannot be negative");
}

int Coin::value() const
{
    return amount_;
}

Coin& Coin::operator+=(const Coin& other)
{
    if (other.amount_ > std::numeric_limits<int>::max() - amount_)
        throw std::overflow_error("Coin amount overflow");
    amount_ += other.amount_;
    return *this;
}

Coin operator+(Coin lhs, const Coin& rhs)
{
    lhs += rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& out, const Coin& coin)
{
    return out << "Coin{ " << coin.value() << " }";
}

Dice::Dice(int count, int sides) : count_(count), sides_(sides)
{
    if (count <= 0 || sides <= 0)
        throw std::invalid_argument("Dice count and sides must be positive");
    if (count > std::numeric_limits<int>::max() / sides)
        throw std::invalid_argument("Maximum dice roll exceeds the supported range");
}

int Dice::count() const
{
    return count_;
}

int Dice::sides() const
{
    return sides_;
}

int Dice::roll() const
{
    int total = 0;
    for (int i = 0; i < count_; ++i)
        total += dd_rand_range(1, sides_);
    return total;
}

Vec2::Vec2(double x, double y) : x_(x), y_(y)
{
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::invalid_argument("Vec2 coordinates must be finite");
}

double Vec2::x() const
{
    return x_;
}

double Vec2::y() const
{
    return y_;
}

Vec2& Vec2::operator+=(const Vec2& other)
{
    Vec2 result{x_ + other.x_, y_ + other.y_};
    x_ = result.x_;
    y_ = result.y_;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& other)
{
    Vec2 result{x_ - other.x_, y_ - other.y_};
    x_ = result.x_;
    y_ = result.y_;
    return *this;
}

Vec2 operator+(Vec2 lhs, const Vec2& rhs)
{
    lhs += rhs;
    return lhs;
}

Vec2 operator-(Vec2 lhs, const Vec2& rhs)
{
    lhs -= rhs;
    return lhs;
}

Damage::Damage(int amount) : amount_(amount)
{
    if (amount < 0)
        throw std::invalid_argument("Damage amount cannot be negative");
}

int Damage::value() const
{
    return amount_;
}

Damage& Damage::operator+=(const Damage& other)
{
    if (other.amount_ > std::numeric_limits<int>::max() - amount_)
        throw std::overflow_error("Damage amount overflow");
    amount_ += other.amount_;
    return *this;
}

Damage operator+(Damage lhs, const Damage& rhs)
{
    lhs += rhs;
    return lhs;
}

Purse::Purse(const Coin& initial_total) : total_(initial_total)
{
}

const Coin& Purse::total() const
{
    return total_;
}

void Purse::deposit(const Coin& amount)
{
    total_ += amount;
}

bool Purse::spend(const Coin& amount)
{
    if (amount.value() > total_.value())
        return false;
    total_ = Coin{total_.value() - amount.value()};
    return true;
}

int Combatant::take_damage(int incoming)
{
    const int dealt = apply_armor(incoming, armor);
    hp -= dealt;
    if (hp < 0)
        hp = 0;
    return dealt;
}

int Monster::take_damage(int incoming)
{
    const int dealt = Combatant::take_damage(incoming);
    if (hp == 0)
        is_dead = 1;
    return dealt;
}

Coin Monster::loot_value() const
{
    return gold_drop;
}

Coin Merchant::loot_value() const
{
    return gold;
}

void Hero::describe(std::ostream& out) const
{
    out << "  " << std::left << std::setw(11) << "[Hero]" << std::setw(13) << name
        << std::right << std::setw(3) << hp << "/" << max_hp
        << "  HP   combatant";
}

void Monster::describe(std::ostream& out) const
{
    out << "  " << std::left << std::setw(11) << "[Monster]" << std::setw(13) << name
        << std::right << std::setw(3) << hp << "/" << max_hp
        << "  HP   combatant, lootable";
}

void Merchant::describe(std::ostream& out) const
{
    out << "  " << std::left << std::setw(11) << "[Merchant]" << std::setw(13) << name
        << " --          lootable";
}

void Roster::add(const Entity& entity)
{
    entities_.push_back(&entity);
}

void Roster::describe_all() const
{
    for (const Entity* entity : entities_) {
        entity->describe(std::cout);
        std::cout << '\n';
    }
}

/* ---------------------------------------------------------------------------
   RNG. Rolled our own so the numbers are the same on every machine, which
   matters because the designers balance encounters against specific seeds.
   --------------------------------------------------------------------------- */

static unsigned g_seed = 1337u;

void dd_seed(unsigned s)
{
    g_seed = s;
}

int dd_rand()
{
    g_seed = g_seed * 1103515245u + 12345u;
    return (int)((g_seed >> 16) & 0x7FFF);
}

/* NOTE: this is inclusive on both ends. */
int dd_rand_range(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + dd_rand() % (hi - lo + 1);
}

int roll_dice(int count, int sides)
{
    return Dice{count, sides}.roll();
}

/* ---------------------------------------------------------------------------
   The loot table.

   This is a plain C array on purpose -- the content team edits it by hand in
   a text editor and they do not want it moved into a database or a map or
   anything clever. Leave it alone.
   --------------------------------------------------------------------------- */

#define LOOT_COUNT 12

static const char* LOOT_TABLE[LOOT_COUNT] = {
    "Rusted Dagger",
    "Cracked Buckler",
    "Mouldy Rations",
    "Copper Ring",
    "Tin Whistle",
    "Bent Lockpick",
    "Waterlogged Scroll",
    "Chipped Whetstone",
    "Iron Spur",
    "Glass Eye",
    "Knotted Rope",
    "Tarnished Locket"
};

static int LOOT_VALUE[LOOT_COUNT] = {
    4, 6, 1, 12, 3, 8, 15, 5, 7, 2, 3, 20
};

/* ---------------------------------------------------------------------------
   Allocation helpers.
   --------------------------------------------------------------------------- */

HeroPtr make_hero(const char* name, int hp, int strength, int armor)
{
    auto h = std::make_unique<Hero>();
    std::strncpy(h->name, name, 31);
    h->name[31] = '\0';
    h->hp       = hp;
    h->max_hp   = hp;
    h->strength = strength;
    h->armor    = armor;
    h->attack_dice = 3;
    h->xp       = 0;
    h->level    = 1;
    h->potions  = 2;
    h->abilities.push_back(ABIL_CLEAVE.clone());
    h->abilities.push_back(ABIL_MEND.clone());
    return h;
}

MonsterPtr make_monster(const char* name, const char* species,
                        int hp, int strength, int armor,
                        int gold_drop, const Ability& ability)
{
    auto m = std::make_unique<Monster>();
    std::strncpy(m->name, name, 31);
    m->name[31] = '\0';
    std::strncpy(m->species, species, 23);
    m->species[23] = '\0';
    m->hp            = hp;
    m->max_hp        = hp;
    m->strength      = strength;
    m->armor         = armor;
    m->gold_drop     = Coin{gold_drop};
    m->ability       = ability.clone();
    m->is_dead       = 0;
    m->summon_budget = ability.initial_uses();
    return m;
}

MerchantPtr make_merchant(const char* name, int gold, int markup)
{
    auto mer = std::make_unique<Merchant>();
    std::strncpy(mer->name, name, 31);
    mer->name[31]       = '\0';
    mer->gold           = Coin{gold};
    mer->markup_percent = markup;
    mer->stock_count    = 0;
    for (int i = 0; i < 8; i++)
        mer->stock_tags[i] = -1;
    return mer;
}

/* ---------------------------------------------------------------------------
   Loot.
   --------------------------------------------------------------------------- */

/* Used by the display code and by the content team's export script. */
const char* loot_at(int i)
{
    if (i < 0 || i >= LOOT_COUNT) return nullptr;
    return LOOT_TABLE[i];
}

int loot_count()
{
    return LOOT_COUNT;
}

const char* roll_loot()
{
    int idx = dd_rand_range(0, LOOT_COUNT - 1);
    return LOOT_TABLE[idx];
}

int loot_value_of(const char* loot_name)
{
    for (int i = 0; i < LOOT_COUNT; i++) {
        if (std::strcmp(LOOT_TABLE[i], loot_name) == 0)
            return LOOT_VALUE[i];
    }
    return 0;
}

/* ---------------------------------------------------------------------------
   Damage.
   --------------------------------------------------------------------------- */

Damage EncounterMath::resolve(const Dice& dice, int strength)
{
    if (strength < 0)
        throw std::invalid_argument("Strength cannot be negative");

    const int roll = dice.roll();
    if (strength > std::numeric_limits<int>::max() - roll)
        throw std::overflow_error("Raw damage overflow");
    return Damage{strength + roll};
}

Damage EncounterMath::resolve(const Damage& incoming, int armor)
{
    if (armor < 0)
        throw std::invalid_argument("Armor cannot be negative");

    const int reduced = incoming.value() - armor;
    return Damage{reduced < 1 ? 1 : reduced};
}

void EncounterMath::resolve(const Damage& incoming, int armor, Damage& result)
{
    result = resolve(incoming, armor);
}

Damage EncounterMath::resolve(const Hero& attacker, Monster& target)
{
    const Damage raw = resolve(Dice{3, 6}, attacker.strength);
    return Damage{target.take_damage(raw.value())};
}

Damage EncounterMath::resolve(Monster& attacker, Hero& target)
{
    const Damage raw = resolve(Dice{2, 6}, attacker.strength);
    return Damage{target.take_damage(raw.value())};
}

int compute_raw_damage(int strength, int dice_count, int dice_sides)
{
    return EncounterMath::resolve(Dice{dice_count, dice_sides}, strength).value();
}

int apply_armor(int raw_damage, int armor)
{
    return EncounterMath::resolve(Damage{raw_damage}, armor).value();
}

int apply_damage_to_monster(Monster& m, int incoming)
{
    return m.take_damage(incoming);
}

int apply_damage_to_hero(Hero& h, int incoming)
{
    return h.take_damage(incoming);
}

/* Hero swings at a monster. Returns the damage that landed. */
int resolve_attack_on_monster(Hero& attacker, Monster& target)
{
    return EncounterMath::resolve(attacker, target).value();
}

/* Monster swings at the hero. Returns the damage that landed. */
int resolve_attack_on_hero(Monster& attacker, Hero& target)
{
    return EncounterMath::resolve(attacker, target).value();
}

/* ---------------------------------------------------------------------------
   Polymorphic abilities.
   --------------------------------------------------------------------------- */

namespace {

class AttackAbility : public Ability {
public:
    AbilityResult apply(AbilityContext& context) const override
    {
        return {deal_damage(context, roll_raw_attack(context)), "", "damage"};
    }

protected:
    static int roll_raw_attack(const AbilityContext& context)
    {
        if (context.actor.strength < 0)
            throw std::invalid_argument("Strength cannot be negative");

        const int roll = roll_dice(context.actor.attack_dice, 6);
        if (context.actor.strength > std::numeric_limits<int>::max() - roll)
            throw std::overflow_error("Raw damage overflow");
        return context.actor.strength + roll;
    }

    static int deal_damage(AbilityContext& context, int incoming)
    {
        return context.target.take_damage(incoming);
    }
};

class Flail final : public AttackAbility {
public:
    const char* describe() const override { return "Flail"; }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Flail>(*this);
    }
};

class Cleave final : public AttackAbility {
public:
    const char* describe() const override { return "Cleave"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        const int raw = roll_raw_attack(context);
        if (raw > std::numeric_limits<int>::max() - raw / 2)
            throw std::overflow_error("Cleave damage overflow");
        return {deal_damage(context, raw + raw / 2), "", "damage"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Cleave>(*this);
    }
};

class Bite final : public AttackAbility {
public:
    const char* describe() const override { return "Bite"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        int amount = AttackAbility::apply(context).amount;
        if (dd_rand_range(1, 100) > 70) {
            if (amount > std::numeric_limits<int>::max() - 2)
                throw std::overflow_error("Bite damage overflow");
            amount += deal_damage(context, 2);
        }
        return {amount, "", "damage"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Bite>(*this);
    }
};

class Mend final : public Ability {
public:
    const char* describe() const override { return "Mend"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        const int rolled = roll_dice(3, 4);
        const int room = context.actor.max_hp > context.actor.hp
                       ? context.actor.max_hp - context.actor.hp : 0;
        const int healed = rolled < room ? rolled : room;
        context.actor.hp += healed;
        return {healed, "+", "HP"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Mend>(*this);
    }
};

class Hex final : public Ability {
public:
    const char* describe() const override { return "Hex"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        const int before = context.target.strength;
        if (before < 1)
            throw std::invalid_argument("Hex target strength must be positive");
        const int reduction = roll_dice(1, 4);
        context.target.strength = before > reduction ? before - reduction : 1;
        return {before - context.target.strength, "-", "STR"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Hex>(*this);
    }
};

class Summon final : public AttackAbility {
public:
    const char* describe() const override { return "Summon"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        if (context.summon_budget != nullptr && *context.summon_budget > 0) {
            MonsterPtr whelp = make_monster(
                "Whelp", "whelp", 6, 2, 0, 3, ABIL_NONE);
            context.wave.push_back(std::move(whelp));
            --*context.summon_budget;
            return {1, "+", "Whelp summoned"};
        }
        return AttackAbility::apply(context);
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Summon>(*this);
    }
    int initial_uses() const override { return 1; }
};

class Guard final : public Ability {
public:
    const char* describe() const override { return "Guard"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        if (context.actor.armor > std::numeric_limits<int>::max() - 2)
            throw std::overflow_error("Armor overflow");
        context.actor.armor += 2;
        return {2, "+", "armor"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Guard>(*this);
    }
};

class Rally final : public Ability {
public:
    const char* describe() const override { return "Rally"; }
    AbilityResult apply(AbilityContext& context) const override
    {
        for (const MonsterPtr& monster : context.wave) {
            if (monster && !monster->is_dead &&
                monster.get() != context.monster_actor &&
                monster->strength == std::numeric_limits<int>::max())
                throw std::overflow_error("Strength overflow during Rally");
        }

        int affected = 0;
        for (const MonsterPtr& monster : context.wave) {
            if (monster && !monster->is_dead &&
                monster.get() != context.monster_actor) {
                ++monster->strength;
                ++affected;
            }
        }
        return {affected, "+", "STR each"};
    }
    std::unique_ptr<Ability> clone() const override
    {
        return std::make_unique<Rally>(*this);
    }
};

const Flail flail_prototype;
const Cleave cleave_prototype;
const Bite bite_prototype;
const Mend mend_prototype;
const Hex hex_prototype;
const Summon summon_prototype;
const Guard guard_prototype;
const Rally rally_prototype;

}  // namespace

const Ability& ABIL_NONE = flail_prototype;
const Ability& ABIL_CLEAVE = cleave_prototype;
const Ability& ABIL_BITE = bite_prototype;
const Ability& ABIL_MEND = mend_prototype;
const Ability& ABIL_HEX = hex_prototype;
const Ability& ABIL_SUMMON = summon_prototype;
const Ability& ABIL_GUARD = guard_prototype;
const Ability& ABIL_RALLY = rally_prototype;

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

/* ---------------------------------------------------------------------------
   Bestiary lookup.
   --------------------------------------------------------------------------- */

/* Returns a non-owning pointer; the Bestiary retains ownership. */
Monster* find_in_bestiary(Bestiary& bestiary, const char* species)
{
    for (size_t i = 0; i < bestiary.size(); i++) {
        if (std::strcmp(bestiary[i]->species, species) == 0)
            return raw(bestiary[i]); /* non-owning; bestiary retains ownership */
    }
    return nullptr;
}

int count_species_in_bestiary(Bestiary& bestiary, const char* species)
{
    int n = 0;
    for (size_t i = 0; i < bestiary.size(); i++) {
        if (std::strcmp(bestiary[i]->species, species) == 0)
            n++;
    }
    return n;
}

Bestiary build_bestiary()
{
    Bestiary b;
    b.push_back(make_monster("Kobold",      "kobold",    12,  4, 1,  6, ABIL_BITE));
    b.push_back(make_monster("Goblin",      "goblin",    10,  3, 0,  5, ABIL_NONE));
    b.push_back(make_monster("Cave Bat",    "bat",        5,  2, 0,  1, ABIL_BITE));
    b.push_back(make_monster("Hex Adept",   "adept",     14,  3, 1, 11, ABIL_HEX));
    b.push_back(make_monster("Bone Priest", "priest",    16,  4, 2, 14, ABIL_MEND));
    b.push_back(make_monster("Warg",        "warg",      18,  6, 1, 10, ABIL_CLEAVE));
    b.push_back(make_monster("Brood Mother","brood",     22,  5, 2, 18, ABIL_SUMMON));
    b.push_back(make_monster("Shield Ogre", "ogre",      26,  7, 3, 21, ABIL_GUARD));
    b.push_back(make_monster("Pack Leader", "packleader",20,  5, 1, 16, ABIL_RALLY));
    return b;
}

MonsterPtr spawn_from_bestiary(Bestiary& bestiary, const char* species)
{
    Monster* proto = find_in_bestiary(bestiary, species); /* non-owning */
    if (proto == nullptr) return MonsterPtr{};

    return make_monster(proto->name, proto->species, proto->max_hp,
                        proto->strength, proto->armor, proto->gold_drop.value(),
                        *proto->ability);
}

/* ---------------------------------------------------------------------------
   Encounter waves.
   --------------------------------------------------------------------------- */

int wave_is_cleared(Wave& wave)
{
    for (size_t i = 0; i < wave.size(); i++) {
        if (!wave[i]->is_dead) return 0;
    }
    return 1;
}

int living_in_wave(Wave& wave)
{
    int n = 0;
    for (size_t i = 0; i < wave.size(); i++) {
        if (!wave[i]->is_dead) n++;
    }
    return n;
}

/* Every living monster in the wave takes its turn against the hero.
   Returns the total damage the wave dealt this round. */
int run_monster_round(Wave& wave, Hero& hero, int verbose,
                      std::vector<std::string>* acted_names)
{
    int total = 0;

    /* Index iteration stays valid when a summon grows the wave; appended monsters
       continue to take a turn this round, matching the existing encounter flow. */
    for (size_t i = 0; i < wave.size(); ++i) {
        Monster* m = raw(wave[i]); /* non-owning; the Wave owns the monster */

        if (m->is_dead) continue;
        if (hero.hp <= 0) break;

        if (acted_names != nullptr)
            acted_names->push_back(std::string(m->name));

        if (!m->ability)
            throw std::logic_error("Monster has no ability");

        AbilityContext context{*m, hero, wave, verbose,
                               &m->summon_budget, m};
        std::vector<AbilityAction> actions;
        actions.push_back({m->ability.get(), context});
        total += resolve_actions(actions);
    }

    return total;
}

/* Hero attacks the first living monster. */
int run_hero_round(Wave& wave, Hero& hero, int verbose)
{
    for (size_t i = 0; i < wave.size(); i++) {
        if (wave[i]->is_dead) continue;

        if (hero.abilities.empty())
            throw std::logic_error("Hero has no abilities");

        const size_t selected = hero.next_ability % hero.abilities.size();
        const Ability* ability = hero.abilities[selected].get();
        hero.next_ability++;
        AbilityContext context{hero, *raw(wave[i]), wave, verbose, nullptr, nullptr};
        std::vector<AbilityAction> actions;
        actions.push_back({ability, context});
        int dealt = resolve_actions(actions);
        if (verbose && wave[i]->is_dead)
            std::printf("  %s falls.\n", wave[i]->name);
        return dealt;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
   The encounter loop.
   --------------------------------------------------------------------------- */

static int run_encounter_impl(Hero& hero, Wave& wave, int verbose)
{
    int round = 1;

    if (verbose) {
        std::printf("\n");
        std::printf("--- ENCOUNTER: %d monster(s) ---\n", living_in_wave(wave));
    }

    while (hero.hp > 0 && !wave_is_cleared(wave) && round <= 40) {

        if (verbose)
            std::printf("\n[round %d]\n", round);

        run_hero_round(wave, hero, verbose);

        if (wave_is_cleared(wave)) break;

        run_monster_round(wave, hero, verbose, nullptr);

        round++;
    }

    if (hero.hp <= 0) {
        if (verbose) std::printf("\n%s has fallen.\n", hero.name);
        return 0;
    }

    Coin gold;
    for (size_t i = 0; i < wave.size(); i++) {
        if (wave[i]->is_dead)
            gold += wave[i]->gold_drop;
    }

    hero.purse.deposit(gold);
    hero.xp += gold.value() * 2;

    const char* drop = roll_loot();

    if (verbose) {
        std::printf("\nWave cleared.\n");
        std::printf("  gold ...... %d\n", gold.value());
        std::printf("  loot ...... %s (worth %d)\n", drop, loot_value_of(drop));
        std::printf("  purse total: %d gold\n", hero.purse.total().value());
    }

    return 1;
}

int run_encounter(Hero& hero, Wave& wave, int verbose)
{
    try {
        return run_encounter_impl(hero, wave, verbose);
    } catch (const std::exception&) {
        std::throw_with_nested(std::runtime_error(
            std::string("Encounter for ") + hero.name + " failed"));
        throw;
    }
}

/* ---------------------------------------------------------------------------
   Levelling.
   --------------------------------------------------------------------------- */

int xp_for_next_level(int level)
{
    return level * 100;
}

void check_level_up(Hero& h, int verbose)
{
    while (h.xp >= xp_for_next_level(h.level)) {
        h.xp -= xp_for_next_level(h.level);
        h.level++;
        h.max_hp   += 10;
        h.hp        = h.max_hp;
        h.strength += 2;

        if (verbose)
            std::printf("\n*** %s reaches level %d ***\n", h.name, h.level);
    }
}

/* ---------------------------------------------------------------------------
   The merchant.
   --------------------------------------------------------------------------- */

int merchant_price(Merchant& mer, int base_value)
{
    return base_value + (base_value * mer.markup_percent) / 100;
}

void visit_merchant(Merchant& mer, Hero& hero, int verbose)
{
    int potion_price = merchant_price(mer, 15);

    if (verbose) {
        std::printf("\n--- %s's stall ---\n", mer.name);
        std::printf("  potion ..... %d gold (you have %d)\n",
                    potion_price, hero.purse.total().value());
    }

    const Coin price{potion_price};
    while (hero.purse.total().value() >= potion_price && hero.potions < 5) {
        if (!hero.purse.spend(price)) break;
        hero.potions++;
        mer.gold += price;
        if (verbose)
            std::printf("  bought a potion. potions=%d gold=%d\n",
                        hero.potions, hero.purse.total().value());
    }
}

void quaff_potion(Hero& h, int verbose)
{
    if (h.potions <= 0) return;
    h.potions--;
    int healed = roll_dice(3, 6);
    h.hp += healed;
    if (h.hp > h.max_hp) h.hp = h.max_hp;
    if (verbose)
        std::printf("  %s drinks a potion ....... +%d HP\n", h.name, healed);
}

/* ---------------------------------------------------------------------------
   Save and load. Plain text, one field per line, order matters.
   Renaming a hero breaks old saves. Don't rename heroes.
   --------------------------------------------------------------------------- */

int save_hero(Hero& h, const char* path)
{
    FILE* f = std::fopen(path, "w");
    if (f == nullptr) return 0;

    std::fprintf(f, "%s\n", h.name);
    std::fprintf(f, "%d\n", h.hp);
    std::fprintf(f, "%d\n", h.max_hp);
    std::fprintf(f, "%d\n", h.strength);
    std::fprintf(f, "%d\n", h.armor);
    std::fprintf(f, "%d\n", h.purse.total().value());
    std::fprintf(f, "%d\n", h.xp);
    std::fprintf(f, "%d\n", h.level);
    std::fprintf(f, "%d\n", h.potions);

    std::fclose(f);
    return 1;
}

HeroPtr load_hero(const char* path)
{
    FILE* f = std::fopen(path, "r");
    if (f == nullptr) return HeroPtr{};

    auto h = std::make_unique<Hero>();
    h->abilities.push_back(ABIL_CLEAVE.clone());
    h->abilities.push_back(ABIL_MEND.clone());
    char line[64];
    int hp = 0;
    int max_hp = 0;
    int strength = 0;
    int armor = 0;
    int gold = 0;
    int xp = 0;
    int level = 0;
    int potions = 0;

    if (std::fgets(line, 64, f) == nullptr) {
        std::fclose(f);
        return HeroPtr{};
    }
    size_t n = std::strlen(line);
    if (n > 0 && line[n - 1] == '\n') line[n - 1] = '\0';
    std::strncpy(h->name, line, 31);
    h->name[31] = '\0';

    if (std::fscanf(f, "%d", &hp) != 1
        || std::fscanf(f, "%d", &max_hp) != 1
        || std::fscanf(f, "%d", &strength) != 1
        || std::fscanf(f, "%d", &armor) != 1
        || std::fscanf(f, "%d", &gold) != 1
        || std::fscanf(f, "%d", &xp) != 1
        || std::fscanf(f, "%d", &level) != 1
        || std::fscanf(f, "%d", &potions) != 1) {
        std::fclose(f);
        return HeroPtr{};
    }

    std::fclose(f);

    if (gold < 0)
        return HeroPtr{};

    h->hp = hp;
    h->max_hp = max_hp;
    h->strength = strength;
    h->armor = armor;
    h->attack_dice = 3;
    h->purse.deposit(Coin{gold});
    h->xp = xp;
    h->level = level;
    h->potions = potions;
    return h;
}

/* ---------------------------------------------------------------------------
   Display. Printing is done wherever it is convenient.
   --------------------------------------------------------------------------- */

void print_banner()
{
    for (int i = 0; i < 64; i++) std::printf("=");
    std::printf("\n");
    std::printf("  D U N G E O N   D E L V E\n");
    for (int i = 0; i < 64; i++) std::printf("=");
    std::printf("\n");
}

void print_hero(Hero& h)
{
    std::printf("\n%s  --  level %d\n", h.name, h.level);
    std::printf("  HP ......... %d / %d\n", h.hp, h.max_hp);
    std::printf("  STR ........ %d\n", h.strength);
    std::printf("  ARMOR ...... %d\n", h.armor);
    std::printf("  GOLD ....... %d\n", h.purse.total().value());
    std::printf("  XP ......... %d / %d\n", h.xp, xp_for_next_level(h.level));
    std::printf("  POTIONS .... %d\n", h.potions);
}

void print_wave(Wave& wave)
{
    std::printf("\nWave:\n");
    for (size_t i = 0; i < wave.size(); i++) {
        std::printf("  %-14s %-11s %3d/%-3d HP  str %-2d  armor %-2d  %s\n",
                    wave[i]->name, wave[i]->species,
                    wave[i]->hp, wave[i]->max_hp,
                    wave[i]->strength, wave[i]->armor,
                    wave[i]->ability->describe());
    }
}

void print_bestiary(Bestiary& bestiary)
{
    std::printf("\nBestiary (%d entries):\n", (int)bestiary.size());
    for (size_t i = 0; i < bestiary.size(); i++) {
        std::printf("  %-14s %-11s %3d HP  str %-2d  armor %-2d  gold %-3d  %s\n",
                    bestiary[i]->name, bestiary[i]->species,
                    bestiary[i]->max_hp, bestiary[i]->strength,
                    bestiary[i]->armor, bestiary[i]->gold_drop.value(),
                    bestiary[i]->ability->describe());
    }
}

void print_loot_table()
{
    std::printf("\nLoot table (%d entries):\n", LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT; i++)
        std::printf("  %-22s %3d gold\n", LOOT_TABLE[i], LOOT_VALUE[i]);
}

/* ---------------------------------------------------------------------------
   The campaign.
   --------------------------------------------------------------------------- */

Wave build_wave(Bestiary& bestiary, const char* a, const char* b, const char* c)
{
    Wave wave;

    if (a != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, a);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }
    if (b != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, b);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }
    if (c != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, c);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }

    return wave;
}

static void print_ability_demo()
{
    struct SeedRestore {
        unsigned value;
        ~SeedRestore() { g_seed = value; }
    } restore{g_seed};

    HeroPtr ash = make_hero("Ash", 100, 1, 3);
    ash->hp = 90;

    Wave wave;
    wave.push_back(make_monster("Kobold", "kobold", 100, 4, 1, 0, ABIL_BITE));

    std::printf("encounter(ash, kobold)\n");
    run_hero_round(wave, *ash, 1);
    run_monster_round(wave, *ash, 1, nullptr);
    run_hero_round(wave, *ash, 1);
    std::printf("\nresolution loop: 4 lines, 0 type checks, 0 casts\n\n");
}

int run_campaign(int verbose)
{
    if (verbose)
        print_ability_demo();

    print_banner();

    Bestiary    bestiary = build_bestiary();
    HeroPtr     hero_owner = make_hero("Ash", 100, 10, 3);
    MerchantPtr yarrow_owner = make_merchant("Old Yarrow", 400, 35);

    Hero&     hero = *hero_owner;          /* non-owning alias for readability */
    Merchant& yarrow = *yarrow_owner;      /* non-owning alias for readability */

    if (verbose) {
        Roster roster;
        roster.add(hero);
        roster.add(*find_in_bestiary(bestiary, "kobold"));
        roster.add(yarrow);
        std::printf("\nRoster:\n");
        roster.describe_all();
        print_hero(hero);
        print_bestiary(bestiary);
        print_loot_table();
    }

    const char* script[6][3] = {
        { "bat",        "bat",     nullptr      },
        { "goblin",     "kobold",  nullptr      },
        { "adept",      "goblin",  "bat"        },
        { "warg",       "kobold",  nullptr      },
        { "brood",      "goblin",  "bat"        },
        { "ogre",       "priest",  "packleader" }
    };

    int cleared = 0;

    for (int stage = 0; stage < 6; stage++) {

        Wave wave = build_wave(bestiary,
                               script[stage][0],
                               script[stage][1],
                               script[stage][2]);

        if (verbose) {
            std::printf("\n");
            for (int i = 0; i < 64; i++) std::printf("-");
            std::printf("\nSTAGE %d\n", stage + 1);
            print_wave(wave);
        }

        if (hero.hp < hero.max_hp / 3)
            quaff_potion(hero, verbose);

        int survived = run_encounter(hero, wave, verbose);

        if (!survived) {
            if (verbose) std::printf("\nRun over at stage %d.\n", stage + 1);
            return cleared;
        }

        cleared++;
        check_level_up(hero, verbose);

        if (stage == 2)
            visit_merchant(yarrow, hero, verbose);

        if (verbose) print_hero(hero);
    }

    if (verbose) {
        std::printf("\n");
        for (int i = 0; i < 64; i++) std::printf("=");
        std::printf("\n  %s clears the delve with %d gold.\n",
                    hero.name, hero.purse.total().value());
        for (int i = 0; i < 64; i++) std::printf("=");
        std::printf("\n");
    }

    save_hero(hero, "ash.save");

    return cleared;
}

/* ---------------------------------------------------------------------------
   main
   --------------------------------------------------------------------------- */

#ifndef DD_NO_MAIN
static void print_exception_chain(const std::exception& error)
{
    std::cerr << error.what();
    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception& nested) {
        std::cerr << "\ncaused by: ";
        print_exception_chain(nested);
    } catch (...) {
        std::cerr << "\ncaused by a non-standard exception";
    }
}

int main(int argc, char** argv)
{
    unsigned seed = 1337u;
    int verbose = 1;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--quiet") == 0) {
            verbose = 0;
        } else if (std::strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = (unsigned)std::atoi(argv[i + 1]);
            i++;
        }
    }

    dd_seed(seed);

    try {
        int cleared = run_campaign(verbose);
        std::printf("\nstages cleared: %d / 6\n", cleared);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "fatal: ";
        print_exception_chain(error);
        std::cerr << '\n';
        return 1;
    }
}
#endif
