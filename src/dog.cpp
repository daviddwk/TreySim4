#include "dog.hpp"

#include <glm/vector_relational.hpp>
namespace Eend = Eendgine;

Dog::Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Terrain* terrain)
    : _bodyId(Eend::Entities::BoardBatch::insert(std::filesystem::path("dog/boards/walk"))),
      _position(position), _speed(speed), _terrain(terrain) {
    Eend::Entities::BoardBatch::getRef(_bodyId)->setScale(scale);
    Eend::Entities::BoardBatch::getRef(_bodyId)->setPosition(
        Eend::Point(position.x, terrain->heightAtPoint(position), position.y));
}

Dog::~Dog() { Eend::Entities::BoardBatch::erase(_bodyId); }

void Dog::setSpeed(float speed) { _speed = speed; }

void Dog::update(float dt, Eend::Point2D approachPoint) {
    _position += 0.1f; //(glm::normalize(_position - approachPoint) * _speed * dt);
    _position = approachPoint;
    // add some offsets for the dog visually here
    Eend::Entities::BoardBatch::getRef(_bodyId)->setPosition(
        Eend::Point(_position.x, 0.0f, _position.y));

    // terrain->heightAtPoint(_position)
}
