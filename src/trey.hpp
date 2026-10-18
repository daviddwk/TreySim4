#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/timer.hpp>
#include <Eendgine/types.hpp>
#include <GL/gl.h>

#include <memory>
#include <optional>

#include "dog.hpp"
#include "facing.hpp"
#include "healthBar.hpp"
#include "items.hpp"

#include "bootCircle.hpp"
#include "treyFeet.hpp"

namespace Eend = Eendgine;

class Trey {
    public:
        static void construct();
        static void destruct();
        static Trey& get();

        void setRotation(Eend::Angle rotation);
        void setPosition(Eend::Point position);
        void setAlive(bool alive);

        void pickupItem(Item::Type itemType);

        Eend::Point getPosition();
        Eend::Point2D getPosition2D();
        float getRadius();
        Eend::Angle getAngle();

        bool isKicking();
        // return true if dog dies
        void hit(Dog& dog);

        void update(bool activeCutscene);
        void cutsceneUpdate();

        HealthBar health;

    private:
        Trey();
        ~Trey();

        Trey(const Trey& other) = delete;
        Trey& operator=(const Trey& other) = delete;

        Trey(Trey&& other) = delete;
        Trey& operator=(Trey&& other) = delete;

        inline static Trey* m_instance = nullptr;

        static constexpr float M_TREY_RADIUS = 5.0f;
        static constexpr float M_MOVE_SPEED = 25.0f;
        static constexpr float M_GRAVITY = -1.0f; // only ducks believe in gravity

        bool updateDirection();
        void updateBody(float dt);

        void handleMovement(float dt, std::optional<Direction> direction);
        void updatePosition(float dt);
        void handleCollision(Eend::Point& oldPosition);
        void kick(float elevationAtPosition);

        Direction oppositeDirection(Direction direction);

        BootCircle boot_circle;

        Eend::StatueId head_id;
        Eend::StatueId hair_id;
        Eend::StatueId hair_outter_id;
        Eend::BoardId body_id;
        Eend::BoardId eye_id;
        Eend::Point position;
        Eend::Angle rotation;
        Eend::Angle target_rotation;

        Eend::Timer body_timer;

        bool kicking;
        bool in_air;
        float up_velocity;
        Facing facing;
        bool moving;
        bool alive;
        std::unique_ptr<Feet::Feet> feet;
        std::optional<Item::Type> item;
};
