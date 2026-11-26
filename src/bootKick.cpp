#include "bootKick.hpp"

#include <Eendgine/audio.hpp>

#include "collision.hpp"
#include "facing.hpp"
#include "treyParticles.hpp"

constexpr float KICK_DEFAULT_RADIUS = 7.5f;
constexpr float KICK_DEFAULT_OFFSET = 3.0f;

// traits
BootKickTraits::BootKickTraits(float kick_offset, float kick_radius)
    : kick_offset(kick_offset), kick_radius(kick_radius) {};

BootKickTraits& BootKickTraits::operator+=(const BootKickTraits& rhs) {
    kick_offset += rhs.kick_offset;
    kick_radius += rhs.kick_radius;
    return *this;
}

BootKickTraits& BootKickTraits::operator-=(const BootKickTraits& rhs) {
    kick_offset -= rhs.kick_offset;
    kick_radius -= rhs.kick_radius;
    return *this;
}

// item
BootKickItem::BootKickItem(float kick_offset, float kick_radius)
    : traits(kick_offset, kick_radius) {};

// circle
BootKick::BootKick() : boot_rotation(1.0f, 0.0f), traits(KICK_DEFAULT_OFFSET, KICK_DEFAULT_RADIUS) {

    // item_traits.emplace(0.0f, 0.0f, 0.0f); // DEBUG
}

BootKick::~BootKick() {}

void BootKick::kick(Eend::Point position, Direction direction, float& upVelocity) const {
    upVelocity = 20.0f;

    Eend::Particles::get().create(position, 2, getKickParticleProperties(direction));
    Eend::Particles::get().create(
        position,
        2,
        getKickParticleProperties(oppositeDirection(direction)));
    Eend::Particles::get().create(position, 5, getJumpParticleProperties());
    Eend::Audio::get().playNoise("resources/noises/Quack_Fart_Noise_44k.wav", 100.0f);
}

void BootKick::hit(Eend::Point trey_position, Direction direction, Dog& dog) const {

    Eend::Vector2D offsetVector =
        vectorFromDirection(direction) * traits.kick_offset; // TODO use traits
    Eend::Point kickSphereOffset = Eend::Vector(offsetVector.x, offsetVector.y, 0.0f);
    Eend::Sphere kickSphere(trey_position + kickSphereOffset, traits.kick_radius);

    bool kicked = pointOnSphere(dog.getPosition3d(), kickSphere);
    // TODO mechanism for excluding behind Trey

    if (kicked) {
        dog.kick(glm::normalize(dog.getPosition3d() - trey_position));
        dog.giveDamage(1);
    }
}

bool BootKick::enabled() { return !item_traits.empty(); }

void BootKick::addTraits(BootKickTraits newTraits) {
    traits += newTraits;
    item_traits.push(newTraits);
}

void BootKick::removeTraits() {
    if (enabled()) {
        BootKickTraits oldTraits = item_traits.front();
        traits -= oldTraits;
        item_traits.pop();
    }
}

void BootKick::emptyTraits() { item_traits = std::queue<BootKickTraits>(); }
