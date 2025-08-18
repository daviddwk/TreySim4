#pragma once

#include <chrono>
#include <optional>
#include <vector>

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

using ParticleMovement = std::function<std::optional<Eend::Point>(int, std::chrono::milliseconds)>;

class Particles {
    private:
        class Particle {
            public:
                Particle(const int seed, const Eend::BoardId id)
                    : seed(seed), id(id), isAlive(true) {}

                int seed;
                Eendgine::BoardId id;
                bool isAlive;
        };
        class ParticleCloud {
            public:
                ParticleCloud(Eend::Point origin, ParticleMovement movement)
                    : origin(origin), movement(movement), start(std::chrono::steady_clock::now()),
                      isAlive(true), particles(std::vector<Particle>()) {}
                Eend::Point origin;
                ParticleMovement movement;
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
            const ParticleMovement movement);

        void update(const float dt);

        Particles();
        ~Particles();

    private:
        std::vector<ParticleCloud> _particleClouds;

        inline static Particles* _instance = nullptr;
};
