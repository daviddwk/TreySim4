#include "dog.hpp"

#include <glm/vector_relational.hpp>
namespace Eend = Eendgine;

const float DOG_UP_OFFSET = 2.0f;
const float DOG_SPEED = 20.0f;
const float DOG_CLOSE_ENOUGH = 1.0f;
const float DOG_ANIM_INCREMENT_TIME = 0.25f;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Terrain* terrain)
    : _bodyId(Eend::Entities::BoardBatch::insert(std::filesystem::path("dog/boards/walk"))),
      _position(position), _speed(speed), _terrain(terrain), _time(0.0f) {
    Eend::Entities::BoardBatch::getRef(_bodyId)->setScale(scale);
    Eend::Entities::BoardBatch::getRef(_bodyId)->setPosition(
        Eend::Point(position.x, terrain->heightAtPoint(position), position.y));
}

Dog::~Dog() { Eend::Entities::BoardBatch::erase(_bodyId); }

void Dog::setSpeed(float speed) { _speed = speed; }

void Dog::update(float dt, Eend::Point2D approachPoint) {
    _time += dt;
    const glm::vec2 difference = approachPoint - _position;
    if (glm::length(difference) > DOG_CLOSE_ENOUGH) {
        _position += glm::normalize(approachPoint - _position) * DOG_SPEED * dt;
    }
    // add some offsets for the dog visually here
    Eend::Entities::BoardBatch::getRef(_bodyId)->setPosition(
        Eend::Point(_position.x, _position.y, _terrain->heightAtPoint(_position) + DOG_UP_OFFSET));
    if (_time > DOG_ANIM_INCREMENT_TIME) {
        _time = 0;
        Eend::Entities::BoardBatch::getRef(_bodyId)->nextStripIdx();
    }

    // terrain->heightAtPoint(_position)
}
