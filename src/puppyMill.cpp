#include "puppyMill.hpp"

#include <glm/glm.hpp>

#include <chrono>

const int DMG_TICK_MS = 500;
const unsigned int DOG_DMG = 5;
const float DOG_RADIUS = 3.0f;

PuppyMill::PuppyMill(Terrain* terrain) : _terrain(terrain) {
    // SPAWN SOME DOGS
    _dogs.emplace_back(Eend::Point2D(0.0f, 0.0f), Eend::Scale2D(5.0f, 5.0f), 0.0f, terrain);
}

PuppyMill::~PuppyMill() {}

void PuppyMill::damage(Duck* duck) {
    static auto tickLast = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto tickMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - tickLast).count();
    // 500 ms per tick
    if (tickMs >= DMG_TICK_MS) {
        tickLast = now;
        for (Dog& dog : _dogs) {
            if (glm::length(dog.getPosition() - duck->getPosition2D()) < DOG_RADIUS) {
                duck->health.damage(DMG_DOG);
            }
        }
    }
}

void PuppyMill::update(float dt, Duck* duck) {
    for (Dog& dog : _dogs) {
        dog.update(dt, duck->getPosition());
    }
}
