#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

class Particles {
    public:
        class Properties {
            public:
                Properties(Eend::Point relativePosition, Eend::Scale2D scale)
                    : relativePosition(relativePosition), scale(scale) {}
                Eend::Point relativePosition;
                Eend::Scale2D scale;
        };
        using Behavior =
            std::function<std::optional<Properties>(int32_t, std::chrono::milliseconds)>;

        static void construct();
        static void destruct();
        static Particles& get();

    private:
        class Particle {
            public:
                Particle(const int seed, const Eend::BoardId id);
                int32_t seed;
                Eendgine::BoardId id;
                bool isAlive;
        };
        class Cloud {
            public:
                Cloud(Eend::Point origin, Behavior movement);
                Eend::Point origin;
                Behavior movement;
                std::chrono::time_point<std::chrono::steady_clock> start;
                bool isAlive;
                std::vector<Particle> particles;
        };

    public:
        void create(
            const Eend::Point& origin, const std::vector<Particle>::size_type count,
            const std::filesystem::path& boardPath, const Behavior movement);

        void update(const float dt);

        Particles();
        ~Particles();

    private:
        std::vector<Cloud> m_clouds;
        inline static Particles* m_instance = nullptr;
};
