#pragma once

#include "dog.hpp"
#include "facing.hpp"

namespace Feet {
class Feet {
    public:
        virtual ~Feet() = default;
        virtual void kick(Eend::Point& position, Facing& facing, float& upVelocimity) = 0;
        virtual bool hit(Eend::Point position, Direction direction, Dog& dog) = 0;
};
class Regular : public Feet {
    public:
        void kick(Eend::Point& position, Facing& facing, float& upVelocimity);
        bool hit(Eend::Point position, Direction direction, Dog& dog);
};

class Double : public Feet {
    public:
        void kick(Eend::Point& position, Facing& facing, float& upVelocimity);
        bool hit(Eend::Point position, Direction direction, Dog& dog);
};
} // namespace Feet
