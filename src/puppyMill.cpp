#include "Eendgine/collisionGeometry.hpp"
#include "puppyMill.hpp"

#include <glm/glm.hpp>

#include <chrono>
#include <print>

float calcKnockback(float depthRatio) {
    // scale from 0 to 1   or   min to max knockback
    return pow(depthRatio, 2.0f);
}

PuppyMill::PuppyMill(Terrain* terrain) : m_terrain(terrain) {
    // SPAWN SOME DOGS
    m_dogs.emplace_back(Eend::Point2D(0.0f, 0.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
}

PuppyMill::~PuppyMill() {}

void PuppyMill::damage(Duck* duck) {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    // 500 ms per tick
    if (tickMs >= M_DMG_TICK_MS) {
        tickLast = now;
        for (Dog& dog : m_dogs) {
            if (glm::length(dog.getPosition() - duck->getPosition2D()) < duck->getRadius()) {
                duck->health.damage(dog.getDamage());
            }
        }
    }

    const std::optional<Eend::Sphere> duckCollision = duck->isKicking();
    if (duckCollision) {
        for (Dog& dog : m_dogs) {
            // kick is a vector of the direction the dog was kicked with a length of
            // penetration / radius (between 0 and 1)
            std::optional<Eend::Point> kick =
                Eend::pointToSphereEdgeRelative(dog.getPosition3d(), *duckCollision);
            if (kick) {
                std::print("kick:{}\n", glm::length(*kick));
                dog.kick(*kick);
            }
        }
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : m_dogs) {
        dog.update(dt, duck->getPosition());
    }
}
