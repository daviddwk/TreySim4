#include <Eendgine/audio.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>

#include <optional>
#include <print>

#include "collision.hpp"
#include "park.hpp"

#include "treyParticles.hpp"

#include "trey.hpp"
// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed
//
//

Trey::Trey()
    : m_headId(Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/head"))),
      m_hairId(Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/hair"))),
      m_hairOutterId(
          Eend::Entities::statues().insert(std::filesystem::path("resources/Trey/hair"))),
      m_bodyId(Eend::Entities::boards().insert(std::filesystem::path("resources/Trey/body"))),
      m_eyeId(Eend::Entities::boards().insert(std::filesystem::path("resources/Trey/eye"))),
      m_position(Eend::Point(0.0f)), m_rotation(Eend::Angle(0.0f)),
      m_targetRotation(Eend::Angle(0.0f)), m_bodyTimer(0.075f), m_kicking(true), m_inAir(false),
      m_upVelocity(0.0f), m_height(0.0f), m_alive(true), m_item(ItemType::doubleKick) {

    Eend::Board* const body = Eend::Entities::boards().getRef(m_bodyId);
    Eend::Board* const eye = Eend::Entities::boards().getRef(m_eyeId);
    // Eend::Statue* const head = Eend::Entities::statues().getRef(m_headId);
    Eend::Statue* const hair = Eend::Entities::statues().getRef(m_hairId);
    Eend::Statue* const hairOutter = Eend::Entities::statues().getRef(m_hairOutterId);

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
    Eend::Entities::statues().erase(m_headId);
    Eend::Entities::statues().erase(m_hairId);
    Eend::Entities::statues().erase(m_hairOutterId);
    Eend::Entities::boards().erase(m_bodyId);
    Eend::Entities::boards().erase(m_eyeId);
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

void Trey::setPosition(Eend::Point position) { m_position = position; }
void Trey::setAlive(bool alive) { m_alive = alive; }

Eend::Point Trey::getPosition() { return m_position; };
Eend::Point2D Trey::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };
float Trey::getRadius() { return M_TREY_RADIUS; }

void Trey::update() {

    float dt = Eend::FrameLimiter::get().deltaTime;
    Eend::Point oldTreyPosition = getPosition();

    if (m_alive) {
        Trey::updateDirection();
        if (m_moving) {
            Trey::updatePosition(dt);
        }
    }
    handleCollision(oldTreyPosition);

    float heightAtPoint = Park::get().heightAtPoint(Eend::Point2D(m_position.x, m_position.y));

    m_kicking = false;
    if (Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_SPACE) && !m_inAir && m_alive) {
        m_kicking = true;
        m_inAir = true;
        if (m_item) {
            if (*m_item == ItemType::doubleKick) {
                m_upVelocity = -M_GRAVITY * 20.0f;
                m_height = heightAtPoint + 0.1f;

                Eend::Particles::get().create(
                    m_position,
                    2,
                    getKickParticleProperties(m_facing.getDirection()));
                Eend::Particles::get().create(
                    m_position,
                    2,
                    getKickParticleProperties(m_facing.getOpposite()));
                Eend::Particles::get().create(m_position, 5, getJumpParticleProperties());
                Eend::Audio::get().playNoise(M_JUMP_NOISE_PATH, 100.0f);
            }
        } else {
            m_upVelocity = -M_GRAVITY * 20.0f;
            m_height = heightAtPoint + 0.1f;

            Eend::Particles::get().create(
                m_position,
                2,
                getKickParticleProperties(m_facing.getDirection()));
            Eend::Particles::get().create(m_position, 5, getJumpParticleProperties());
            Eend::Audio::get().playNoise(M_JUMP_NOISE_PATH, 100.0f);
        }
    } else if (m_inAir) {
        m_upVelocity += M_GRAVITY;
        m_height += (m_upVelocity * dt);
        if (m_height < heightAtPoint) {
            m_inAir = false;
            m_height = heightAtPoint;
        }
    } else {
        m_height = heightAtPoint;
    }

    m_position.z = m_height;

    Trey::updateBody(dt);
}

bool Trey::kick(Dog& dog) {
    bool dies = false;
    if (m_item) {
        if (*m_item == ItemType::doubleKick) {
            std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
                dog.getPosition3d(),
                Eend::Sphere(getPosition(), M_KICK_RADIUS),
                m_rotation,
                M_KICK_SPREAD);
            if (kick) {
                // include the damage here also I think
                dog.kick(*kick);
                dies = dog.giveDamage(1);
            }
            kick = pointToSphereSliceEdgeRelative(
                dog.getPosition3d(),
                Eend::Sphere(getPosition(), M_KICK_RADIUS),
                m_rotation + Eend::Angle(180.0f),
                M_KICK_SPREAD);
            if (kick) {
                dog.kick(*kick);
                dies = dog.giveDamage(1);
            }
        }
    } else {
        std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
            dog.getPosition3d(),
            Eend::Sphere(getPosition(), M_KICK_RADIUS),
            m_rotation,
            M_KICK_SPREAD);
        if (kick) {
            dog.kick(*kick);
            dies = dog.giveDamage(1);
        }
    }
    return dies;
}

void Trey::updateBody(float dt) {

    Eend::Statue* head = Eend::Entities::statues().getRef(m_headId);
    Eend::Statue* hair = Eend::Entities::statues().getRef(m_hairId);
    Eend::Statue* hairOutter = Eend::Entities::statues().getRef(m_hairOutterId);
    Eend::Board* body = Eend::Entities::boards().getRef(m_bodyId);
    Eend::Board* eye = Eend::Entities::boards().getRef(m_eyeId);

    Direction direction = m_facing.getDirection();
    std::string prefix;

    // update strip
    if (m_inAir) {
        prefix = "kick";
    } else if (m_moving) {
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
        if (m_facing.getUpOrDown() == Direction::up) {
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
        if (m_facing.getUpOrDown() == Direction::up) {
            body->setStrip(prefix + "SideBackward");
        } else {
            body->setStrip(prefix + "SideForward");
        }
        body->setStripFlip(true);
    } else /*  direction == Direction::upLeft */ {
        body->setStrip(prefix + "SideBackward");
        body->setStripFlip(true);
    }

    if (m_bodyTimer.update(dt)) {
        m_bodyTimer.reset();
        body->nextStripIdx();
    }

    // set head rotation
    m_rotation = m_facing.getAngle();
    head->setRotation(Eend::Rotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f));
    hair->setRotation(Eend::Rotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f));
    hairOutter->setRotation(Eend::Rotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f));

    // set eye rotation
    eye->setRotation(eye->getRotation() + (1.0f * dt));
    eye->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 4.1f));

    // set position
    body->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 3.0f));
    head->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 4.1f));
    hair->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 4.1f));
    hairOutter->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 4.1f));
}

bool Trey::isKicking() { return m_kicking; }

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
        if (m_facing.getDirection() == *direction) newDirection = true;
        m_facing.setDirection(*direction);
        m_moving = true;
    } else {
        m_moving = false;
    }
    return newDirection;
}

void Trey::updatePosition(float dt) {

    switch (m_facing.getDirection()) {
    case Direction::up:
        m_position.y += M_MOVE_SPEED * dt;
        break;
    case Direction::upRight:
        m_position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::right:
        m_position.x += M_MOVE_SPEED * dt;
        break;
    case Direction::downRight:
        m_position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::down:
        m_position.y -= M_MOVE_SPEED * dt;
        break;
    case Direction::downLeft:
        m_position.y -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    case Direction::left:
        m_position.x -= M_MOVE_SPEED * dt;
        break;
    case Direction::upLeft:
        m_position.y += M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        m_position.x -= M_MOVE_SPEED * Eend::INV_SQRT_TWO * dt;
        break;
    }
}

void Trey::handleCollision(Eend::Point& oldPosition) {
    if (!Park::get().colliding(Eend::Point2D(m_position.x, m_position.y))) {
    } else if (!Park::get().colliding(Eend::Point2D(oldPosition.x, m_position.y))) {
        m_position.x = oldPosition.x;
    } else if (!Park::get().colliding(Eend::Point2D(m_position.x, oldPosition.y))) {
        m_position.y = oldPosition.y;
    } else {
        m_position.x = oldPosition.x;
        m_position.y = oldPosition.y;
    }
}

Eend::Angle Trey::getAngle() { return m_facing.getAngle(); }
