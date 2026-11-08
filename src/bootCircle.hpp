#include <glm/ext/quaternion_transform.hpp>
#include <queue>

#include <Eendgine/frameLimiter.hpp>

#include "dog.hpp"

namespace Eend = Eendgine;

class BootCircleTraits {
    public:
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
        // Item item;
};

class BootCircle { // TODO particle system that follows boot!
    public:
        BootCircle();
        ~BootCircle();

        void update(Eend::Point trey_position);
        void hit(Eend::Point trey_position, Dog& dog);
        bool enabled();

        void addTraits(BootCircleTraits newTraits);
        void removeTraits();

    private:
        std::optional<Eend::BoardId> boot_sprite;
        Eend::Point boot_position;
        Eend::Vector2D boot_rotation;

        BootCircleTraits traits;
        std::queue<BootCircleTraits> item_traits;
};
