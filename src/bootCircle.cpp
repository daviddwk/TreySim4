#include "bootCircle.hpp"

#include "collision.hpp"

constexpr float DEFAULT_BOOT_RADIUS = 2.5f;
constexpr float DEFAULT_CIRCLE_RADIUS = 15.0f;
constexpr float DEFAULT_SPEED = 10.0f;

constexpr float BOOT_SPRITE_OFFSET = 2.5f;
constexpr float BOOT_COLLISION_OFFSET = 0.0f;
const std::filesystem::path BOOT_SPRITE_PATH = std::filesystem::path("resources/boot");

// traits
BootCircleTraits::BootCircleTraits(float bootRadius, float circleRadius, float speed)
    : boot_radius(bootRadius), circle_radius(circleRadius), speed(speed) {};

BootCircleTraits& BootCircleTraits::operator+=(const BootCircleTraits& rhs) {
    boot_radius += rhs.boot_radius;
    circle_radius += rhs.circle_radius;
    speed += rhs.speed;
    return *this;
}

BootCircleTraits& BootCircleTraits::operator-=(const BootCircleTraits& rhs) {
    boot_radius -= rhs.boot_radius;
    circle_radius -= rhs.circle_radius;
    speed -= rhs.speed;
    return *this;
}

// item
BootCircleItem::BootCircleItem(float boot_radius, float circle_radius, float speed)
    : traits(boot_radius, circle_radius, speed) {};

// circle
BootCircle::BootCircle()
    : boot_rotation(1.0f, 0.0f), traits(DEFAULT_BOOT_RADIUS, DEFAULT_CIRCLE_RADIUS, DEFAULT_SPEED) {

    item_traits.emplace(0.0f, 0.0f, 0.0f);
}

BootCircle::~BootCircle() {
    if (boot_sprite) Eend::Entities::boards().erase(*boot_sprite);
}

void BootCircle::update(Eend::Point trey_position) {

    float dt = Eend::FrameLimiter::get().deltaTime;

    // unconditionally update boot position
    boot_rotation = glm::rotate(boot_rotation, dt * traits.speed);
    const Eend::Vector boot_position_relative =
        traits.circle_radius * Eend::Vector(boot_rotation.x, boot_rotation.y, 0.0f);
    boot_position = trey_position + boot_position_relative + BOOT_COLLISION_OFFSET;
    // TODO add a bit of lag behind trey's position?

    // handle sprite
    if (boot_sprite && !enabled()) {
        Eend::Entities::boards().erase(*boot_sprite);
    } else if (!boot_sprite && enabled()) {
        boot_sprite = Eend::Entities::boards().insert(BOOT_SPRITE_PATH);
    }
    if (boot_sprite) {
        Eend::Board* board_ref = Eend::Entities::boards().getRef(*boot_sprite);
        board_ref->setPosition(boot_position + BOOT_SPRITE_OFFSET);
        board_ref->setScale(Eend::Scale2D(traits.boot_radius * 2));
    }
}

void BootCircle::hit(Eend::Point trey_position, Dog& dog) {
    if (enabled()) {
        Eend::Sphere bootSphere(boot_position, traits.boot_radius);
        bool kicked = pointOnSphere(dog.getPosition3d(), bootSphere);
        if (kicked) {
            dog.kick(glm::normalize(boot_position - trey_position));
            dog.giveDamage(1);
        }
    }
}

bool BootCircle::enabled() { return !item_traits.empty(); }

void BootCircle::addTraits(BootCircleTraits newTraits) {
    traits += newTraits;
    item_traits.push(newTraits);
}

void BootCircle::removeTraits() {
    if (enabled()) {
        BootCircleTraits oldTraits = item_traits.front();
        traits -= oldTraits;
        item_traits.pop();
    }
}
