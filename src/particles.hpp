#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

using ParticleBehavior =
    std::function<std::optional<Eend::Point>(int32_t, std::chrono::milliseconds)>;

class Particles {
    private:
        class Particle {
            public:
                Particle(const int seed, const Eend::BoardId id);
                int32_t seed;
                Eendgine::BoardId id;
                bool isAlive;
        };
        class ParticleCloud {
            public:
                ParticleCloud(Eend::Point origin, ParticleBehavior movement);
                Eend::Point origin;
                ParticleBehavior movement;
                std::chrono::time_point<std::chrono::steady_clock> start;
                bool isAlive;
                std::vector<Particle> particles;
        };

    public:
        static void construct();
        static void destruct();
        static Particles& get();

        void create(
            const Eend::Point& origin, const Eend::Scale2D& scale,
            const std::vector<Particle>::size_type count, const std::filesystem::path& boardPath,
            const ParticleBehavior movement);

        void update(const float dt);

        Particles();
        ~Particles();

    private:
        std::vector<ParticleCloud> _particleClouds;

        inline static Particles* _instance = nullptr;
};
