#include "ability.h"

namespace lab04 {
namespace {

class Shatter final : public Ability {
public:
    Shatter() : Ability(ABIL_SHATTER, "Shatter", 7, true) {}

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int damage = self.strength + lab_rand_range(1, 6);
        hero.hp -= damage;
        if (hero.hp < 0) hero.hp = 0;
        hero.armor = (hero.armor > 0) ? hero.armor - 1 : 0;
        announce(verbose, self, "shatters .................", damage);
        return damage;
    }
};

const Shatter shatter;
const AbilityRegistration register_shatter{shatter};

}  // namespace
}  // namespace lab04
