#include <Eendgine/inputManager.hpp>

#include "duck.hpp"

Duck::Duck()
    : _bodyId(Eend::Entities::StatueBatch::insert(std::filesystem::path("duck/statues/body"))),
      _headId(Eend::Entities::BoardBatch::insert(std::filesystem::path("duck/boards/head"))),
      _position(Eend::Point(0.0f)), _rotX(0.0f), _rotY(0.0f) {
    Eend::Entities::BoardBatch::getRef(_headId)->setScale(Eend::Scale2D(3.5f, 3.5f));
}

Duck::~Duck() {
    Eend::Entities::StatueBatch::erase(_bodyId);
    Eend::Entities::BoardBatch::erase(_headId);
}

void Duck::setPosition(Eend::Point position) {
    if (position.y < _position.y) {
        Eend::Entities::BoardBatch::getRef(_headId)->setStrip("eyesOpen");
    } else if (position.y > _position.y) {
        Eend::Entities::BoardBatch::getRef(_headId)->setStrip("eyesClose");
    }
    _position = position;
    Eend::Statue* bodyRef = Eend::Entities::StatueBatch::getRef(_bodyId);
    Eend::Board* headRef = Eend::Entities::BoardBatch::getRef(_headId);
    bodyRef->setPosition(Eend::Point(position.x - 0.5f, position.y, position.z + 0.08f));
    headRef->setPosition(Eend::Point(position.x, position.y, position.z + 3.00f));
}

void Duck::setRotation(float x, float y, float z) {
    Eend::Statue* bodyRef = Eend::Entities::StatueBatch::getRef(_bodyId);
    bodyRef->setRotation(x, y, z);
};

Eend::Point Duck::getPosition() { return _position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(_position.x, _position.y); };

void Duck::update(float dt, Terrain* terrain) {

    Eend::Point duckPosition = getPosition();
    Eend::Point oldDuckPosition = duckPosition;

    float duckRotation = 0.0f;

    float duckRotationOffset = 0.0f;
    unsigned int numPressed = 0;
    if (Eend::InputManager::getUpPress()) {
        duckPosition.y += 25.0f * dt;
        // stupid hack because my trig is mid
        if (Eend::InputManager::getRightPress()) {
            duckRotationOffset = -180.0f;
        } else {
            duckRotationOffset += 180.0f;
        }
        numPressed++;
    }
    if (Eend::InputManager::getDownPress()) {
        duckPosition.y -= 25.0f * dt;
        duckRotationOffset += 0.0f;
        numPressed++;
    }
    if (Eend::InputManager::getLeftPress()) {
        duckPosition.x -= 25.0f * dt;
        duckRotationOffset += 90.0f;
        numPressed++;
    }
    if (Eend::InputManager::getRightPress()) {
        duckPosition.x += 25.0f * dt;
        duckRotationOffset -= 90.0f;
        numPressed++;
    }
    // COORDINATE SYSTMES ARE TOTALLY WACKED UP RN

    if (!terrain->colliding(Eend::Point2D(duckPosition.x, duckPosition.y))) {
    } else if (!terrain->colliding(Eend::Point2D(oldDuckPosition.x, duckPosition.y))) {
        duckPosition.x = oldDuckPosition.x;
    } else if (!terrain->colliding(Eend::Point2D(duckPosition.x, oldDuckPosition.y))) {
        duckPosition.y = oldDuckPosition.y;
    } else {
        duckPosition.x = oldDuckPosition.x;
        duckPosition.y = oldDuckPosition.y;
    }

    if (numPressed) {
        duckRotation = (duckRotationOffset / (float)numPressed);
    } else {
        duckRotation += 100.0f * dt;
    }

    duckPosition.z = terrain->heightAtPoint(Eend::Point2D(duckPosition.x, duckPosition.y));
    setPosition(duckPosition);
    setRotation(0.0f, 0.0f, duckRotation);
}
