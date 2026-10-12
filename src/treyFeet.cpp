#include "treyFeet.hpp"

#include <Eendgine/audio.hpp>
#include <Eendgine/particles.hpp>
#include <Eendgine/types.hpp>

#include "collision.hpp"
#include "facing.hpp"
#include "treyParticles.hpp"

#define VISUAL

static constexpr float KICK_SPREAD = 120.0f;

static Eend::Angle KICK_SPREAD_REGULAR = Eend::Angle(120.0f);
static float KICK_RADIUS_REGULAR = 7.5f;
static float KICK_OFFSET_REGULAR = 3.0f;

void Feet::Regular::kick(Eend::Point& position, Facing& facing, float& upVelocity) {

    upVelocity = 20.0f;

    Eend::Vector2D offsetVector = vectorFromDirection(facing.getDirection()) * KICK_OFFSET_REGULAR;
    Eend::Point kickSphereOffset = Eend::Vector(offsetVector.x, offsetVector.y, 0.0f);

    Eend::Particles::get().create(
        position + kickSphereOffset,
        200,
        getVisualParticleProperties(
            facing.getDirection(),
            Eend::Angle(359.999f),
            KICK_RADIUS_REGULAR));

    // Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getDirection()));
    Eend::Particles::get().create(position, 5, getJumpParticleProperties());
    Eend::Audio::get().playNoise("resources/noises/Quack_Fart_Noise_44k.wav", 100.0f);
}

void Feet::Double::kick(Eend::Point& position, Facing& facing, float& upVelocity) {

    upVelocity = 20.0f;

    Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getDirection()));
    Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getOpposite()));
    Eend::Particles::get().create(position, 5, getJumpParticleProperties());
    Eend::Audio::get().playNoise("resources/noises/Quack_Fart_Noise_44k.wav", 100.0f);
}

bool Feet::Regular::hit(Eend::Point position, Direction direction, Dog& dog) {
    /*
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS_REGULAR),
        rotation,
        KICK_SPREAD_REGULAR);
    */

    Eend::Vector2D offsetVector = vectorFromDirection(direction) * KICK_OFFSET_REGULAR;
    Eend::Point kickSphereOffset = Eend::Vector(offsetVector.x, offsetVector.y, 0.0f);
    Eend::Sphere kickSphere(position + kickSphereOffset, KICK_RADIUS_REGULAR);

    bool kicked = pointOnSphere(dog.getPosition3d(), kickSphere);

    if (kicked) {
        dog.kick(glm::normalize(dog.getPosition3d() - position));
        return dog.giveDamage(1);
    }
    return false;
}

bool Feet::Double::hit(Eend::Point position, Direction direction, Dog& dog) {
    bool dies = false;

    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS_REGULAR),
        Eend::Angle(90.f),
        // rotation,
        KICK_SPREAD);

    if (kick) {
        // include the damage here also I think
        dog.kick(*kick);
        dies = dog.giveDamage(1);
    }
    kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS_REGULAR),
        Eend::Angle(90.0f),
        // rotation + Eend::Angle(180.0f),
        KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        dies = dog.giveDamage(1);
    }
    return dies;
}
