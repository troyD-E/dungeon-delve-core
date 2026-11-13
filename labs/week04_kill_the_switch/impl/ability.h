#ifndef DD_LAB04_ABILITY_H
#define DD_LAB04_ABILITY_H

#include "kill_the_switch.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace lab04 {

class Ability {
public:
    Ability(int tag, const char* name, int mana_cost, bool hostile)
        : tag_(tag), name_(name), mana_cost_(mana_cost), hostile_(hostile) {}
    virtual ~Ability() = default;

    int tag() const { return tag_; }
    const char* name() const { return name_; }
    int mana_cost() const { return mana_cost_; }
    bool hostile() const { return hostile_; }

    int resolve(Monster& self, Hero& hero, Wave& wave, int verbose) const
    {
        if (self.mana < mana_cost_) {
            if (verbose)
                std::printf("  %s tries to %s and fizzles\n", self.name, name_);
            return 0;
        }
        self.mana -= mana_cost_;
        return apply(self, hero, wave, verbose);
    }

    virtual int apply(Monster& self, Hero& hero, Wave& wave,
                      int verbose) const = 0;

private:
    int tag_;
    const char* name_;
    int mana_cost_;
    bool hostile_;
};

inline std::vector<const Ability*>& ability_registry()
{
    static std::vector<const Ability*> abilities;
    return abilities;
}

class AbilityRegistration {
public:
    explicit AbilityRegistration(const Ability& ability)
    {
        ability_registry().push_back(&ability);
    }
};

inline const std::vector<const Ability*>& registered_abilities()
{
    static const std::vector<const Ability*> ordered = [] {
        std::vector<const Ability*> abilities = ability_registry();
        std::sort(abilities.begin(), abilities.end(),
                  [](const Ability* left, const Ability* right) {
                      return left->tag() < right->tag();
                  });
        return abilities;
    }();
    return ordered;
}

inline void announce(int verbose, const Monster& self,
                    const char* phrase, int amount)
{
    if (verbose)
        std::printf("  %s %s %d\n", self.name, phrase, amount);
}

}  // namespace lab04

#endif
