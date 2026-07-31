#pragma once

#include "dog.hpp"
#include "facing.hpp"

void regularKick(Eend::Point& position, Facing& facing, float& upVelocity);
void doubleKick(Eend::Point& position, Facing& facing, float& upVelocity);

bool regularHit(Eend::Point position, Eend::Angle rotation, Dog& dog);
bool doubleHit(Eend::Point position, Eend::Angle rotation, Dog& dog);
