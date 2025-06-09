#include "puppyMill.hpp"
#include <glm/glm.hpp>

PuppyMill::PuppyMill(Terrain* terrain) : _terrain(terrain) {
    // SPAWN SOME DOGS
    _dogs.emplace_back(Eend::Point2D(0.0f, 0.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
}

PuppyMill::~PuppyMill() {}

void PuppyMill::damage(Duck* duck) {
    for (Dog& dog : _dogs) {
        if (glm::length(dog.getPosition() - duck->getPosition2D()) < 2.0f) {
            duck->health.damage(1);
        }
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : _dogs) {
        dog.update(dt, duck->getPosition());
    }
}
