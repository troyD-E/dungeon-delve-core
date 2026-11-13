#include "ability.h"

namespace lab04 {
namespace {

class UnknownAbility final : public Ability {
public:
    UnknownAbility() : Ability(-1, "???", 0, false) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = attack_hero(self, hero);
        announce(verbose, self, "does something ..........", amount);
        return amount;
    }
};

const Ability& find_ability(int tag)
{
    for (const Ability* ability : registered_abilities())
        if (ability->tag() == tag) return *ability;

    static const UnknownAbility unknown;
    return unknown;
}

}  // namespace
}  // namespace lab04

const char* ability_name(int tag)
{
    return lab04::find_ability(tag).name();
}

int ability_mana_cost(int tag)
{
    return lab04::find_ability(tag).mana_cost();
}

bool ability_is_hostile(int tag)
{
    return lab04::find_ability(tag).hostile();
}

int ability_count()
{
    return static_cast<int>(lab04::registered_abilities().size());
}

int ability_tag_at(int index)
{
    const auto& abilities = lab04::registered_abilities();
    if (index < 0 || index >= static_cast<int>(abilities.size())) return -1;
    return abilities[static_cast<std::size_t>(index)]->tag();
}

int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave, int verbose)
{
    return lab04::find_ability(tag).resolve(self, hero, wave, verbose);
}
