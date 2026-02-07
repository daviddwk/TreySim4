#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include <optional>

#include "terrain.hpp"

namespace Eend = Eendgine;

class Dog {
    public:
        Dog(Eend::Point2D position, Eend::Scale2D scale, float speed);
        ~Dog();

        Dog(const Dog& other) = delete;
        Dog& operator=(const Dog& other) = delete;

        Dog(Dog&& other) noexcept;
        Dog& operator=(Dog&& other) noexcept;

        void setSpeed(float speed);
        Eend::Point2D getPosition();
        Eend::Point getPosition3d();

        unsigned int getDamage();
        // retruns true if dies
        bool giveDamage(unsigned int damage);

        unsigned int getHealth();
        bool shouldDelete();

        void kick(Eend::Point kick);

        void update(float dt);

    private:
        static constexpr float M_SPEED = 15.0f;

        static constexpr unsigned int M_DAMAGE = 2;
        static constexpr unsigned int M_HEALTH = 3;

        static constexpr float M_KNOCKBACK_MAX = M_SPEED * 4.0f;
        static constexpr float M_KNOCKBACK_MIN = M_SPEED;
        static constexpr float M_KNOCKBACK_DECAY_FACTOR = 1.0f;
        static constexpr float M_KNOCKBACK_STOP = 5.0;
        static constexpr float M_KNOCKBACK_DEAD_MULTIPLIER = 1.5f;

        static constexpr float M_ANIM_INCREMENT_TIME = 0.25f;
        static constexpr float M_QUICK_INCREMENT_TIME = 0.125f;

        static constexpr float M_UP_OFFSET = 2.0f;
        static constexpr float M_STOP_DISTANCE = 3.0f;

        static constexpr float M_BLINK_AFTER_DEATH_TIME = 3.0f;
        static constexpr float M_DELETE_AFTER_DEATH_TIME = 5.0f;

        std::optional<Eend::BoardId> m_bodyId;
        Eend::Point2D m_position;
        float m_speed;
        Eend::Vector2D m_knockback;
        float m_animTime;
        unsigned int m_health;
        float m_deadTime;
        bool m_delete;
};
