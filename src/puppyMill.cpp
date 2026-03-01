#include "puppyMill.hpp"

#include "park.hpp"
#include "trey.hpp"

#include <Eendgine/random.hpp>

#include <glm/glm.hpp>
#include <print>

#include <chrono>

float calcKnockback(float depthRatio) {
    // scale from 0 to 1   or   min to max knockback
    return pow(depthRatio, 2.0f);
}

PuppyMill::PuppyMill() : m_numKilled(0) {}

void PuppyMill::update() {
    for (Dog& dog : m_dogs) {
        dog.update();
    }

    damage();
    spawn();
}

unsigned int PuppyMill::getNumKilled() { return m_numKilled; }

void PuppyMill::spawn() {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    if (tickMs >= M_SPAWN_TIME_MS) {
        tickLast = now;
        Eend::Point2D spawnPosition = Eend::Point2D(0.0f);
        if (Eend::randomRange(0, 1)) {
            spawnPosition.x = Park::get().getWidth();
        }
        if (Eend::randomRange(0, 1)) {
            spawnPosition.y = -Park::get().getHeight();
        }
        m_dogs.emplace_back(spawnPosition, Eend::Scale2D(5.0f, 5.0f), 0.0f);
    }
}

void PuppyMill::damage() {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    // 500 ms per tick
    if (tickMs >= M_DMG_TICK_MS) {
        tickLast = now;
        for (Dog& dog : m_dogs) {
            const Eend::Point2D treyPosition = Trey::get().getPosition();
            const float treyRadius = Trey::get().getRadius();
            const float distance = glm::length(dog.getPosition() - treyPosition);
            const bool colliding = distance < treyRadius;
            if (colliding && (dog.getHealth() > 0)) {
                Trey::get().health.damage(dog.getDamage());
            }
        }
    }

    if (Trey::get().isKicking()) {
        for (Dog& dog : m_dogs) {
            bool dogDies = Trey::get().kick(dog);
            if (dogDies) m_numKilled++;
        }
    }
    for (std::vector<Dog>::size_type dogIdx = m_dogs.size() - 1; dogIdx < m_dogs.size(); --dogIdx) {
        if (m_dogs[dogIdx].shouldDelete()) {
            m_dogs.erase(m_dogs.begin() + dogIdx);
            break;
        }
    }
}
