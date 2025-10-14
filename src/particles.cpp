#include "Eendgine/entityBatches.hpp"
#include "particles.hpp"

#include <Eendgine/random.hpp>

#include <chrono>

namespace Eend = Eendgine;

Particles::Particle::Particle(const int32_t seed, const Eend::BoardId id)
    : seed(seed), id(id), isAlive(true) {}

Particles::Cloud::Cloud(Eend::Point origin, Particles::Behavior movement)
    : origin(origin), movement(movement), start(std::chrono::steady_clock::now()), isAlive(true),
      particles(std::vector<Particle>()) {}

Particles::Particles() {}

Particles::~Particles() {
    for (auto const& cloud : m_clouds) {
        for (auto const& particle : cloud.particles) {
            Eend::Entities::boards().erase(particle.id);
        }
    }
}

void Particles::construct() {
    assert(m_instance == nullptr);
    m_instance = new Particles();
}

void Particles::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Particles& Particles::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void Particles::create(
    const Eend::Point& origin, const std::vector<Particle>::size_type count,
    const std::filesystem::path& boardPath, const Particles::Behavior movement) {

    std::vector<Particle>::size_type cloudIdx = m_clouds.size();

    m_clouds.emplace_back(origin, movement);

    for (std::vector<Particle>::size_type particleIdx = 0; particleIdx < count; ++particleIdx) {
        const Eend::BoardId id = Eend::Entities::boards().insert(boardPath);
        uint32_t seed = Eend::randomIntLimit();
        m_clouds[cloudIdx].particles.emplace_back(seed, id);
    }
}

void Particles::update(const float dt) {
    for (auto cloudIter = m_clouds.begin(); cloudIter != m_clouds.end();) {
        bool cloudIsAlive = false;

        for (auto& particle : cloudIter->particles) {
            if (particle.isAlive) {

                std::chrono::milliseconds duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - cloudIter->start);

                std::optional<Particles::Properties> properties =
                    cloudIter->movement(particle.seed, duration);

                if (properties) {
                    cloudIsAlive = true;
                    Eend::Board* boardRef = Eend::Entities::boards().getRef(particle.id);

                    const Eend::Point position = properties->relativePosition + cloudIter->origin;

                    boardRef->setPosition(position);
                    boardRef->setScale(properties->scale);
                    boardRef->setStripIdx(properties->stripIdx);
                } else {
                    particle.isAlive = false;
                    Eend::Entities::boards().erase(particle.id);
                }
            }
        }
        if (cloudIsAlive) {
            ++cloudIter;
        } else {
            cloudIter = m_clouds.erase(cloudIter);
        }
    }
}
