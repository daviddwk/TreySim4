#include "puppyMill.hpp"
#include <Eendgine/random.hpp>

#include <glm/glm.hpp>
#include <print>

#include <chrono>

float calcKnockback(float depthRatio) {
    // scale from 0 to 1   or   min to max knockback
    return pow(depthRatio, 2.0f);
}

PuppyMill::PuppyMill(Terrain* terrain) : m_terrain(terrain) {}

PuppyMill::~PuppyMill() {
    for (Dog& dog : m_dogs) {
        dog.eraseEntities();
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : m_dogs) {
        dog.update(dt, duck->getPosition());
    }
    damage(duck);
    spawn();
}

void PuppyMill::spawn() {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    if (tickMs >= M_SPAWN_TIME_MS) {
        tickLast = now;
        Eend::Point2D spawnPosition = Eend::Point2D(0.0f);
        if (Eend::randomRange(0, 1)) {
            spawnPosition.x = m_terrain->getWidth();
        }
        if (Eend::randomRange(0, 1)) {
            spawnPosition.y = -m_terrain->getHeight();
        }
        m_dogs.emplace_back(spawnPosition, Eend::Scale2D(5.0f, 5.0f), 0.0f, m_terrain);
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
            if (colliding && (dog.getHealth() > 0)) {
                duck->health.damage(dog.getDamage());
            }
        }
    }

    if (duck->isKicking()) {
        for (Dog& dog : m_dogs) {
            duck->kick(dog);
        }
    }
    for (std::vector<Dog>::size_type dogIdx = m_dogs.size() - 1; dogIdx < m_dogs.size(); --dogIdx) {
        if (m_dogs[dogIdx].shouldDelete()) {
            m_dogs[dogIdx].eraseEntities();
            m_dogs.erase(m_dogs.begin() + dogIdx);
            break;
        }
    }
}
