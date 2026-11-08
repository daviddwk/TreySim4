#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

#include <optional>

namespace Eend = Eendgine;

class Dog {
    public:
        enum class Type { Classic, Snow };
        Dog(Eend::Point2D position, Eend::Scale2D scale, float speed, Dog::Type type);
        ~Dog();

        Dog(const Dog& other) noexcept;
        Dog& operator=(const Dog& other) noexcept;

        void setSpeed(float speed);
        Eend::Point2D getPosition();
        Eend::Point getPosition3d();

        unsigned int getDamage();
        // retruns true if dies
        bool giveDamage(unsigned int damage);

        unsigned int getHealth();
        bool shouldDelete();

        void kick(Eend::Point kick);

        void update();

        static Dog::Type stringToType(std::string string);

    private:
        std::filesystem::path getDogPath(Type type);

        Type type;
        Eend::BoardId body_id;
        Eend::Point2D position;
        float speed;
        Eend::Vector2D knockback;
        float anim_time;
        unsigned int health;
        float dead_time;
        bool to_delete;
};
