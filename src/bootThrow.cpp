#include "bootThrow.hpp"

namespace Eend = Eendgine;

constexpr float THROW_DEFAULT_BOOT_RADIUS = 2.5f;
constexpr float THROW_DEFAULT_THROW_DISTANCE = 25.0f;
constexpr float THROW_DEFAULT_FREQUENCY = 1.0f;
constexpr float THROW_DEFAULT_SPEED = 1.0f;
const Eend::Point BOOT_THROW_SPRITE_OFFSET = Eend::Point(0.0f, 0.0f, 5.0f);
const std::filesystem::path BOOT_THROW_SPRITE_PATH = std::filesystem::path("resources/boot");

// traits
BootThrowTraits::BootThrowTraits(
    float boot_radius, float throw_distance, float throw_frequency, float throw_speed)
    : boot_radius(boot_radius), throw_distance(throw_distance), throw_frequency(throw_frequency),
      throw_speed(throw_speed) {};

BootThrowTraits& BootThrowTraits::operator+=(const BootThrowTraits& rhs) {
    boot_radius += rhs.boot_radius;
    throw_distance += rhs.throw_distance;
    throw_frequency += rhs.throw_frequency;
    throw_speed += rhs.throw_speed;
    return *this;
}
BootThrowTraits& BootThrowTraits::operator-=(const BootThrowTraits& rhs) {
    boot_radius -= rhs.boot_radius;
    throw_distance -= rhs.throw_distance;
    throw_distance -= rhs.throw_distance;
    throw_speed -= rhs.throw_speed;
    return *this;
}

// item
BootThrowItem::BootThrowItem(
    float boot_radius, float throw_distance, float throw_frequency, float throw_speed)
    : traits(boot_radius, throw_distance, throw_frequency, throw_speed) {}

// throw
BootThrow::BootThrow()
    : traits(
          THROW_DEFAULT_BOOT_RADIUS, THROW_DEFAULT_THROW_DISTANCE, THROW_DEFAULT_FREQUENCY,
          THROW_DEFAULT_SPEED),
      last_throw(0.0f) {
    BootThrow::item_traits.emplace(0.0f, 0.0f, 0.0f, 0.0f); // DEBUG
}

BootThrow::~BootThrow() {}

void BootThrow::update(Eend::Point trey_position, Eend::Vector2D trey_direction) {
    if (enabled()) {
        float dt = Eend::FrameLimiter::get().deltaTime;

        last_throw += dt;

        const bool throwing = boot_sprite.has_value();
        const bool new_throw = last_throw >= traits.throw_frequency;
        if (!throwing && new_throw) {

            last_throw = 0.0f;
            boot_sprite = Eend::Entities::boards().insert(BOOT_THROW_SPRITE_PATH);
            throw_vector = Eend::Vector(trey_direction.x, trey_direction.y, 0.0f);
            boot_position = trey_position;
            Eend::Entities::boards()
                .getRef(*boot_sprite)
                ->setScale(Eend::Scale2D(THROW_DEFAULT_BOOT_RADIUS * 2.0f));
            Eend::Entities::boards()
                .getRef(*boot_sprite)
                ->setPosition(boot_position + BOOT_THROW_SPRITE_OFFSET);

        } else if (throwing) {

            boot_position += throw_vector * THROW_DEFAULT_SPEED;
            Eend::Entities::boards()
                .getRef(*boot_sprite)
                ->setPosition(boot_position + BOOT_THROW_SPRITE_OFFSET);

            const bool throw_distance = glm::length(trey_position - boot_position);
            if (throw_distance > traits.throw_distance) {

                Eend::Entities::boards().erase(*boot_sprite);
                boot_sprite = std::nullopt;
            }
        }
    }
};

void BootThrow::hit(Eend::Point trey_position, Dog& dog) {
    if (enabled()) {

        Eend::Sphere bootSphere(boot_position, traits.boot_radius);

        const bool kicked = pointOnSphere(dog.getPosition3d(), bootSphere);
        if (kicked) {

            Eend::Vector kick_vector = boot_position - trey_position;
            if (glm::length(kick_vector) != 0.0f) {
                kick_vector = glm::normalize(kick_vector);
            }
            dog.kick(kick_vector);
            dog.giveDamage(1);
        }
    }
};

bool BootThrow::enabled() { return !BootThrow::item_traits.empty(); };

void BootThrow::addTraits(BootThrowTraits new_traits) {
    BootThrow::traits += new_traits;
    item_traits.push(new_traits);
};

void BootThrow::removeTraits() {
    if (enabled()) {
        BootThrowTraits oldTraits = item_traits.front();
        traits -= oldTraits;
        item_traits.pop();
    }
};
