#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include "bootCircle.hpp"
#include "bootThrow.hpp"

namespace Eend = Eendgine;

class Item {
    public:
        enum class Type { Kick, Circle, Throw };
        union Traits {
                Traits() = default;
                Traits(BootCircleTraits traits) : boot_circle(traits) {};
                Traits(BootThrowTraits traits) : boot_throw(traits) {};
                BootCircleTraits boot_circle;
                BootThrowTraits boot_throw;
        };

        Item(Eend::Point position, std::filesystem::path texture, BootCircleTraits traits);
        Item(Eend::Point position, std::filesystem::path texture, BootThrowTraits traits);
        ~Item();

        Item(const Item& other) noexcept;
        Item& operator=(const Item& other) noexcept;

        Eend::Point getPosition();

        Type getType();

        BootCircleTraits getBootCircleTraits();
        BootThrowTraits getBootThrowTraits();

    private:
        void construct();

        Eend::Point position;
        Eend::BoardId sprite;

        std::filesystem::path texture;

        Traits traits;
};
