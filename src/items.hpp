#pragma once

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

class Item {
    public:
        enum class Type { doubleKick };

        Item(Item::Type type, Eend::Point position);
        ~Item();

        Item(const Item& other) noexcept;
        Item& operator=(const Item& other) noexcept;

        Eend::Point getPosition();
        Item::Type getType();

        static Item::Type typeFromString(std::string typeString);

    private:
        std::filesystem::path spritePath(Item::Type type);

        Item::Type m_type;
        Eend::Point m_position;
        Eend::BoardId m_sprite;
};
