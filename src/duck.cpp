#include <Eendgine/audio.hpp>
#include <Eendgine/inputManager.hpp>

#include <optional>
#include <print>

#include "collision.hpp"
#include "park.hpp"

#include "duck.hpp"
#include "duckParticles.hpp"
// have static last direction in update function
// use this to get new diruction if there is one, and know that keys are actually being pressed
//
//

Duck::Duck()
    : m_bodyId(Eend::Entities::statues().insert(std::filesystem::path("duck/statues/body"))),
      m_headId(Eend::Entities::boards().insert(std::filesystem::path("duck/boards/head"))),
      m_position(Eend::Point(0.0f)), m_rotation(Eend::Angle(0.0f)), m_kicking(true), m_inAir(false),
      m_upVelocity(0.0f), m_height(0.0f), m_direction(Direction::up), m_alive(true) {
    Eend::Board* head = Eend::Entities::boards().getRef(m_headId);
    head->setScale(Eend::Scale2D(3.5f, 3.5f));
    head->setStrip("eyesOpen");
}

Duck::~Duck() {
    Eend::Entities::statues().erase(m_bodyId);
    Eend::Entities::boards().erase(m_headId);
}

void Duck::construct() {
    assert(m_instance == nullptr);
    m_instance = new Duck();
}

void Duck::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Duck& Duck::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

void Duck::setPosition(Eend::Point position) { m_position = position; }
void Duck::setAlive(bool alive) { m_alive = alive; }

Eend::Point Duck::getPosition() { return m_position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };
float Duck::getRadius() { return M_DUCK_RADIUS; }

void Duck::update(float dt) {

    Eend::Point oldDuckPosition = getPosition();

    std::optional<Direction> currentDirection = getDirection();
    if (currentDirection && m_alive) {
        m_direction = *currentDirection;
        m_rotation = getAngle();
        updatePosition(dt);
    } else {
        m_rotation = m_rotation + Eend::Angle(100 * dt);
    }
    handleCollision(oldDuckPosition);

    float heightAtPoint = Park::get().heightAtPoint(Eend::Point2D(m_position.x, m_position.y));

    m_kicking = false;
    if (Eend::InputManager::get().getSpacePress() && !m_inAir && m_alive) {
        m_kicking = true;
        m_inAir = true;
        m_upVelocity = -M_GRAVITY * 20.0f;
        m_height = heightAtPoint + 0.1f;

        Eend::Particles::get().create(m_position, 2, getKickParticleProperties(m_direction));
        Eend::Particles::get().create(m_position, 5, getJumpParticleProperties());
        Eend::Audio::get().playNoise(M_JUMP_NOISE_PATH, 100.0f);
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

    // update entities
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(m_headId);

    if (m_position.y < oldDuckPosition.y) {
        headRef->setStrip("eyesOpen");
    } else if (m_position.y > oldDuckPosition.y) {
        headRef->setStrip("eyesClose");
    }

    bodyRef->setPosition(Eend::Point(m_position.x - 0.5f, m_position.y, m_position.z + 0.08f));
    headRef->setPosition(Eend::Point(m_position.x, m_position.y, m_position.z + 3.00f));
    bodyRef->setRotation(0.0f, 0.0f, m_rotation.getDegrees() + 180.0f);
}

bool Duck::isKicking() { return m_kicking; }

std::optional<Duck::Direction> Duck::getDirection() {
    const bool upPress = Eend::InputManager::get().getUpPress();
    const bool rightPress = Eend::InputManager::get().getRightPress();
    const bool downPress = Eend::InputManager::get().getDownPress();
    const bool leftPress = Eend::InputManager::get().getLeftPress();

    if (upPress) {
        if (rightPress) return Direction::upRight;
        if (leftPress) return Direction::upLeft;
        return Direction::up;
    }
    if (downPress) {
        if (rightPress) return Direction::downRight;
        if (leftPress) return Direction::downLeft;
        return Direction::down;
    }
    if (rightPress) return Direction::right;
    if (leftPress) return Direction::left;
    return std::nullopt;
}

void Duck::updatePosition(float dt) {

    switch (m_direction) {
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

void Duck::handleCollision(Eend::Point& oldPosition) {
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

Eend::Angle Duck::getAngle() {
    switch (m_direction) {
    case Direction::up:
        return Eend::Angle(45.0f) * 0.0f;
    case Direction::upRight:
        return Eend::Angle(45.0f) * 1.0f;
    case Direction::right:
        return Eend::Angle(45.0f) * 2.0f;
    case Direction::downRight:
        return Eend::Angle(45.0f) * 3.0f;
    case Direction::down:
        return Eend::Angle(45.0f) * 4.0f;
    case Direction::downLeft:
        return Eend::Angle(45.0f) * 5.0f;
    case Direction::left:
        return Eend::Angle(45.0f) * 6.0f;
    case Direction::upLeft:
        return Eend::Angle(45.0f) * 7.0f;
    }
}

bool Duck::kick(Dog& dog) {
    std::optional<Eend::Vector> kick = pointToSphereSliceEdgeRelative(
        dog.getPosition3d(),
        Eend::Sphere(getPosition(), M_KICK_RADIUS),
        m_rotation,
        M_KICK_SPREAD);
    if (kick) {
        dog.kick(*kick);
        bool dies = dog.giveDamage(1);
        return dies;
    }
    return false;
}
