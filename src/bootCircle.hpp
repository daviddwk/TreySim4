#pragma once

#include <glm/ext/quaternion_transform.hpp>

#include <Eendgine/frameLimiter.hpp>

#include "dog.hpp"
#include "traitQueue.hpp"

namespace Eend = Eendgine;

class BootCircleTraits {
    public:
        BootCircleTraits() = default;
        BootCircleTraits(float bootRadius, float circleRadius, float speed);

        BootCircleTraits& operator+=(const BootCircleTraits& rhs);
        BootCircleTraits& operator-=(const BootCircleTraits& rhs);

        float boot_radius;
        float circle_radius;
        float speed;
};

class BootCircleItem {
    public:
        BootCircleItem(float boot_radius, float circle_radius, float speed);
        const BootCircleTraits traits;

    private:
};

class BootCircle { // TODO particle system that follows boot!
    public:
        BootCircle();
        ~BootCircle();
        BootCircle(const BootCircle&) = delete;
        BootCircle& operator=(const BootCircle&) = delete;

        void update(Eend::Point trey_position);
        void hit(Eend::Point trey_position, Dog& dog);

        void addTrait(BootCircleTraits newTraits, std::filesystem::path texture);
        void popTrait();
        void emptyTraits();

    private:
        void positionIcons();

        std::optional<Eend::BoardId> boot_sprite;

        Eend::Point boot_position;
        Eend::Vector2D boot_rotation;

        TraitQueue<BootCircleTraits, 0.0f> trait_queue;
};
