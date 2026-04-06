#include "puppyMill.hpp"

#include "park.hpp"
#include "trey.hpp"

#include <Eendgine/fatalError.hpp>
#include <Eendgine/random.hpp>

#include <fstream>
#include <glm/glm.hpp>
#include <json/json.h>
#include <print>

#include <chrono>

float calcKnockback(float depthRatio) {
    // scale from 0 to 1   or   min to max knockback
    return pow(depthRatio, 2.0f);
}

PuppyMill::PuppyMill(std::weak_ptr<Terrain> terrain, std::filesystem::path parkPath)
    : m_numKilled(0), m_terrain(terrain), m_waveIdx(0) {
    /*
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
    */

    Json::Value rootJson;
    std::filesystem::path metadataPath = "resources" / parkPath / "generate/metadata.json";
    std::ifstream metadata(metadataPath);
    if (!metadata.is_open()) {
        Eend::fatalError("could not open: " + metadataPath.string());
    }
    try {
        metadata >> rootJson;
    } catch (...) {
        Eend::fatalError("improper json: " + metadataPath.string());
    }

    Json::Value dogWavesJson = rootJson["Waves"];
    if (!dogWavesJson.isArray()) Eend::fatalError("No Dog waves array");
    for (int dogWaveIdx = 0; dogWaveIdx < dogWavesJson.size(); ++dogWaveIdx) {
        Json::Value dogSpawnsJson = dogWavesJson[dogWaveIdx];
        if (!dogSpawnsJson.isArray()) Eend::fatalError("Dog spawns json not array");
        m_spawnWaves.emplace_back();
        for (int dogSpawnIdx = 0; dogSpawnIdx < dogSpawnsJson.size(); ++dogSpawnIdx) {
            Json::Value dogSpawnJson = dogSpawnsJson[dogSpawnIdx];
            Json::Value dogPositionJson = dogSpawnJson["position"];
            if (!dogPositionJson.isArray()) Eend::fatalError("Dog spawn position not array");
            if (dogPositionJson.size() != 2) Eend::fatalError("Dog spawn position not size 2");
            if (!dogPositionJson[0].isNumeric())
                Eend::fatalError("Dog spawn position x not a number");
            if (!dogPositionJson[1].isNumeric())
                Eend::fatalError("Dog spawn position y not a number");
            Tile position = Tile(dogPositionJson[0].asFloat(), dogPositionJson[1].asFloat());
            Json::Value dogTimingsJson = dogSpawnJson["timing"];
            if (!dogTimingsJson.isArray()) Eend::fatalError("Dog timing not array");
            // TODO make this have a constructor and emplace back here
            m_spawnWaves[dogWaveIdx].emplace_back();
            m_spawnWaves[dogWaveIdx][dogSpawnIdx].tile = position;
            for (int dogTimingIdx = 0; dogTimingIdx < dogTimingsJson.size(); ++dogTimingIdx) {
                Json::Value dogTimingJson = dogTimingsJson[dogTimingIdx];
                Json::Value dogTypeJson = dogTimingJson["type"];
                Json::Value dogFrequencyJson = dogTimingJson["frequency"];
                if (!dogTypeJson.isString()) Eend::fatalError("Dog type not string");
                if (!dogFrequencyJson.isNumeric()) Eend::fatalError("Dog frequency not a number");
                // TODO I need a string to dog type util function
                Dog::Type type = Dog::stringToType(dogTypeJson.asString());
                m_spawnWaves[dogWaveIdx][dogSpawnIdx].timing[type] = std::make_tuple(
                    std::chrono::milliseconds(dogFrequencyJson.asInt()),
                    std::chrono::steady_clock::now());
            }
        }
    }
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
    if (m_spawnWaves.size() == 0) return;
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
