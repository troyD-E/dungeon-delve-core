#include "ability.h"

namespace lab04 {
namespace {

class Flail final : public Ability {
public:
    Flail() : Ability(ABIL_NONE, "Flail", 0, true) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = attack_hero(self, hero);
        announce(verbose, self, "flails ..................", amount);
        return amount;
    }
};

class Cleave final : public Ability {
public:
    Cleave() : Ability(ABIL_CLEAVE, "Cleave", 3, true) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int first = attack_hero(self, hero);
        const int follow = (first / 2 > 1) ? first / 2 : 1;
        hero.hp -= follow;
        if (hero.hp < 0) hero.hp = 0;
        const int amount = first + follow;
        announce(verbose, self, "cleaves .................", amount);
        return amount;
    }
};

class Bite final : public Ability {
public:
    Bite() : Ability(ABIL_BITE, "Bite", 2, true) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int first = attack_hero(self, hero);
        const int roll = lab_rand_range(1, 100);
        int amount = first;
        if (roll > 70) {
            hero.hp -= 2;
            if (hero.hp < 0) hero.hp = 0;
            amount += 2;
        }
        announce(verbose, self, "bites ...................", amount);
        return amount;
    }
};

class Mend final : public Ability {
public:
    Mend() : Ability(ABIL_MEND, "Mend", 4, false) {}

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        const int rolled = roll_dice(2, 4);
        const int room = (self.max_hp > self.hp) ? self.max_hp - self.hp : 0;
        const int amount = (rolled < room) ? rolled : room;
        self.hp += amount;
        announce(verbose, self, "mends ...................", amount);
        return amount;
    }
};

class Hex final : public Ability {
public:
    Hex() : Ability(ABIL_HEX, "Hex", 5, true) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int before = hero.strength;
        hero.strength -= roll_dice(1, 4);
        if (hero.strength < 1) hero.strength = 1;
        const int amount = before - hero.strength;
        announce(verbose, self, "hexes ...................", amount);
        return amount;
    }
};

class Summon final : public Ability {
public:
    Summon() : Ability(ABIL_SUMMON, "Summon", 8, false) {}

    int apply(Monster& self, Hero& hero, Wave& wave, int verbose) const override
    {
        int amount;
        const char* phrase;
        if (self.summon_budget > 0) {
            self.summon_budget--;
            spawn_into_wave(wave, "Whelp", 6, 2, 0);
            amount = 1;
            phrase = "summons a Whelp .........";
        } else {
            amount = attack_hero(self, hero);
            phrase = "is out of whelps ........";
        }
        announce(verbose, self, phrase, amount);
        return amount;
    }
};

class Guard final : public Ability {
public:
    Guard() : Ability(ABIL_GUARD, "Guard", 2, false) {}

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        const int before = self.armor;
        self.armor = (self.armor + 2 > 8) ? 8 : self.armor + 2;
        const int amount = self.armor - before;
        announce(verbose, self, "guards ..................", amount);
        return amount;
    }
};

class Rally final : public Ability {
public:
    Rally() : Ability(ABIL_RALLY, "Rally", 6, false) {}

    int apply(Monster& self, Hero&, Wave& wave, int verbose) const override
    {
        int amount = 0;
        for (const auto& monster : wave) {
            if (monster && !monster->is_dead && monster.get() != &self) {
                monster->strength += 1;
                amount++;
            }
        }
        announce(verbose, self, "rallies the pack ........", amount);
        return amount;
    }
};

const Flail flail;
const Cleave cleave;
const Bite bite;
const Mend mend;
const Hex hex;
const Summon summon;
const Guard guard;
const Rally rally;

const AbilityRegistration register_flail{flail};
const AbilityRegistration register_cleave{cleave};
const AbilityRegistration register_bite{bite};
const AbilityRegistration register_mend{mend};
const AbilityRegistration register_hex{hex};
const AbilityRegistration register_summon{summon};
const AbilityRegistration register_guard{guard};
const AbilityRegistration register_rally{rally};

}  // namespace
}  // namespace lab04
