#include <Eendgine/inputManager.hpp>
#include <cmath>

#include "duck.hpp"
#include "particles.hpp"

static Particles::Behavior particleMovement =
    [](int seed, std::chrono::milliseconds time) -> std::optional<Particles::Properties> {
    if (time.count() < 1000) {
        const unsigned int bits = 8;
        const unsigned int max = std::pow(2, bits);

        const float speedScale = 0.3f;

        float x = static_cast<float>(time.count()) / static_cast<float>((seed >> bits * 0) % max) *
                  (((seed >> bits * 0) % 2) ? 1 : -1) * speedScale;
        float y = static_cast<float>(time.count()) / static_cast<float>((seed >> bits * 1) % max) *
                  (((seed >> bits * 1) % 2) ? 1 : -1) * speedScale;
        float z = static_cast<float>(time.count()) /
                  (8.0f * static_cast<float>((seed >> bits * 2) % max)) * speedScale;

        float scale = (3000.0f - static_cast<float>(time.count() * 3)) / 3000.0f;

        return std::make_optional(
            Particles::Properties(Eend::Point(x, y, z), Eend::Scale2D(scale * 2.0f)));
    }
    return std::nullopt;
};

Duck::Duck()
    : _bodyId(Eend::Entities::statues().insert(std::filesystem::path("duck/statues/body"))),
      _headId(Eend::Entities::boards().insert(std::filesystem::path("duck/boards/head"))),
      _position(Eend::Point(0.0f)), _rotX(0.0f), _rotY(0.0f), _inAir(false), _upVelocity(0.0f),
      _height(0.0f) {
    Eend::Entities::boards().getRef(_headId)->setScale(Eend::Scale2D(3.5f, 3.5f));
}

Duck::~Duck() {
    Eend::Entities::statues().erase(_bodyId);
    Eend::Entities::boards().erase(_headId);
}

void Duck::setPosition(Eend::Point position) {
    if (position.y < _position.y) {
        Eend::Entities::boards().getRef(_headId)->setStrip("eyesOpen");
    } else if (position.y > _position.y) {
        Eend::Entities::boards().getRef(_headId)->setStrip("eyesClose");
    }
    _position = position;
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(_headId);
    bodyRef->setPosition(Eend::Point(position.x - 0.5f, position.y, position.z + 0.08f));
    headRef->setPosition(Eend::Point(position.x, position.y, position.z + 3.00f));
}

void Duck::setRotation(float x, float y, float z) {
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(_bodyId);
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
    if (Eend::InputManager::get().getUpPress()) {
        duckPosition.y += 25.0f * dt;
        // stupid hack because my trig is mid
        if (Eend::InputManager::get().getRightPress()) {
            duckRotationOffset = -180.0f;
        } else {
            duckRotationOffset += 180.0f;
        }
        numPressed++;
    }
    if (Eend::InputManager::get().getDownPress()) {
        duckPosition.y -= 25.0f * dt;
        duckRotationOffset += 0.0f;
        numPressed++;
    }
    if (Eend::InputManager::get().getLeftPress()) {
        duckPosition.x -= 25.0f * dt;
        duckRotationOffset += 90.0f;
        numPressed++;
    }
    if (Eend::InputManager::get().getRightPress()) {
        duckPosition.x += 25.0f * dt;
        duckRotationOffset -= 90.0f;
        numPressed++;
    }

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

    float heightAtPoint = terrain->heightAtPoint(Eend::Point2D(duckPosition.x, duckPosition.y));

    if (Eend::InputManager::get().getSpacePress() && !_inAir) {
        Particles::get().create(
            duckPosition, 5, std::filesystem::path("duck/boards/poo"), particleMovement);
        _inAir = true;
        _upVelocity = -GRAVITY * 20.0f;
        _height = heightAtPoint + 0.1f;
    } else if (_inAir) {
        _upVelocity += GRAVITY;
        _height += (_upVelocity * dt);
        if (_height < heightAtPoint) {
            _inAir = false;
            _height = heightAtPoint;
        }
    } else {
        _height = heightAtPoint;
    }

    duckPosition.z = _height;

    if (_inAir) {
    }

    setPosition(duckPosition);
    setRotation(0.0f, 0.0f, duckRotation);
}
