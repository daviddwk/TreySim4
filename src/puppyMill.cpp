#include "puppyMill.hpp"

#include <glm/glm.hpp>

#include <chrono>
#include <print>

const int DMG_TICK_MS = 500;
const unsigned int DOG_DMG = 5;
const float DOG_RADIUS = 3.0f;

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
    if (tickMs >= DMG_TICK_MS) {
        tickLast = now;
        for (Dog& dog : m_dogs) {
            if (glm::length(dog.getPosition() - duck->getPosition2D()) < DOG_RADIUS) {
                duck->health.damage(DOG_DMG);
            }
        }
    }

    std::optional<Eend::CollisionSphere> duckCollision = duck->isKicking();
    if (duckCollision) {
        for (Dog& dog : m_dogs) {
            Eend::Point2D dogPosition2D = dog.getPosition();
            float dogHeight = m_terrain->heightAtPoint(dogPosition2D);
            Eend::Point dogPosition = Eend::Point(dogPosition2D.x, dogPosition2D.y, dogHeight);
            Eend::CollisionSphere dogCollision(dogPosition, DOG_RADIUS);
            // TODO actually use penetration vector for doggy knockback
            // also implement a kickback method for doggys
            Eend::Point knockback3d;
            if (Eend::colliding(*duckCollision, dogCollision, &knockback3d)) {
                dog.kick(Eend::Point2D(knockback3d.x, knockback3d.y));
                std::print("kicked\n");
            }
        }
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : m_dogs) {
        dog.update(dt, duck->getPosition());
    }
}
