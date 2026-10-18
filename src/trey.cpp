#include <Eendgine/audio.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>

#include <optional>
#include <print>

#include "collision.hpp"
#include "park.hpp"

#include "treyFeet.hpp"
#include "treyParticles.hpp"

#include "trey.hpp"
// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed
//
//

Trey::Trey()
    : head_id(Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/head"))),
      hair_id(Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/hair"))),
      hair_outter_id(
          Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/hair"))),
      body_id(Eend::Entities::boards().insert(std::filesystem::path("resources/Trey/body"))),
      eye_id(Eend::Entities::boards().insert(std::filesystem::path("resources/Trey/eye"))),
      position(Eend::Point(0.0f)), rotation(Eend::Angle(0.0f)), target_rotation(Eend::Angle(0.0f)),
      body_timer(0.075f), kicking(true), in_air(false), up_velocity(0.0f), alive(true),
      feet(std::make_unique<Feet::Regular>()), item(std::nullopt) {

    Eend::Board* const body = Eend::Entities::boards().getRef(body_id);
    Eend::Board* const eye = Eend::Entities::boards().getRef(eye_id);
    // Eend::Statue* const head = Eend::Entities::statues().getRef(m_headId);
    Eend::Statue* const hair = Eend::Entities::statues().getRef(hair_id);
    Eend::Statue* const hairOutter = Eend::Entities::statues().getRef(hair_outter_id);

    // TODO  make convenience function for setting to size of texture
    const float bodyHeight = static_cast<float>(body->getTexture().height);
    const float bodyWidth = static_cast<float>(body->getTexture().width);
    constexpr float BODY_SCALE = 0.12f;

    body->setScale(Eend::Scale2D(bodyWidth * BODY_SCALE, bodyHeight * BODY_SCALE));
    body->setStrip("standForward");

    // Eend::Entities::statues().getRef(m_headId)->setScale(Eend::Scale(0.7f));
    eye->setScale(Eend::Scale(3.3f));
    eye->setStrip("open");

    hair->setStrip("hair");
    hairOutter->setStrip("hairOutter");
    hair->setScale(Eend::Scale(1.1f));
    hairOutter->setScale(Eend::Scale(1.2f));
}

Trey::~Trey() {
    Eend::Entities::statues().erase(head_id);
    Eend::Entities::statues().erase(hair_id);
    Eend::Entities::statues().erase(hair_outter_id);
    Eend::Entities::boards().erase(body_id);
    Eend::Entities::boards().erase(eye_id);
}

void Trey::construct() {
    assert(m_instance == nullptr);
    m_instance = new Trey();
}

void Trey::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Trey& Trey::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void Trey::setPosition(Eend::Point position) { Trey::position = position; }
void Trey::setAlive(bool alive) { Trey::alive = alive; }
void Trey::pickupItem(Item::Type itemType) {
    // TODO make an item queue kinda situation
    item = itemType;
    if (item) {
        feet = std::make_unique<Feet::Double>();
    } else {
        feet = std::make_unique<Feet::Regular>();
    }
}

Eend::Point Trey::getPosition() { return position; };
Eend::Point2D Trey::getPosition2D() { return Eend::Point2D(position.x, position.y); };
float Trey::getRadius() { return M_TREY_RADIUS; }

void Trey::update(bool activeCutscene) {

    float dt = Eend::FrameLimiter::get().deltaTime;
    if (!activeCutscene) {
        Eend::Point oldTreyPosition = getPosition();

        if (alive) {
            Trey::updateDirection();
            if (moving) {
                Trey::updatePosition(dt);
            }

            kicking = false;
            float elevationAtPosition = Park::get().elevationAtPoint(position);

            if (Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_SPACE) && !in_air) {
                Trey::kick(elevationAtPosition);
            } else if (in_air) {
                up_velocity += M_GRAVITY;
                position.z += (up_velocity * dt);
                if (position.z < elevationAtPosition) {
                    in_air = false;
                    position.z = elevationAtPosition;
                }
            } else {
                position.z = elevationAtPosition;
            }
        }

        Trey::handleCollision(oldTreyPosition);
    }
    Trey::updateBody(dt);

    boot_circle.update(Eend::Point(position.x, position.y, Park::get().elevationAtPoint(position)));
}

void Trey::cutsceneUpdate() { Trey::updateBody(Eend::FrameLimiter::get().deltaTime); }

void Trey::kick(float elevationAtPosition) {
    kicking = true;
    in_air = true;
    position.z = elevationAtPosition + 0.1f;
    feet->kick(position, facing, up_velocity);
}

void Trey::hit(Dog& dog) {
    if (Trey::isKicking()) {
        feet->hit(position, facing.getDirection(), dog);
    }
    boot_circle.hit(position, dog);
}

void Trey::updateBody(float dt) {

    Eend::Statue* head = Eend::Entities::statues().getRef(head_id);
    Eend::Statue* hair = Eend::Entities::statues().getRef(hair_id);
    Eend::Statue* hairOutter = Eend::Entities::statues().getRef(hair_outter_id);
    Eend::Board* body = Eend::Entities::boards().getRef(body_id);
    Eend::Board* eye = Eend::Entities::boards().getRef(eye_id);

    Direction direction = facing.getDirection();
    std::string prefix;

    // update strip
    if (in_air) {
        prefix = "kick";
    } else if (moving) {
        prefix = "walk";
    } else {
        prefix = "stand";
    }

    if (direction == Direction::up) {
        body->setStrip(prefix + "Backward");
        body->setStripFlip(false);
    } else if (direction == Direction::upRight) {
        body->setStrip(prefix + "SideBackward");
        body->setStripFlip(false);
    } else if (direction == Direction::right) {
        if (facing.getUpOrDown() == Direction::up) {
            body->setStrip(prefix + "SideBackward");
        } else {
            body->setStrip(prefix + "SideForward");
        }
        body->setStripFlip(false);
    } else if (direction == Direction::downRight) {
        body->setStrip(prefix + "SideForward");
        body->setStripFlip(false);
    } else if (direction == Direction::down) {
        body->setStrip(prefix + "Forward");
        body->setStripFlip(false);
    } else if (direction == Direction::downLeft) {
        body->setStrip(prefix + "SideForward");
        body->setStripFlip(true);
    } else if (direction == Direction::left) {
        if (facing.getUpOrDown() == Direction::up) {
            body->setStrip(prefix + "SideBackward");
        } else {
            body->setStrip(prefix + "SideForward");
        }
        body->setStripFlip(true);
    } else /*  direction == Direction::upLeft */ {
        body->setStrip(prefix + "SideBackward");
        body->setStripFlip(true);
    }

    if (body_timer.update(dt)) {
        body_timer.reset();
        body->nextStripIdx();
    }

    // set head rotation
    rotation = facing.getAngle();
    head->setRotation(Eend::Rotation(0.0f, 0.0f, rotation.getDegrees() + 180.0f));
    hair->setRotation(Eend::Rotation(0.0f, 0.0f, rotation.getDegrees() + 180.0f));
    hairOutter->setRotation(Eend::Rotation(0.0f, 0.0f, rotation.getDegrees() + 180.0f));

    // set eye rotation
    eye->setRotation(eye->getRotation() + (1.0f * dt));
    eye->setPosition(Eend::Point(position.x, position.y, position.z + 4.1f));

    // set position
    body->setPosition(Eend::Point(position.x, position.y, position.z + 3.0f));
    head->setPosition(Eend::Point(position.x, position.y, position.z + 4.1f));
    hair->setPosition(Eend::Point(position.x, position.y, position.z + 4.1f));
    hairOutter->setPosition(Eend::Point(position.x, position.y, position.z + 4.1f));
}

bool Trey::isKicking() { return kicking; }

bool Trey::updateDirection() {

    std::optional<Direction> direction = std::nullopt;
    bool newDirection = false;

    const bool upPress = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_UP);
    const bool rightPress = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_RIGHT);
    const bool downPress = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_DOWN);
    const bool leftPress = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_LEFT);

    if (upPress) {
        if (rightPress) {
            direction = Direction::upRight;
        } else if (leftPress) {
            direction = Direction::upLeft;
        } else {
            direction = Direction::up;
        }
    } else if (downPress) {
        if (rightPress) {
            direction = Direction::downRight;
        } else if (leftPress) {
            direction = Direction::downLeft;
        } else {
            direction = Direction::down;
        }
    } else if (rightPress) {
        direction = Direction::right;
    } else if (leftPress) {
        direction = Direction::left;
    }

    if (direction) {
        if (facing.getDirection() == *direction) newDirection = true;
        facing.setDirection(*direction);
        moving = true;
    } else {
        moving = false;
    }
    return newDirection;
}

void Trey::updatePosition(float dt) {

    switch (facing.getDirection()) {
    case Direction::up:
        position.y += M_MOVE_SPEED * dt;
        break;
    case Direction::upRight:
        position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::right:
        position.x += M_MOVE_SPEED * dt;
        break;
    case Direction::downRight:
        position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::down:
        position.y -= M_MOVE_SPEED * dt;
        break;
    case Direction::downLeft:
        position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::left:
        position.x -= M_MOVE_SPEED * dt;
        break;
    case Direction::upLeft:
        position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    }
}

void Trey::handleCollision(Eend::Point& oldPosition) {
    if (!Park::get().colliding(Eend::Point2D(position.x, position.y))) {
    } else if (!Park::get().colliding(Eend::Point2D(oldPosition.x, position.y))) {
        position.x = oldPosition.x;
    } else if (!Park::get().colliding(Eend::Point2D(position.x, oldPosition.y))) {
        position.y = oldPosition.y;
    } else {
        position.x = oldPosition.x;
        position.y = oldPosition.y;
    }
}

Eend::Angle Trey::getAngle() { return facing.getAngle(); }
