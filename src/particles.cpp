#include "Eendgine/entityBatches.hpp"
#include "particles.hpp"

#include <Eendgine/random.hpp>

#include <chrono>

namespace Eend = Eendgine;

Particles::Particle::Particle(const int32_t seed, const Eend::BoardId id)
    : seed(seed), id(id), isAlive(true) {}

Particles::ParticleCloud::ParticleCloud(Eend::Point origin, ParticleBehavior movement)
    : origin(origin), movement(movement), start(std::chrono::steady_clock::now()), isAlive(true),
      particles(std::vector<Particle>()) {}

Particles::Particles() {}

Particles::~Particles() {
    for (auto const& cloud : _particleClouds) {
        for (auto const& particle : cloud.particles) {
            Eend::Entities::boards().erase(particle.id);
        }
    }
}

void Particles::construct() {
    assert(_instance == nullptr);
    _instance = new Particles();
}

void Particles::destruct() {
    assert(_instance != nullptr);
    delete _instance;
    _instance = nullptr;
}

Particles& Particles::get() {
    assert(_instance != nullptr);
    return *_instance;
}

void Particles::create(
    const Eend::Point& origin, const Eend::Scale2D& scale,
    const std::vector<Particle>::size_type count, const std::filesystem::path& boardPath,
    const ParticleBehavior movement) {

    std::vector<Particle>::size_type cloudIdx = _particleClouds.size();

    _particleClouds.emplace_back(origin, movement);

    for (std::vector<Particle>::size_type particleIdx = 0; particleIdx < count; ++particleIdx) {
        const Eend::BoardId id = Eend::Entities::boards().insert(boardPath);
        Eend::Entities::boards().getRef(id)->setScale(scale);
        uint32_t seed = Eend::randomIntLimit();
        _particleClouds[cloudIdx].particles.emplace_back(seed, id);
    }
}

void Particles::update(const float dt) {
    for (auto cloudIter = _particleClouds.begin(); cloudIter != _particleClouds.end();) {
        bool cloudIsAlive = false;

        for (auto& particle : cloudIter->particles) {
            if (particle.isAlive) {
                std::chrono::milliseconds duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - cloudIter->start);
                std::optional<Eend::Point> relativePosition =
                    cloudIter->movement(particle.seed, duration);
                if (relativePosition) {
                    cloudIsAlive = true;
                    Eend::Point position = *relativePosition + cloudIter->origin;
                    Eend::Entities::boards().getRef(particle.id)->setPosition(position);
                } else {
                    particle.isAlive = false;
                    Eend::Entities::boards().erase(particle.id);
                }
            }
        }
        if (cloudIsAlive) {
            ++cloudIter;
        } else {
            cloudIter = _particleClouds.erase(cloudIter);
        }
    }
}
