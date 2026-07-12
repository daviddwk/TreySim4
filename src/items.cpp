#include "items.hpp"
#include "park.hpp"

constexpr float HEIGHT_OFFSET = 5.0f;
constexpr Eend::Scale2D SCALE = Eend::Scale2D(5.0f);

Item::Item(ItemType type, Eend::Point2D position)
    : m_type(type),
      m_position(
          Eend::Point(position.x, position.y, Park::get().heightAtPoint(position) + HEIGHT_OFFSET)),
      m_sprite(Eend::Entities::boards().insert(Item::spritePath(type))) {
    Eend::Entities::boards().getRef(m_sprite)->setScale(Eend::Scale2D(SCALE));
}

Item::~Item() { Eend::Entities::boards().erase(m_sprite); }

Eend::Point Item::getPosition() { return m_position; }

std::filesystem::path Item::spritePath(ItemType type) {
    switch (type) {
    case ItemType::doubleKick:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    default:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    }
}
