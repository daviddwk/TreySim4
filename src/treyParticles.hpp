#include <Eendgine/particles.hpp>
#include <Eendgine/types.hpp>

#include "facing.hpp"

Eend::Particles::Properties getJumpParticleProperties();
Eend::Particles::Properties getKickParticleProperties(Direction direction);

Eend::Particles::Properties
getVisualParticleProperties(Direction direction, Eend::Angle spread, float distance);
