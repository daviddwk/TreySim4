#pragma once

#include <queue>

#include <Eendgine/frameLimiter.hpp>

#include "collision.hpp"
#include "dog.hpp"
#include "items.hpp"

class BootThrowTraits {
    public:
        BootThrowTraits(
            float boot_radius, float throw_distance, float throw_frequency, float throw_speed);

        BootThrowTraits& operator+=(const BootThrowTraits& rhs);
        BootThrowTraits& operator-=(const BootThrowTraits& rhs);

        float boot_radius;
        float throw_distance;
        float throw_frequency;
        float throw_speed;
};

class BootThrowItem {
    public:
        BootThrowItem(
            float boot_radius, float throw_distance, float throw_frequency, float throw_speed);
        BootThrowTraits traits;

    private:
        // item Item;
};

class BootThrow {
    public:
        BootThrow();
        ~BootThrow();
        BootThrow(const BootThrow&) = delete;
        BootThrow& operator=(const BootThrow&) = delete;

        void update(Eend::Point trey_position, Eend::Vector2D trey_direction);
        void hit(Eend::Point trey_position, Dog& dog);

        bool enabled();
        void addTraits(BootThrowTraits new_traits);
        void removeTraits();

    private:
        std::optional<Eend::BoardId> boot_sprite = std::nullopt;
        Eend::Point boot_position;

        Eend::Vector throw_vector;

        BootThrowTraits traits;
        std::queue<BootThrowTraits> item_traits;

        float last_throw;
};
