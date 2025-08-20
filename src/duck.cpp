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
    : m_bodyId(Eend::Entities::statues().insert(std::filesystem::path("duck/statues/body"))),
      m_headId(Eend::Entities::boards().insert(std::filesystem::path("duck/boards/head"))),
      m_position(Eend::Point(0.0f)), m_rotX(0.0f), m_rotY(0.0f), m_inAir(false), m_upVelocity(0.0f),
      m_height(0.0f) {
    Eend::Entities::boards().getRef(m_headId)->setScale(Eend::Scale2D(3.5f, 3.5f));
}

Duck::~Duck() {
    Eend::Entities::statues().erase(m_bodyId);
    Eend::Entities::boards().erase(m_headId);
}

void Duck::setPosition(Eend::Point position) {
    if (position.y < m_position.y) {
        Eend::Entities::boards().getRef(m_headId)->setStrip("eyesOpen");
    } else if (position.y > m_position.y) {
        Eend::Entities::boards().getRef(m_headId)->setStrip("eyesClose");
    }
    m_position = position;
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    Eend::Board* headRef = Eend::Entities::boards().getRef(m_headId);
    bodyRef->setPosition(Eend::Point(position.x - 0.5f, position.y, position.z + 0.08f));
    headRef->setPosition(Eend::Point(position.x, position.y, position.z + 3.00f));
}

void Duck::setRotation(float x, float y, float z) {
    Eend::Statue* bodyRef = Eend::Entities::statues().getRef(m_bodyId);
    bodyRef->setRotation(x, y, z);
};

Eend::Point Duck::getPosition() { return m_position; };
Eend::Point2D Duck::getPosition2D() { return Eend::Point2D(m_position.x, m_position.y); };

void Duck::update(float dt, Terrain* terrain) {

    Eend::Point duckPosition = getPosition();
    Eend::Point oldDuckPosition = duckPosition;

    static float duckRotation = 0.0f;

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

    if (Eend::InputManager::get().getSpacePress() && !m_inAir) {
        Particles::get().create(
            duckPosition, 5, std::filesystem::path("duck/boards/poo"), particleMovement);
        m_inAir = true;
        m_upVelocity = -GRAVITY * 20.0f;
        m_height = heightAtPoint + 0.1f;
    } else if (m_inAir) {
        m_upVelocity += GRAVITY;
        m_height += (m_upVelocity * dt);
        if (m_height < heightAtPoint) {
            m_inAir = false;
            m_height = heightAtPoint;
        }
    } else {
        m_height = heightAtPoint;
    }

    duckPosition.z = m_height;

    setPosition(duckPosition);
    setRotation(0.0f, 0.0f, duckRotation);
}
