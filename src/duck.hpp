#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>
#include <GL/gl.h>

namespace Eend = Eendgine;

class Duck {
    public:
        static void construct();
        static void destruct();
        static Duck& get();

        void setRotation(Eend::Angle rotation);
        void setPosition(Eend::Point position);

        Eend::Point getPosition();
        Eend::Point2D getPosition2D();
        float getRadius();
        Eend::Angle getAngle();

        void update();

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

        Eend::StatueId m_bodyId;
        Eend::BoardId m_headId;
        Eend::Point m_position;
        Eend::Angle m_rotation;
};
