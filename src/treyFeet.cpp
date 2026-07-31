#include "treyFeet.hpp"

#include <Eendgine/audio.hpp>
#include <Eendgine/particles.hpp>
#include <Eendgine/types.hpp>

#include "collision.hpp"
#include "treyParticles.hpp"

static constexpr float KICK_RADIUS = 15.0f;
static constexpr float KICK_SPREAD = 120.0f;

void regularKick(Eend::Point& position, Facing& facing, float& upVelocity) {

    upVelocity = 20.0f;

    Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getDirection()));
    Eend::Particles::get().create(position, 5, getJumpParticleProperties());
    Eend::Audio::get().playNoise("resources/noises/Quack_Fart_Noise_44k.wav", 100.0f);
}

void doubleKick(Eend::Point& position, Facing& facing, float& upVelocity) {

    upVelocity = 20.0f;

    Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getDirection()));
    Eend::Particles::get().create(position, 2, getKickParticleProperties(facing.getOpposite()));
    Eend::Particles::get().create(position, 5, getJumpParticleProperties());
    Eend::Audio::get().playNoise("resources/noises/Quack_Fart_Noise_44k.wav", 100.0f);
}

bool regularHit(Eend::Point position, Eend::Angle rotation, Dog& dog) {
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS),
        rotation,
        KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        return dog.giveDamage(1);
    }
    return false;
}

bool doubleHit(Eend::Point position, Eend::Angle rotation, Dog& dog) {
    bool dies = false;
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS),
        rotation,
        KICK_SPREAD);
    if (kick) {
        // include the damage here also I think
        dog.kick(*kick);
        dies = dog.giveDamage(1);
    }
    kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(position, KICK_RADIUS),
        rotation + Eend::Angle(180.0f),
        KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        dies = dog.giveDamage(1);
    }
    return dies;
}
