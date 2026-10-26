#pragma once

#include <queue>

#include <Eendgine/frameLimiter.hpp>

#include "collision.hpp"
#include "dog.hpp"
#include "items.hpp"

const float THROW_DEFAULT_BOOT_RADIUS = 2.5f;
const float THROW_DEFAULT_THROW_DISTANCE = 25.0f;
const float THROW_DEFAULT_FREQUENCY = 4.0f;
const float THROW_DEFAULT_SPEED = 1.0f;

const Eend::Point BOOT_THROW_SPRITE_OFFSET = Eend::Point(0.0f, 0.0f, 0.0f);
const std::filesystem::path BOOT_THROW_SPRITE_PATH = std::filesystem::path("resources/boot");

class BootThrowTraits {
    public:
        BootThrowTraits(
            float boot_radius, float throw_distance, float throw_frequency, float throw_speed)
            : boot_radius(boot_radius), throw_distance(throw_distance),
              throw_frequency(throw_frequency), throw_speed(throw_speed) {};

        BootThrowTraits& operator+=(const BootThrowTraits& rhs) {
            BootThrowTraits::boot_radius += rhs.boot_radius;
            BootThrowTraits::throw_distance += rhs.throw_distance;
            BootThrowTraits::throw_frequency += rhs.throw_frequency;
            BootThrowTraits::throw_speed += rhs.throw_speed;
            return *this;
        }
        BootThrowTraits& operator-=(const BootThrowTraits& rhs) {
            BootThrowTraits::boot_radius -= rhs.boot_radius;
            BootThrowTraits::throw_distance -= rhs.throw_distance;
            BootThrowTraits::throw_distance -= rhs.throw_distance;
            BootThrowTraits::throw_speed -= rhs.throw_speed;
            return *this;
        }

        float boot_radius;
        float throw_distance;
        float throw_frequency;
        float throw_speed;
};

class BootThrowItem {
    public:
        BootThrowItem(
            float boot_radius, float throw_distance, float throw_frequency, float throw_speed)
            : traits(boot_radius, throw_distance, throw_frequency, throw_speed) {};
        BootThrowTraits traits;

    private:
        // item Item;
};

class BootThrow {
    public:
        BootThrow()
            : traits(
                  THROW_DEFAULT_BOOT_RADIUS, THROW_DEFAULT_THROW_DISTANCE, THROW_DEFAULT_FREQUENCY,
                  THROW_DEFAULT_SPEED),
              last_throw(0.0f) {
            item_traits.emplace(0.0f, 0.0f, 0.0f, 0.0f); // DEBUG
        };
        ~BootThrow() {};
        BootThrow(const BootThrow&) = delete;
        BootThrow& operator=(const BootThrow&) = delete;

        void update(Eend::Point trey_position, Eend::Vector2D trey_direction) {
            if (enabled()) {
                float dt = Eend::FrameLimiter::get().deltaTime;
                last_throw += dt;
                if (!boot_sprite && (last_throw >= traits.throw_frequency)) { // new throw
                    last_throw = 0.0f;
                    BootThrow::boot_sprite =
                        Eend::Entities::boards().insert(BOOT_THROW_SPRITE_PATH);
                    throw_vector = Eend::Vector(trey_direction.x, trey_direction.y, 0.0f);
                    boot_position = trey_position;
                    Eend::Entities::boards()
                        .getRef(*boot_sprite)
                        ->setScale(Eend::Scale2D(THROW_DEFAULT_BOOT_RADIUS * 2.0f));
                    Eend::Entities::boards()
                        .getRef(*boot_sprite)
                        ->setPosition(boot_position + BOOT_THROW_SPRITE_OFFSET);
                } else if (boot_sprite) { // continue throw
                    boot_position += throw_vector * THROW_DEFAULT_SPEED;
                    Eend::Entities::boards()
                        .getRef(*boot_sprite)
                        ->setPosition(boot_position + BOOT_THROW_SPRITE_OFFSET);
                    if (glm::length(trey_position - boot_position) > traits.throw_distance) {
                        Eend::Entities::boards().erase(*boot_sprite);
                        boot_sprite = std::nullopt;
                    }
                }

                // end throw
                //}
            }
        };
        void hit(Eend::Point trey_position, Dog& dog) {
            if (enabled()) {
                // TODO collision
            }
        };

        bool enabled() { return !item_traits.empty(); };
        void addTraits(BootThrowTraits new_traits) {
            traits += new_traits;
            item_traits.push(new_traits);
        };
        void removeTraits() {
            if (enabled()) {
                BootThrowTraits oldTraits = item_traits.front();
                traits -= oldTraits;
                item_traits.pop();
            }
        };

    private:
        std::optional<Eend::BoardId> boot_sprite = std::nullopt;
        Eend::Point boot_position;

        Eend::Vector throw_vector;

        BootThrowTraits traits;
        std::queue<BootThrowTraits> item_traits;

        float last_throw;
};
