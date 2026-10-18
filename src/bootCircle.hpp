#include <glm/ext/quaternion_transform.hpp>
#include <queue>

#include <Eendgine/frameLimiter.hpp>

#include "collision.hpp"
#include "dog.hpp"
#include "items.hpp"

constexpr float DEFAULT_BOOT_RADIUS = 2.5f;
constexpr float DEFAULT_CIRCLE_RADIUS = 15.0f;
constexpr float DEFAULT_SPEED = 10.0f;

constexpr float BOOT_UP_OFFSET = 3.0f;

const std::filesystem::path BOOT_SPRITE_PATH = std::filesystem::path("resources/duck/statues/body");

class BootCircleTraits {
    public:
        BootCircleTraits(float bootRadius, float circleRadius, float speed)
            : boot_radius(bootRadius), circle_radius(circleRadius), speed(speed) {};

        BootCircleTraits& operator+=(const BootCircleTraits& rhs) {
            boot_radius += rhs.boot_radius;
            circle_radius += rhs.circle_radius;
            speed += rhs.speed;
            return *this;
        }
        BootCircleTraits& operator-=(const BootCircleTraits& rhs) {
            boot_radius -= rhs.boot_radius;
            circle_radius -= rhs.circle_radius;
            speed -= rhs.speed;
            return *this;
        }

        float boot_radius; // TODO make everything snakecase lol
        float circle_radius;
        float speed;
};

class BootCircleItem {
    public:
        BootCircleItem(float boot_radius, float circle_radius, float speed)
            : traits(boot_radius, circle_radius, speed) {};
        const BootCircleTraits traits;

    private:
        // Item item;
};

class BootCircle { // TODO particle system that follows boot!
    public:
        BootCircle()
            : boot_rotation(1.0f, 0.0f),
              traits(DEFAULT_BOOT_RADIUS, DEFAULT_CIRCLE_RADIUS, DEFAULT_SPEED) {

            itemTraits.emplace(0.0f, 0.0f, 0.0f);
        };

        void update(Eend::Point trey_position) {

            float dt = Eend::FrameLimiter::get().deltaTime;

            // unconditionally update boot position
            boot_rotation = glm::rotate(boot_rotation, dt * traits.speed);
            const Eend::Vector boot_position_relative =
                traits.circle_radius * Eend::Vector(boot_rotation.x, boot_rotation.y, 0.0f);
            boot_position = trey_position + boot_position_relative + BOOT_UP_OFFSET;

            // handle sprite
            if (boot_sprite && !enabled()) {
                Eend::Entities::boards().erase(*boot_sprite);
            } else if (!boot_sprite && enabled()) {
                boot_sprite = Eend::Entities::boards().insert(BOOT_SPRITE_PATH);
            }
            if (boot_sprite) {
                Eend::Board* board_ref = Eend::Entities::boards().getRef(*boot_sprite);
                board_ref->setPosition(boot_position);
                board_ref->setScale(Eend::Scale2D(traits.boot_radius * 2));
            }
        };
        bool hit(Dog& dog) {
            if (enabled()) {
                Eend::Sphere bootSphere(boot_position, traits.boot_radius);
                bool kicked = pointOnSphere(dog.getPosition3d(), bootSphere);

                if (kicked) {
                    // TODO trey position
                    dog.kick(glm::normalize(dog.getPosition3d() - boot_position));
                    return dog.giveDamage(1);
                }
            }
            return false;
        };
        bool enabled() { return !itemTraits.empty(); }

        void addTraits(BootCircleTraits newTraits) {
            traits += newTraits;
            itemTraits.push(newTraits);
        };
        void removeTraits() {
            if (enabled()) {
                BootCircleTraits oldTraits = itemTraits.front();
                traits -= oldTraits;
                itemTraits.pop();
            }
        };

    private:
        std::optional<Eend::BoardId> boot_sprite;
        Eend::Point boot_position;
        Eend::Vector2D boot_rotation;

        BootCircleTraits traits;
        std::queue<BootCircleTraits> itemTraits;
};
