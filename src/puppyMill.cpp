#include "puppyMill.hpp"

#include <glm/glm.hpp>
#include <print>

#include <chrono>

float calcKnockback(float depthRatio) {
    // scale from 0 to 1   or   min to max knockback
    return pow(depthRatio, 2.0f);
}

PuppyMill::PuppyMill(Terrain* terrain) : m_terrain(terrain) {
    m_dogs.reserve(10);
    m_dogs.emplace_back(Eend::Point2D(0.0f, 0.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
    m_dogs.emplace_back(Eend::Point2D(10.0f, 0.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
    m_dogs.emplace_back(Eend::Point2D(0.0f, 10.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
}

PuppyMill::~PuppyMill() {
    for (Dog& dog : m_dogs) {
        dog.eraseEntities();
    }
}

void PuppyMill::damage(Duck* duck) {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    // 500 ms per tick
    if (tickMs >= M_DMG_TICK_MS) {
        tickLast = now;
        for (Dog& dog : m_dogs) {
            const float distance = glm::length(dog.getPosition() - duck->getPosition2D());
            const bool colliding = distance < duck->getRadius();
            if (colliding) {
                duck->health.damage(dog.getDamage());
            }
        }
    }

    if (duck->isKicking()) {
        for (Dog& dog : m_dogs) {
            duck->kick(dog);
        }
    }
    for (Dog& dog : m_dogs) {
        if (dog.getHealth() == 0) {
            // use kill method to make dog ded, but still use puppymill to delete when time
            // std::print("KILL\n");
            // kill
        }
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : m_dogs) {
        dog.update(dt, duck->getPosition());
    }
}
