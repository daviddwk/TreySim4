#include "items.hpp"
#include "park.hpp"

#include <Eendgine/fatalError.hpp>

constexpr float HEIGHT_OFFSET = 5.0f;
constexpr Eend::Scale2D SCALE = Eend::Scale2D(5.0f);

Item::Item(Item::Type type, Eend::Point2D position)
    : m_type(type),
      m_position(
          Eend::Point(position.x, position.y, Park::get().heightAtPoint(position) + HEIGHT_OFFSET)),
      m_sprite(Eend::Entities::boards().insert(Item::spritePath(type))) {
    Eend::Entities::boards().getRef(m_sprite)->setScale(Eend::Scale2D(SCALE));
}

Item::~Item() { Eend::Entities::boards().erase(m_sprite); }

Eend::Point Item::getPosition() { return m_position; }

std::filesystem::path Item::spritePath(Item::Type type) {
    switch (type) {
    case Item::Type::doubleKick:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    default:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    }
}

Item::Type Item::typeFromString(std::string typeString) {
    if (typeString == "doubleKick") {
        return Item::Type::doubleKick;
    } else {
        Eend::fatalError("item type " + typeString + " does not exsist");
    }
}
