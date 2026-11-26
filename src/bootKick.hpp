#pragma once

#include <glm/ext/quaternion_transform.hpp>
#include <queue>

#include <Eendgine/frameLimiter.hpp>

#include "dog.hpp"
#include "facing.hpp"

namespace Eend = Eendgine;

class BootKickTraits {
    public:
        BootKickTraits() = default;
        BootKickTraits(float kick_offset, float kick_radius);

        BootKickTraits& operator+=(const BootKickTraits& rhs);
        BootKickTraits& operator-=(const BootKickTraits& rhs);

        float kick_offset;
        float kick_radius;
};

class BootKickItem {
    public:
        BootKickItem(float kick_offset, float kick_radius);
        const BootKickTraits traits;

    private:
};

class BootKick { // TODO particle system that follows boot!
    public:
        BootKick();
        ~BootKick();

        void kick(Eend::Point position, Direction direction, float& upVelocity) const;
        void hit(Eend::Point trey_position, Direction direction, Dog& dog) const;
        bool enabled();

        void addTraits(BootKickTraits newTraits);
        void removeTraits();
        void emptyTraits();

    private:
        Eend::Point boot_position;
        Eend::Vector2D boot_rotation;

        BootKickTraits traits;
        std::queue<BootKickTraits> item_traits;
};
