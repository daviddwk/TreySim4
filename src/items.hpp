#pragma once

enum class ItemType { doubleKick };

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/types.hpp>

namespace Eend = Eendgine;

class Item {
    public:
        Item(ItemType type, Eend::Point2D position);
        ~Item();

        Item(const Item& other) = delete;
        Item& operator=(const Item& other) = delete;

        Item(Item&& other) = delete;
        Item& operator=(Item&& other) = delete;

        Eend::Point getPosition();

    private:
        std::filesystem::path spritePath(ItemType type);

        ItemType m_type;
        Eend::Point m_position;
        Eend::BoardId m_sprite;
};
