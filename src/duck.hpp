#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <GL/gl.h>

#include "dog.hpp"
#include "healthBar.hpp"
#include "terrain.hpp"

namespace Eend = Eendgine;

class Duck {
    public:
        static void construct();
        static void destruct();
        static Duck& get();

        void setRotation(Eend::Angle rotation);
        void setPosition(Eend::Point position);
        void setAlive(bool alive);

        Eend::Point getPosition();
        Eend::Point2D getPosition2D();
        float getRadius();
        Eend::Angle getAngle();

        bool isKicking();
        // return true if dog dies
        bool kick(Dog& dog);

        void update(float dt);

        HealthBar health;
        enum class Direction { up, upRight, right, downRight, down, downLeft, left, upLeft };

    private:
        Duck();
        ~Duck();

        Duck(const Duck& other) = delete;
        Duck& operator=(const Duck& other) = delete;

        Duck(Duck&& other) = delete;
        Duck& operator=(Duck&& other) = delete;

        inline static Duck* m_instance = nullptr;

        static constexpr float M_MOVE_SPEED = 25.0f;
        static constexpr float M_KICK_RADIUS = 15.0f;
        static constexpr float M_KICK_OFFSET = 5.0f;
        static constexpr float M_KICK_SPREAD = 120.0f;
        static constexpr float M_DUCK_RADIUS = 5.0f;
        static constexpr float M_GRAVITY = -1.0f; // only ducks believe in gravity

        const std::filesystem::path M_JUMP_NOISE_PATH =
            std::filesystem::path("resources/noises/Quack_Fart_Noise_44k.wav");

        std::optional<Direction> getDirection();

        void handleMovement(float dt, std::optional<Duck::Direction> direction);
        void updatePosition(float dt);
        void handleCollision(Terrain& terrain, Eend::Point& oldPosition);

        Eend::StatueId m_bodyId;
        Eend::BoardId m_headId;
        Eend::Point m_position;
        Eend::Angle m_rotation;

        bool m_kicking;
        bool m_inAir;
        float m_upVelocity;
        float m_height;
        Direction m_direction;
        bool m_alive;
};
