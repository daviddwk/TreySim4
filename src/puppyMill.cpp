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

PuppyMill::PuppyMill(std::weak_ptr<Terrain> terrain)
    : m_numKilled(0), m_terrain(terrain), m_waveIdx(0) {
    m_spawnWaves.emplace_back();
    m_spawnWaves[0].emplace_back();
    m_spawnWaves[0][0].tile = Tile(10.0f, 10.0f);
    m_spawnWaves[0][0].timing[Dog::Type::Classic] =
        std::make_tuple(std::chrono::milliseconds(2000), std::chrono::steady_clock::now());
    m_spawnWaves.emplace_back();
    m_spawnWaves[1].emplace_back();
    m_spawnWaves[1][0].tile = Tile(20.0f, 10.0f);
    m_spawnWaves[1][0].timing[Dog::Type::Snow] =
        std::make_tuple(std::chrono::milliseconds(1000), std::chrono::steady_clock::now());
}

bool PuppyMill::setWaveIdx(std::vector<Dog>::size_type waveIdx) {
    bool overflowed = false;
    if (waveIdx >= m_spawnWaves.size()) {
        // do loop waves
        waveIdx = waveIdx % m_spawnWaves.size();
        overflowed = true;
    }
    m_waveIdx = waveIdx;
    return overflowed;
}
// returns true if end of waves
bool PuppyMill::nextWave() {
    auto nextWaveIdx = m_waveIdx + 1;
    bool reachedEnd = false;
    if (nextWaveIdx >= m_spawnWaves.size()) {
        // don't loop waves
        nextWaveIdx = m_spawnWaves.size() - 1;
        reachedEnd = true;
    }
    m_waveIdx = nextWaveIdx;
    return reachedEnd;
}

void PuppyMill::update() {
    for (Dog& dog : m_dogs) {
        dog.update();
    }

    damage();
    spawn();
}

unsigned int PuppyMill::getNumKilled() { return m_numKilled; }

void PuppyMill::spawn() {
    auto now = std::chrono::steady_clock::now();
    for (Spawn& spawn : m_spawnWaves[m_waveIdx]) {
        for (auto& [dogType, timing] : spawn.timing) {
            auto& [frequency, next] = timing;
            if (now > next) {
                m_dogs.emplace_back(
                    m_terrain.lock().get()->positionAtTile(spawn.tile),
                    Eend::Scale2D(5.0f, 5.0f),
                    0.0f,
                    dogType);
                next = now + frequency;
                // no drift but I get a large backlog of dogs for future waves atm
                // next = next + frequency;
            }
        }
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
